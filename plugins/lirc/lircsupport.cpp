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

#ifdef HAVE_LIRC
#include <lirc/lirc_client.h>
#endif

#include <QtCore/QSocketNotifier>
#include <QtCore/QTimer>
#include <QtCore/QFile>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kstandarddirs.h>

#include <lirc/lirc_client.h>

#include "errorlog_interfaces.h"
#include "radiodevice_interfaces.h"
#include "seekradio_interfaces.h"
#include "stationlist.h"
//#include "aboutwidget.h"

#include "lirc-configuration.h"

#define LIRCRC  ".lircrc"

///////////////////////////////////////////////////////////////////////
//// plugin library functions

//#ifdef HAVE_LIRC
PLUGIN_LIBRARY_FUNCTIONS(LircSupport, PROJECT_NAME, i18n("Linux Infrared Control (LIRC) Support"));
//#endif

/////////////////////////////////////////////////////////////////////////////

LircSupport::LircSupport(const QString &instanceID, const QString &name)
    : PluginBase(instanceID, name, i18n("LIRC Plugin"))
{

    logDebug(i18n("initializing kradio lirc plugin"));
    fprintf (stderr, "%s\n", (const char*)i18n("initializing kradio lirc plugin").toUtf8());
    char *prg = (char*)"kradio";

    QString slircrc = getenv("HOME");
    slircrc += "/" LIRCRC;

    QFile  lircrc(slircrc);
    if (!lircrc.exists()) {
        logWarning(i18n("%1 does not exist. File was created with KRadio's default .lircrc proposal", QString(LIRCRC)));
        QString default_lircrc_filename = KStandardDirs::locate("data", "kradio4/default-dot-lircrc");
        QFile   default_lircrc(default_lircrc_filename);
        default_lircrc.open(QIODevice::ReadOnly);
        if (!default_lircrc.error()) {
            lircrc.open(QIODevice::WriteOnly);
            char *buf = new char [default_lircrc.size() + 1];
            default_lircrc.read(buf, default_lircrc.size());
            lircrc.write(buf, default_lircrc.size());
            lircrc.close();
            default_lircrc.close();
            delete buf;
        } else {
            logError(i18n("failed to read file %1", default_lircrc_filename));
        }
    }

    m_fd_lirc = lirc_init(prg, 1);
    m_lirc_notify = 0;
    m_lircConfig  = 0;

    if (m_fd_lirc != -1) {
        if (lirc_readconfig (NULL, &m_lircConfig, NULL) == 0) {
            m_lirc_notify = new QSocketNotifier(m_fd_lirc, QSocketNotifier::Read, this);
            if (m_lirc_notify)
                QObject::connect(m_lirc_notify, SIGNAL(activated(int)), this, SLOT(slotLIRC(int)));

            // check config
            lirc_config_entry *found = NULL;
            for (lirc_config_entry *e = m_lircConfig ? m_lircConfig->first : NULL; e; e = e->next) {
                if (QString(e->prog) == prg) {
                    found = e;
                }
                if ((e->flags & startup_mode) == startup_mode) {
                    m_lircrc_startup_mode = e->change_mode;
                }
            }
            if (!found) {
                logWarning(i18n("There is no entry for kradio in any of your .lircrc files."));
                logWarning(i18n("Please setup your .lircrc files correctly."));
            }

        } else {
            lirc_deinit();
            m_fd_lirc = -1;
        }
    }

    if (m_fd_lirc == -1) {
        logWarning      (i18n("Initializing kradio lirc plugin failed"));
        staticLogWarning(i18n("Initializing kradio lirc plugin failed"));
    } else {
        logDebug      (i18n("Initializing kradio lirc plugin successful"));
        staticLogDebug(i18n("Initializing kradio lirc plugin successful"));
    }

    m_kbdTimer = new QTimer (this);
    QObject::connect (m_kbdTimer, SIGNAL(timeout()), this, SLOT(slotKbdTimedOut()));

    m_addIndex = 0;
}


