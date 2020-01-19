/***************************************************************************
                          lircsupport.cpp  -  description
                             -------------------
    begin                : Mon Feb 4 2002
    copyright            : (C) 2002 by Martin Witte / Frank Schwanz
    email                : emw-kradio@nocabal.de / schwanz@fh-brandenburg.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "lircsupport.h"

#include <lirc/lirc_client.h>

#include <QSocketNotifier>
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QApplication>

#include <KAboutData>
#include <QStandardPaths>

#include "errorlog_interfaces.h"
#include "radiodevice_interfaces.h"
#include "seekradio_interfaces.h"
#include "stationlist.h"

#include "lirc-configuration.h"

#define LIRCRC     ".lircrc"
#define LIRCPROG   ((char*)"kradio")

///////////////////////////////////////////////////////////////////////
//// plugin library functions

static KAboutData aboutData()
{
    KAboutData about("LircSupport",
                     i18nc("@title", "LIRC"),
                     KRADIO_VERSION,
                     i18nc("@title", "Linux Infrared Control (LIRC) Support"),
                     KAboutLicense::LicenseKey::GPL,
                     i18nc("@info:credit", "(c) 2002-2005 Martin Witte"),
                     NULL,
                     "http://sourceforge.net/projects/kradio",
                     "emw-kradio@nocabal.de");
    about.addAuthor(i18nc("@info:credit", "Martin Witte"), NULL, "emw-kradio@nocabal.de");
    return about;
}

KRADIO_EXPORT_PLUGIN(LircSupport, aboutData())

/////////////////////////////////////////////////////////////////////////////

LircSupport::LircSupport(const QString &instanceID, const QString &name)
    : PluginBase(instanceID, name, i18n("LIRC Plugin")),
      m_lirc_config_file(QDir::homePath() + "/" + LIRCRC),
      m_lirc_notify(NULL),
      m_fd_lirc(-1),
      m_lircConfig(NULL),
      m_LIRCModeSyncAtStartup(true),
      m_LIRCModeSyncAtRuntime(true),
      m_inStartupPhase(true),
      m_ignorePowerOnOff(false)
{
    m_kbdTimer = new QTimer (this);
    QObject::connect (m_kbdTimer, SIGNAL(timeout()), this, SLOT(slotKbdTimedOut()));

    m_addIndex = 0;

    LIRC_init_fd();
}


LircSupport::~LircSupport()
{
    LIRC_close_config();
    LIRC_close_fd();
}


void LircSupport::LIRC_init_fd()
{
    QString dbgStartMsg1 = i18n("initializing KRadio LIRC plugin");
    QString dbgStartMsg2 = i18n("warnings/errors about missing sockets do not harm - usually the LIRC daemon is not running in these cases.");
    logDebug(dbgStartMsg1);
    logDebug(dbgStartMsg2);
    fprintf (stderr, "%s\n%s\n", qPrintable(dbgStartMsg1), qPrintable(dbgStartMsg2));

    m_fd_lirc     = lirc_init(LIRCPROG, 1);

    if (m_fd_lirc == -1) {
        m_lirc_notify = NULL;
        QString         warnMsg = i18n("Initializing KRadio LIRC plugin failed");
        logWarning      (warnMsg);
        staticLogWarning(warnMsg);
        fprintf (stderr, "%s\n", qPrintable(warnMsg));
    } else {
        m_lirc_notify = new QSocketNotifier(m_fd_lirc, QSocketNotifier::Read, this);
        if (m_lirc_notify)
            QObject::connect(m_lirc_notify, SIGNAL(activated(int)), this, SLOT(slotLIRC(int)));

        QString       dbgMsg = i18n("Initializing KRadio LIRC plugin successful");
        logDebug      (dbgMsg);
        staticLogDebug(dbgMsg);
        fprintf (stderr, "%s\n", qPrintable(dbgMsg));
    }
}

void LircSupport::LIRC_init_config()
{
    m_lircConfig  = 0;

    if (m_fd_lirc != -1) {
        checkLIRCConfigurationFile(m_lirc_config_file);

        QByteArray lirc_cfg_filename_ba = QFile::encodeName(m_lirc_config_file);
        if (lirc_readconfig (lirc_cfg_filename_ba.data(), &m_lircConfig, NULL) == 0) {

            // check config
            lirc_config_entry *found = NULL;
            for (lirc_config_entry *e = m_lircConfig ? m_lircConfig->first : NULL; e; e = e->next) {
                if (QString(e->prog) == LIRCPROG) {
                    found = e;

                    // syntax check
                    const lirc_code *code = e->code;
                    if (!code) {
                        logWarning(i18n("LircSupport::LIRC_init_config: In %1 an LIRC Config Entry for KRadio does not have a button/remote associated.\n"
                                        "This will most probably lead to problems. Please use \"button=*\" and \"remote=*\" as wildcard for all buttons/remotes", m_lirc_config_file));
                    }
                    while (code) {
                        const char *btn = code->button;
                        const char *rem = code->remote;
                        if (!btn || ((btn != (char*)-1ll) && strlen(btn) == 0) || !rem || ((rem != (char*)-1ll) && strlen(rem) == 0)) {
                            logWarning(i18n("LircSupport::LIRC_init_config: In %1 an LIRC Config Entry for KRadio has an incomplete button/remote associated.\n"
                                            "This will most probably lead to problems. Please use \"button=*\" and \"remote=*\" as wildcard for all buttons/remotes", m_lirc_config_file));
                        }
                        code = code->next;
                    }
                }
                if ((e->flags & startup_mode) == startup_mode) {
                    m_lircrc_startup_mode = e->change_mode;
                }
            }
            if (!found) {
                logWarning(i18n("There is no entry for KRadio in your .lircrc files %1.", m_lirc_config_file));
                logWarning(i18n("Please setup your .lircrc file %1 correctly.",           m_lirc_config_file));
            }

        } else {
            logWarning      (i18n("Initializing KRadio LIRC plugin failed. Could not read config file %1", m_lirc_config_file));
            staticLogWarning(i18n("Initializing KRadio LIRC plugin failed. Could not read config file %1", m_lirc_config_file));
        }
    }
}


void LircSupport::LIRC_close_fd()
{
    if (m_fd_lirc != -1)
        lirc_deinit();
    m_fd_lirc = -1;

    if (m_lirc_notify)
        delete m_lirc_notify;
    m_lirc_notify = NULL;
}

void LircSupport::LIRC_close_config()
{
    if (m_lircConfig)
        lirc_freeconfig(m_lircConfig);
    m_lircConfig = 0;
}


void LircSupport::setLIRCConfigurationFile(const QString &fname)
{
    LIRC_close_config();
    m_lirc_config_file = fname;
    LIRC_init_config();
}


void LircSupport::checkLIRCConfigurationFile(const QString &fname)
{
    if (!QFile::exists(fname)) {
        logWarning(i18n("%1 does not exist. File was created with KRadio's default .lircrc proposal", fname));
        QString default_lircrc_filename = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kradio4/default-dot-lircrc");
        if (!QFile::copy(default_lircrc_filename, fname)) {
            logError(i18n("Failed to copy %1 to %2", default_lircrc_filename, fname));
        }
    }
}


struct tmp_code_struct
{
    QString  code;
    bool     is_event_map;
    bool     is_raw;
    tmp_code_struct() : code(), is_event_map(false), is_raw(false) {}
    tmp_code_struct(const QString &c, bool m, bool r) : code(c), is_event_map(m), is_raw(r) {}
};

void LircSupport::slotLIRC(int /*socket*/ )
{
    // we need this list to catch all commands as quickly as possible
    // in order to free all lirc stuff *before* we start running our own
    // code. Makes for some unknown reason a major difference e.g. for internet
    // radio: it happened, that repeat codes haven't been decoded properly
    // plus getting the whole lirc stuff out of sync. basically only observed
    // together with lircrcd
    QList<tmp_code_struct> list;

    if (!m_lircConfig || !m_lirc_notify || m_fd_lirc == -1)
        return;

    char *code = NULL, *c = NULL;
    if ((lirc_nextcode(&code) == 0) && code) {
        // option 1) lirc_code2char results in (possibly multiple) data, then loop until c == NULL
        // option 1a) c is "eventmap": do the same as with option 2) in this loop iteration
        // option 1b) c is other: send c directly
        // option 2) lirc_code2char results in no data, split line and send code

        int char_count = 0;
        while((lirc_code2char (m_lircConfig, code, &c) == 0 && c != NULL)) {
            ++char_count;
            bool    is_eventmap = false;
            QString x      = c;
            if (x == "eventmap") {
                is_eventmap = true;
                x           = code;
            }
            list.append(tmp_code_struct(x, is_eventmap, false));
        }
        if (char_count == 0) {
            list.append(tmp_code_struct(code, false, true));
        }
    }
    else {
        // some error has occurred on the socket => close lirc plugin
        logWarning(i18n("Reading from LIRC socket failed. Disabling LIRC functionalities until the next start of KRadio"));
        delete m_lirc_notify;
        m_lirc_notify = NULL;
    }

    if (code)
        free (code);

    // only process commands after finishing the lirc api processing

    tmp_code_struct tmp_code;
    foreach(tmp_code, list) {
        if (tmp_code.is_raw)
            logDebug(QString("LIRC(mode=%1): decoding raw, lirc_code2char gave no answer").arg(lirc_getmode(m_lircConfig)));
        processLIRCCode(tmp_code.code, tmp_code.is_event_map, tmp_code.is_raw);
    }
}