LircSupport::~LircSupport()
{
    if (m_fd_lirc != -1)
        lirc_deinit();
    if (m_lircConfig)
        lirc_freeconfig(m_lircConfig);
    m_fd_lirc = -1;
    m_lircConfig = 0;
}


void LircSupport::slotLIRC(int /*socket*/ )
{
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
            processLIRCCode(x, is_eventmap, /*is_raw = */ false);
        }
        if (char_count == 0) {
            logDebug("LIRC: decoding raw, lirc_code2char gave no answer");
            processLIRCCode(code, /*is_eventmap = */ false, /*is_raw = */ true);
        }
    }
    else {
        // some error has occurred on the socket => close lirc plugin
        logWarning(i18n("Reading from LIRC socket failed. Disabling LIRC Functions till next start of kradio"));
        delete m_lirc_notify;
        m_lirc_notify = NULL;
    }

    if (code)
        free (code);
}


void LircSupport::processLIRCCode(const QString &c, bool is_eventmap, bool is_raw)
{
    QString x = c;
    int     repeat_counter = 1;

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
    logDebug(QString("LIRC: ") + x);

    emit sigRawLIRCSignal(x, repeat_counter, consumed);

    if (!consumed) {
        if (!checkActions(x, repeat_counter, m_Actions))
            checkActions(x, repeat_counter, m_AlternativeActions);
    }
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

    c.writeEntry("StartupPowerOffMode",       m_LIRCStartupPowerOffMode);
    c.writeEntry("StartupPowerOnMode",        m_LIRCStartupPowerOnMode);
}

void   LircSupport::restoreState (const KConfigGroup &c)
{
    PluginBase::restoreState(c);

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

    QString offmode = c.readEntry("StartupPowerOffMode", QString());
    QString onmode  = c.readEntry("StartupPowerOnMode",  QString());
    setStartupPowerOnMode (onmode);
    setStartupPowerOffMode(offmode);

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


/*AboutPageInfo LircSupport::createAboutPage()
{*/
/*    KAboutData aboutData("kradio4",
                         NULL,
                         NULL,
                         I18N_NOOP("Linux Infrared Remote Control Support for KRadio"),
                         KAboutData::License_GPL,
                         "(c) 2002-2005 Martin Witte",
                         0,
                         "http://sourceforge.net/projects/kradio",
                         0);
    aboutData.addAuthor("Martin Witte",  "", "emw-kradio@nocabal.de");

    return AboutPageInfo(
              new KRadioAboutWidget(aboutData, KRadioAboutWidget::AbtTabbed),
              i18n("LIRC Support"),
              i18n("LIRC Plugin"),
              "connect_creating"
           );*/
//     return AboutPageInfo();
// }


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
                    kapp->quit();
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



void LircSupport::setStartupPowerOnMode(const QString &m)
{
    logDebug(QString("LircSupport::setStartupPowerOnMode(%1)").arg(m));
    if (m_LIRCStartupPowerOnMode != m) {
        m_LIRCStartupPowerOnMode = m;
        if (queryIsPowerOn()) {
            QString x = m.length() ? m : m_lircrc_startup_mode;
            logDebug(QString("setting lirc startup mode (power on) to %1").arg(x));
            lirc_setmode(m_lircConfig, x.toLocal8Bit());
        }
    }
}

void LircSupport::setStartupPowerOffMode(const QString &m)
{
    logDebug(QString("LircSupport::setStartupPowerOffMode(%1)").arg(m));
    if (m_LIRCStartupPowerOffMode != m) {
        m_LIRCStartupPowerOffMode = m;
        if (!queryIsPowerOn()) {
            QString x = m.length() ? m : m_lircrc_startup_mode;
            logDebug(QString("setting lirc startup mode (power off) to %1").arg(x));
            lirc_setmode(m_lircConfig, x.toLocal8Bit());
        }
    }
}


#include "lircsupport.moc"