void LircSupport::processLIRCCode(const QString &c, bool is_eventmap, bool is_raw)
{
    m_ignorePowerOnOff = true;

    QString x = c;
    int     repeat_counter = 0;

    if (is_eventmap || is_raw) {
        QStringList l = QString(c).split(" ");
        if (l.count() >= 4) {
            x = l[2];
            repeat_counter = l[1].toInt(NULL, 16);
        }
    }
    if (is_raw)
        x = "raw::" + x;

    bool consumed = false;
    logDebug(QString("LIRC(mode=%1): %2 (rep = %3)").arg(lirc_getmode(m_lircConfig), x).arg(repeat_counter));

    emit sigRawLIRCSignal(x, repeat_counter, consumed);

    if (!consumed) {
        if (!checkActions(x, repeat_counter, m_Actions))
            checkActions(x, repeat_counter, m_AlternativeActions);
    }

    m_ignorePowerOnOff = false;
}


void LircSupport::slotKbdTimedOut()
{
    activateStation (m_addIndex);
    m_addIndex = 0;
}


void LircSupport::activateStation (int i)
{
    if (! sendActivateStation(i - 1))
        sendActivateStation( (i + 9) % 10);
}


bool LircSupport::connectI (Interface *i)
{
    bool a = IRadioClient::connectI (i);
    bool b = ITimeControlClient::connectI (i);
    bool c = IRadioDevicePoolClient::connectI (i);
    bool d = PluginBase::connectI(i);
    bool e = ISoundStreamClient::connectI(i);
    return a || b || c || d || e;
}


bool LircSupport::disconnectI (Interface *i)
{
    bool a = IRadioClient::disconnectI (i);
    bool b = ITimeControlClient::disconnectI (i);
    bool c = IRadioDevicePoolClient::disconnectI (i);
    bool d = PluginBase::disconnectI(i);
    bool e = ISoundStreamClient::disconnectI(i);
    return a || b || c || d || e;
}



void   LircSupport::saveState (KConfigGroup &c) const
{
    PluginBase::saveState(c);

    c.writeEntry("lirc-configuration-file", m_lirc_config_file);

    c.writeEntry("LIRC_DIGIT_0",          m_Actions[LIRC_DIGIT_0]);
    c.writeEntry("LIRC_DIGIT_1",          m_Actions[LIRC_DIGIT_1]);
    c.writeEntry("LIRC_DIGIT_2",          m_Actions[LIRC_DIGIT_2]);
    c.writeEntry("LIRC_DIGIT_3",          m_Actions[LIRC_DIGIT_3]);
    c.writeEntry("LIRC_DIGIT_4",          m_Actions[LIRC_DIGIT_4]);
    c.writeEntry("LIRC_DIGIT_5",          m_Actions[LIRC_DIGIT_5]);
    c.writeEntry("LIRC_DIGIT_6",          m_Actions[LIRC_DIGIT_6]);
    c.writeEntry("LIRC_DIGIT_7",          m_Actions[LIRC_DIGIT_7]);
    c.writeEntry("LIRC_DIGIT_8",          m_Actions[LIRC_DIGIT_8]);
    c.writeEntry("LIRC_DIGIT_9",          m_Actions[LIRC_DIGIT_9]);
    c.writeEntry("LIRC_POWER_ON",         m_Actions[LIRC_POWER_ON]);
    c.writeEntry("LIRC_POWER_OFF",        m_Actions[LIRC_POWER_OFF]);
    c.writeEntry("LIRC_PAUSE",            m_Actions[LIRC_PAUSE]);
    c.writeEntry("LIRC_RECORD_START",     m_Actions[LIRC_RECORD_START]);
    c.writeEntry("LIRC_RECORD_STOP",      m_Actions[LIRC_RECORD_STOP]);
    c.writeEntry("LIRC_VOLUME_INC",       m_Actions[LIRC_VOLUME_INC]);
    c.writeEntry("LIRC_VOLUME_DEC",       m_Actions[LIRC_VOLUME_DEC]);
    c.writeEntry("LIRC_CHANNEL_NEXT",     m_Actions[LIRC_CHANNEL_NEXT]);
    c.writeEntry("LIRC_CHANNEL_PREV",     m_Actions[LIRC_CHANNEL_PREV]);
    c.writeEntry("LIRC_SEARCH_NEXT",      m_Actions[LIRC_SEARCH_NEXT]);
    c.writeEntry("LIRC_SEARCH_PREV",      m_Actions[LIRC_SEARCH_PREV]);
    c.writeEntry("LIRC_SLEEP",            m_Actions[LIRC_SLEEP]);
    c.writeEntry("LIRC_APPLICATION_QUIT", m_Actions[LIRC_APPLICATION_QUIT]);


    c.writeEntry("ALT_LIRC_DIGIT_0",          m_AlternativeActions[LIRC_DIGIT_0]);
    c.writeEntry("ALT_LIRC_DIGIT_1",          m_AlternativeActions[LIRC_DIGIT_1]);
    c.writeEntry("ALT_LIRC_DIGIT_2",          m_AlternativeActions[LIRC_DIGIT_2]);
    c.writeEntry("ALT_LIRC_DIGIT_3",          m_AlternativeActions[LIRC_DIGIT_3]);
    c.writeEntry("ALT_LIRC_DIGIT_4",          m_AlternativeActions[LIRC_DIGIT_4]);
    c.writeEntry("ALT_LIRC_DIGIT_5",          m_AlternativeActions[LIRC_DIGIT_5]);
    c.writeEntry("ALT_LIRC_DIGIT_6",          m_AlternativeActions[LIRC_DIGIT_6]);
    c.writeEntry("ALT_LIRC_DIGIT_7",          m_AlternativeActions[LIRC_DIGIT_7]);
    c.writeEntry("ALT_LIRC_DIGIT_8",          m_AlternativeActions[LIRC_DIGIT_8]);
    c.writeEntry("ALT_LIRC_DIGIT_9",          m_AlternativeActions[LIRC_DIGIT_9]);
    c.writeEntry("ALT_LIRC_POWER_ON",         m_AlternativeActions[LIRC_POWER_ON]);
    c.writeEntry("ALT_LIRC_POWER_OFF",        m_AlternativeActions[LIRC_POWER_OFF]);
    c.writeEntry("ALT_LIRC_PAUSE",            m_AlternativeActions[LIRC_PAUSE]);
    c.writeEntry("ALT_LIRC_RECORD_START",     m_AlternativeActions[LIRC_RECORD_START]);
    c.writeEntry("ALT_LIRC_RECORD_STOP",      m_AlternativeActions[LIRC_RECORD_STOP]);
    c.writeEntry("ALT_LIRC_VOLUME_INC",       m_AlternativeActions[LIRC_VOLUME_INC]);
    c.writeEntry("ALT_LIRC_VOLUME_DEC",       m_AlternativeActions[LIRC_VOLUME_DEC]);
    c.writeEntry("ALT_LIRC_CHANNEL_NEXT",     m_AlternativeActions[LIRC_CHANNEL_NEXT]);
    c.writeEntry("ALT_LIRC_CHANNEL_PREV",     m_AlternativeActions[LIRC_CHANNEL_PREV]);
    c.writeEntry("ALT_LIRC_SEARCH_NEXT",      m_AlternativeActions[LIRC_SEARCH_NEXT]);
    c.writeEntry("ALT_LIRC_SEARCH_PREV",      m_AlternativeActions[LIRC_SEARCH_PREV]);
    c.writeEntry("ALT_LIRC_SLEEP",            m_AlternativeActions[LIRC_SLEEP]);
    c.writeEntry("ALT_LIRC_APPLICATION_QUIT", m_AlternativeActions[LIRC_APPLICATION_QUIT]);

    c.writeEntry("PowerOffMode",              m_LIRCPowerOffMode);
    c.writeEntry("PowerOnMode",               m_LIRCPowerOnMode);
    // delete old entries restored for compatibility
    c.deleteEntry("StartupPowerOffMode");
    c.deleteEntry("StartupPowerOnMode" );

    c.writeEntry("LIRCModeSyncAtStartup",     m_LIRCModeSyncAtStartup);
    c.writeEntry("LIRCModeSyncAtRuntime",     m_LIRCModeSyncAtRuntime);

}

void   LircSupport::restoreState (const KConfigGroup &c)
{
    PluginBase::restoreState(c);


    QString lirc_config_file_default = QDir::homePath() + "/" LIRCRC;

    m_lirc_config_file               = c.readEntry("lirc-configuration-file", lirc_config_file_default);

    m_Actions[LIRC_DIGIT_0]          = c.readEntry("LIRC_DIGIT_0", "0");
    m_Actions[LIRC_DIGIT_1]          = c.readEntry("LIRC_DIGIT_1", "1");
    m_Actions[LIRC_DIGIT_2]          = c.readEntry("LIRC_DIGIT_2", "2");
    m_Actions[LIRC_DIGIT_3]          = c.readEntry("LIRC_DIGIT_3", "3");
    m_Actions[LIRC_DIGIT_4]          = c.readEntry("LIRC_DIGIT_4", "4");
    m_Actions[LIRC_DIGIT_5]          = c.readEntry("LIRC_DIGIT_5", "5");
    m_Actions[LIRC_DIGIT_6]          = c.readEntry("LIRC_DIGIT_6", "6");
    m_Actions[LIRC_DIGIT_7]          = c.readEntry("LIRC_DIGIT_7", "7");
    m_Actions[LIRC_DIGIT_8]          = c.readEntry("LIRC_DIGIT_8", "8");
    m_Actions[LIRC_DIGIT_9]          = c.readEntry("LIRC_DIGIT_9", "9");
    m_Actions[LIRC_POWER_ON]         = c.readEntry("LIRC_POWER_ON",         "RADIO");
    m_Actions[LIRC_POWER_OFF]        = c.readEntry("LIRC_POWER_OFF",        "RADIO");
    m_Actions[LIRC_PAUSE]            = c.readEntry("LIRC_PAUSE",            "FULL_SCREEN");
    m_Actions[LIRC_RECORD_START]     = c.readEntry("LIRC_RECORD_START",     "");
    m_Actions[LIRC_RECORD_STOP]      = c.readEntry("LIRC_RECORD_STOP",      "");
    m_Actions[LIRC_VOLUME_INC]       = c.readEntry("LIRC_VOLUME_INC",       "VOL+");
    m_Actions[LIRC_VOLUME_DEC]       = c.readEntry("LIRC_VOLUME_DEC",       "VOL-");
    m_Actions[LIRC_CHANNEL_NEXT]     = c.readEntry("LIRC_CHANNEL_NEXT",     "CH+");
    m_Actions[LIRC_CHANNEL_PREV]     = c.readEntry("LIRC_CHANNEL_PREV",     "CH-");
    m_Actions[LIRC_SEARCH_NEXT]      = c.readEntry("LIRC_SEARCH_NEXT",      "SOURCE");
    m_Actions[LIRC_SEARCH_PREV]      = c.readEntry("LIRC_SEARCH_PREV",      "MUTE");
    m_Actions[LIRC_SLEEP]            = c.readEntry("LIRC_SLEEP",            "MINIMIZE");
    m_Actions[LIRC_APPLICATION_QUIT] = c.readEntry("LIRC_APPLICATION_QUIT", "");


    m_AlternativeActions[LIRC_DIGIT_0]          = c.readEntry("ALT_LIRC_DIGIT_0", "");
    m_AlternativeActions[LIRC_DIGIT_1]          = c.readEntry("ALT_LIRC_DIGIT_1", "");
    m_AlternativeActions[LIRC_DIGIT_2]          = c.readEntry("ALT_LIRC_DIGIT_2", "");
    m_AlternativeActions[LIRC_DIGIT_3]          = c.readEntry("ALT_LIRC_DIGIT_3", "");
    m_AlternativeActions[LIRC_DIGIT_4]          = c.readEntry("ALT_LIRC_DIGIT_4", "");
    m_AlternativeActions[LIRC_DIGIT_5]          = c.readEntry("ALT_LIRC_DIGIT_5", "");
    m_AlternativeActions[LIRC_DIGIT_6]          = c.readEntry("ALT_LIRC_DIGIT_6", "");
    m_AlternativeActions[LIRC_DIGIT_7]          = c.readEntry("ALT_LIRC_DIGIT_7", "");
    m_AlternativeActions[LIRC_DIGIT_8]          = c.readEntry("ALT_LIRC_DIGIT_8", "");
    m_AlternativeActions[LIRC_DIGIT_9]          = c.readEntry("ALT_LIRC_DIGIT_9", "");
    m_AlternativeActions[LIRC_POWER_ON]         = c.readEntry("ALT_LIRC_POWER_ON",         "");
    m_AlternativeActions[LIRC_POWER_OFF]        = c.readEntry("ALT_LIRC_POWER_OFF",        "TV");
    m_AlternativeActions[LIRC_PAUSE]            = c.readEntry("ALT_LIRC_PAUSE",            "");
    m_AlternativeActions[LIRC_RECORD_START]     = c.readEntry("ALT_LIRC_RECORD_START",     "");
    m_AlternativeActions[LIRC_RECORD_STOP]      = c.readEntry("ALT_LIRC_RECORD_STOP",      "");
    m_AlternativeActions[LIRC_VOLUME_INC]       = c.readEntry("ALT_LIRC_VOLUME_INC",       "");
    m_AlternativeActions[LIRC_VOLUME_DEC]       = c.readEntry("ALT_LIRC_VOLUME_DEC",       "");
    m_AlternativeActions[LIRC_CHANNEL_NEXT]     = c.readEntry("ALT_LIRC_CHANNEL_NEXT",     "");
    m_AlternativeActions[LIRC_CHANNEL_PREV]     = c.readEntry("ALT_LIRC_CHANNEL_PREV",     "");
    m_AlternativeActions[LIRC_SEARCH_NEXT]      = c.readEntry("ALT_LIRC_SEARCH_NEXT",      "");
    m_AlternativeActions[LIRC_SEARCH_PREV]      = c.readEntry("ALT_LIRC_SEARCH_PREV",      "");
    m_AlternativeActions[LIRC_SLEEP]            = c.readEntry("ALT_LIRC_SLEEP",            "");
    m_AlternativeActions[LIRC_APPLICATION_QUIT] = c.readEntry("ALT_LIRC_APPLICATION_QUIT", "");

    QString offmode;
    QString onmode;
    if (c.hasKey("PowerOffMode")) {
        offmode = c.readEntry("PowerOffMode", QString());
        onmode  = c.readEntry("PowerOnMode",  QString());
    } else {
        offmode = c.readEntry("StartupPowerOffMode", QString());
        onmode  = c.readEntry("StartupPowerOnMode",  QString());
    }
    bool at_startup = c.readEntry("LIRCModeSyncAtStartup", true);
    bool at_runtime = c.readEntry("LIRCModeSyncAtRuntime", true);

    setLIRCModeSync(at_startup, at_runtime);
    setPowerOnMode (onmode);
    setPowerOffMode(offmode);

    emit sigUpdateConfig();
}


ConfigPageInfo LircSupport::createConfigurationPage()
{
    LIRCConfiguration *conf = new LIRCConfiguration(NULL, this);
    QObject::connect(this, SIGNAL(sigUpdateConfig()), conf, SLOT(slotUpdateConfig()));
    QObject::connect(this, SIGNAL(sigRawLIRCSignal(const QString &, int, bool &)),
                     conf, SLOT  (slotRawLIRCSignal(const QString &, int, bool &)));
    return ConfigPageInfo (conf,
                           i18n("LIRC Support"),
                           i18n("LIRC Plugin"),
                           "network-wireless");
}


bool LircSupport::checkActions(const QString &lirc_string, int repeat_counter, const QMap<LIRC_Actions, QString> &map)
{
    SoundStreamID streamSinkID = queryCurrentSoundStreamSinkID();

    bool        retval = false;
    bool        q      = false;
    SoundFormat sf;
    ISeekRadio *seeker = NULL;

    QMap<LIRC_Actions, QString>::const_iterator it  = map.begin();
    QMap<LIRC_Actions, QString>::const_iterator end = map.end();
    for (; !retval && it != end; ++it) {
        if ((*it).length() && lirc_string == *it) {
            LIRC_Actions action = it.key();
            int digit = -1;
            switch (action) {
                case LIRC_DIGIT_0 :
                    if (repeat_counter == 0) {
                        digit = 0;
                        retval = true;
                    }
                    break;
                case LIRC_DIGIT_1 :
                    if (repeat_counter == 0) {
                        digit = 1;
                        retval = true;
                    }
                    break;
                case LIRC_DIGIT_2 :
                    if (repeat_counter == 0) {
                        digit = 2;
                        retval = true;
                    }
                    break;
                case LIRC_DIGIT_3 :
                    if (repeat_counter == 0) {
                        digit = 3;
                        retval = true;
                    }
                    break;
                case LIRC_DIGIT_4 :
                    if (repeat_counter == 0) {
                        digit = 4;
                        retval = true;
                    }
                    break;
                case LIRC_DIGIT_5 :
                    if (repeat_counter == 0) {
                        digit = 5;
                        retval = true;
                    }
                    break;
                case LIRC_DIGIT_6 :
                    if (repeat_counter == 0) {
                        digit = 6;
                        retval = true;
                    }
                    break;
                case LIRC_DIGIT_7 :
                    if (repeat_counter == 0) {
                        digit = 7;
                        retval = true;
                    }
                    break;
                case LIRC_DIGIT_8 :
                    if (repeat_counter == 0) {
                        digit = 8;
                        retval = true;
                    }
                    break;
                case LIRC_DIGIT_9 :
                    if (repeat_counter == 0) {
                        digit = 9;
                        retval = true;
                    }
                    break;
                case LIRC_POWER_ON :
                    if (repeat_counter == 0 && !queryIsPowerOn()) {
                        retval = true;
                        sendPowerOn();
                    }
                    break;
                case LIRC_POWER_OFF :
                    if (repeat_counter == 0 && queryIsPowerOn()) {
                        retval = true;
                        sendPowerOff();
                    }
                    break;
                case LIRC_PAUSE :
                    if (repeat_counter == 0 && queryIsPowerOn()) {
                        retval = true;
                        bool paused = false;
                        queryIsPlaybackPaused(streamSinkID, paused);
                        if (paused) {
                            sendResumePlayback(streamSinkID);
                        } else {
                            sendPausePlayback(streamSinkID);
                        }
                    }
                    break;
                case LIRC_RECORD_START :
                    queryIsRecordingRunning(streamSinkID, q = false, sf);
                    if (repeat_counter == 0 && !q) {
                        retval = true;
                        sendStartRecording(streamSinkID);
                    }
                    break;
                case LIRC_RECORD_STOP :
                    queryIsRecordingRunning(streamSinkID, q = false, sf);
                    if (repeat_counter == 0 && q) {
                        retval = true;
                        sendStopRecording(streamSinkID);
                    }
                    break;
                case LIRC_VOLUME_INC :
                    if (queryIsPowerOn()) {
                        retval = true;
                        float oldVolume = 0;
                        queryPlaybackVolume(streamSinkID, oldVolume);
                        sendPlaybackVolume (streamSinkID, oldVolume + 1.0/32.0);
                    }
                    break;
                case LIRC_VOLUME_DEC :
                    if (queryIsPowerOn()) {
                        retval = true;
                        float oldVolume = 0;
                        queryPlaybackVolume(streamSinkID, oldVolume);
                        sendPlaybackVolume (streamSinkID, oldVolume - 1.0/32.0);
                    }
                    break;
                case LIRC_CHANNEL_NEXT :
                    if (repeat_counter == 0 && queryIsPowerOn()) {
                        retval = true;
                        int k = queryCurrentStationIdx() + 1;
                        if (k >= queryStations().count())
                            k = 0;
                        sendActivateStation(k);
                    }
                    break;
                case LIRC_CHANNEL_PREV :
                    if (repeat_counter == 0 && queryIsPowerOn()) {
                        retval = true;
                        int k = queryCurrentStationIdx() - 1;
                        if (k < 0)
                            k = queryStations().count() - 1;
                        sendActivateStation(k);
                    }
                    break;
                case LIRC_SEARCH_NEXT :
                    if (repeat_counter == 0 && queryIsPowerOn()) {
                        retval = true;
                        seeker = dynamic_cast<ISeekRadio*> (queryActiveDevice());
                        if (seeker) {
                            seeker->startSeekUp();
                        }
                    }
                    break;
                case LIRC_SEARCH_PREV :
                    if (repeat_counter == 0 && queryIsPowerOn()) {
                        retval = true;
                        seeker = dynamic_cast<ISeekRadio*> (queryActiveDevice());
                        if (seeker) {
                            seeker->startSeekDown();
                        }
                    }
                    break;
                case LIRC_SLEEP :
                    if (repeat_counter == 0 && queryIsPowerOn()) {
                        retval = true;
                        sendStartCountdown();
                    }
                    break;
                case LIRC_APPLICATION_QUIT :
                    retval = true;
                    qApp->quit();
                    break;
                default:
                    break;
            }

            if (digit >= 0) {
                if (m_addIndex || digit == 0) {
                    activateStation(m_addIndex * 10 + digit);
                    m_kbdTimer->stop();
                    m_addIndex = 0;
                } else {
                    m_addIndex = digit;
                    m_kbdTimer->setSingleShot(true);
                    m_kbdTimer->start(500);
                }
            }
        }
    }
    return retval;
}


void LircSupport::setActions(const QMap<LIRC_Actions, QString> &actions, const QMap<LIRC_Actions, QString> &alt_actions)
{
    m_Actions            = actions;
    m_AlternativeActions = alt_actions;
}



void LircSupport::setPowerOnMode(const QString &m)
{
//     logDebug(QString("LircSupport::setPowerOnMode(%1)").arg(m));
    if (m_LIRCPowerOnMode != m) {
        m_LIRCPowerOnMode = m;
        if (queryIsPowerOn()) {
            setLIRCMode(m);
        }
    }
}

void LircSupport::setPowerOffMode(const QString &m)
{
//     logDebug(QString("LircSupport::setPowerOffMode(%1)").arg(m));
    if (m_LIRCPowerOffMode != m) {
        m_LIRCPowerOffMode = m;
        if (!queryIsPowerOn()) {
            setLIRCMode(m);
        }
    }
}

void LircSupport::setLIRCMode(const QString &m)
{
    if (m_lircConfig && doLIRCModeSync() && m.length()) {
        logDebug(QString("setting lirc mode to %1").arg(m));
        lirc_setmode(m_lircConfig, m.toLocal8Bit().constData());
    } else {
//         logDebug(QString("ignored request for setting lirc mode to %1").arg(m));
    }
}

void LircSupport::setLIRCModeSync(bool at_startup, bool at_runtime)
{
    m_LIRCModeSyncAtStartup = at_startup;
    m_LIRCModeSyncAtRuntime = at_runtime;
}

void LircSupport::startPlugin()
{
    LIRC_init_config();
    noticePowerChanged(queryIsPowerOn());
    m_inStartupPhase = false;
}

bool LircSupport::doLIRCModeSync() const
{
    return (m_inStartupPhase && m_LIRCModeSyncAtStartup) || (!m_inStartupPhase && m_LIRCModeSyncAtRuntime);
}


bool LircSupport::noticePowerChanged(bool on)
{
    if (!m_ignorePowerOnOff) {
        setLIRCMode(on ? m_LIRCPowerOnMode : m_LIRCPowerOffMode);
    }
    return true;
}


