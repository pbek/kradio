/***************************************************************************
                          shortcuts.cpp  -  description
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

#include "shortcuts.h"
#include "widgetpluginbase.h"

#include <QSocketNotifier>
#include <QTimer>
#include <QFile>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kstandarddirs.h>
#include <kshortcutseditor.h>
#include <kaction.h>
#include <kglobalaccel.h>

#include "errorlog_interfaces.h"
#include "radiodevice_interfaces.h"
#include "seekradio_interfaces.h"
#include "stationlist.h"
#include "radiostation.h"
//#include "aboutwidget.h"

#include "shortcuts-configuration.h"




///////////////////////////////////////////////////////////////////////
//// plugin library functions

PLUGIN_LIBRARY_FUNCTIONS(Shortcuts, PROJECT_NAME, ki18n("Shortcuts Support"));

/////////////////////////////////////////////////////////////////////////////


Shortcuts::Shortcuts(const QString &instanceID, const QString &name)
    : PluginBase(instanceID, name, i18n("Shortcuts Plugin")),
      m_stdCollection    (NULL),
      m_stationCollection(NULL),
      m_stdActions       (NULL),
      m_stationActions   (NULL)
{
    m_stdCollection     = new KActionCollection(this);
    m_stationCollection = new KActionCollection(this);
    m_stdActions        = new KActionCategory(i18n("Standard Actions"), m_stdCollection);
    m_stationActions    = new KActionCategory(i18n("Station Actions"),  m_stationCollection);

    addAction(i18n("Digit 0"),                 SHORTCUT_DIGIT_0,          Qt::Key_0);
    addAction(i18n("Digit 1"),                 SHORTCUT_DIGIT_1,          Qt::Key_1);
    addAction(i18n("Digit 2"),                 SHORTCUT_DIGIT_2,          Qt::Key_2);
    addAction(i18n("Digit 3"),                 SHORTCUT_DIGIT_3,          Qt::Key_3);
    addAction(i18n("Digit 4"),                 SHORTCUT_DIGIT_4,          Qt::Key_4);
    addAction(i18n("Digit 5"),                 SHORTCUT_DIGIT_5,          Qt::Key_5);
    addAction(i18n("Digit 6"),                 SHORTCUT_DIGIT_6,          Qt::Key_6);
    addAction(i18n("Digit 7"),                 SHORTCUT_DIGIT_7,          Qt::Key_7);
    addAction(i18n("Digit 8"),                 SHORTCUT_DIGIT_8,          Qt::Key_8);
    addAction(i18n("Digit 9"),                 SHORTCUT_DIGIT_9,          Qt::Key_9);
    addAction(i18n("Power on"),                SHORTCUT_POWER_ON,         Qt::Key_P);
    addAction(i18n("Power off"),               SHORTCUT_POWER_OFF,        Qt::Key_P | Qt::CTRL);
    addAction(i18n("Pause"),                   SHORTCUT_PAUSE,            Qt::Key_Space);
    addAction(i18n("Start recording"),         SHORTCUT_RECORD_START,     Qt::Key_R);
    addAction(i18n("Stop recording"),          SHORTCUT_RECORD_STOP,      Qt::Key_R | Qt::CTRL);
    addAction(i18n("Increase volume"),         SHORTCUT_VOLUME_INC,       Qt::Key_Up);
    addAction(i18n("Decrease volume"),         SHORTCUT_VOLUME_DEC,       Qt::Key_Down);
    addAction(i18n("Increase Frequency"),      SHORTCUT_FREQ_INC,         Qt::Key_Right | Qt::SHIFT);
    addAction(i18n("Decrease Frequency"),      SHORTCUT_FREQ_DEC,         Qt::Key_Left  | Qt::SHIFT);
    addAction(i18n("Next station"),            SHORTCUT_STATION_NEXT,     Qt::Key_Right);
    addAction(i18n("Previous station"),        SHORTCUT_STATION_PREV,     Qt::Key_Left);
    addAction(i18n("Search next station"),     SHORTCUT_SEARCH_NEXT,      Qt::Key_Right | Qt::CTRL);
    addAction(i18n("Search previous station"), SHORTCUT_SEARCH_PREV,      Qt::Key_Left  | Qt::CTRL);
    addAction(i18n("Sleep"),                   SHORTCUT_SLEEP,            Qt::Key_Z | Qt::CTRL);
    addAction(i18n("Quit KRadio"),             SHORTCUT_APPLICATION_QUIT, Qt::Key_Q | Qt::CTRL);

    QObject::connect(m_stdCollection,     SIGNAL(actionTriggered(QAction *)), this, SLOT(slotActionTriggered (QAction *)));
    QObject::connect(m_stationCollection, SIGNAL(actionTriggered(QAction *)), this, SLOT(slotStationTriggered(QAction *)));

    m_kbdTimer = new QTimer (this);
    QObject::connect (m_kbdTimer, SIGNAL(timeout()), this, SLOT(slotKbdTimedOut()));

    m_addIndex = 0;

}


Shortcuts::~Shortcuts()
{
    while (m_ShortcutsEditors.size()) {
        delete m_ShortcutsEditors.first(); // will be automatically removed from list by destroyed signal
    }
    if (m_stationActions) {
        delete m_stationActions;
        m_stationActions = NULL;
    }
    if (m_stdActions) {
        delete m_stdActions;
        m_stdActions = NULL;
    }
    if (m_stdCollection) {
        delete m_stdCollection;
        m_stdCollection = NULL;
    }
    if (m_stationCollection) {
        delete m_stationCollection;
        m_stationCollection = NULL;
    }
}


void Shortcuts::addAction(const QString &name, ShortcutID id, int key)
{
    KAction *a = m_stdActions->addAction(name);
    a->setData          ((int)id);
    a->setText          (name);
    a->setShortcut      (KShortcut(key));
    a->setGlobalShortcut(KShortcut());
}



void Shortcuts::slotKbdTimedOut()
{
    activateStation (m_addIndex);
    m_addIndex = 0;
}


void Shortcuts::activateStation (int i)
{
    if (! sendActivateStation(i - 1))
        sendActivateStation( (i + 9) % 10);
}


bool Shortcuts::connectI (Interface *i)
{
    bool a = IRadioClient::connectI (i);
    bool b = ITimeControlClient::connectI (i);
    bool c = IRadioDevicePoolClient::connectI (i);
    bool d = PluginBase::connectI(i);
    bool e = ISoundStreamClient::connectI(i);
    return a || b || c || d || e;
}


bool Shortcuts::disconnectI (Interface *i)
{
    bool a = IRadioClient::disconnectI (i);
    bool b = ITimeControlClient::disconnectI (i);
    bool c = IRadioDevicePoolClient::disconnectI (i);
    bool d = PluginBase::disconnectI(i);
    bool e = ISoundStreamClient::disconnectI(i);
    return a || b || c || d || e;
}



void   Shortcuts::saveState (KConfigGroup &c) const
{
    PluginBase::saveState(c);

    m_stdActions    ->collection()->writeSettings(&c);
    m_stationActions->collection()->writeSettings(&c);
}

void   Shortcuts::restoreState (const KConfigGroup &c)
{
    PluginBase::restoreState(c);

    m_stdActions    ->collection()->readSettings(const_cast<KConfigGroup*>(&c));
    m_stationActions->collection()->readSettings(const_cast<KConfigGroup*>(&c));
}


ConfigPageInfo Shortcuts::createConfigurationPage()
{
    ShortcutsConfiguration *tmp = new ShortcutsConfiguration();
    QObject::connect(tmp, SIGNAL(destroyed(QObject *)), this, SLOT(slotConfigPageDestroyed(QObject *)));
    m_ShortcutsEditors.append(tmp);
    updateShortcutsEditor(tmp);

    return ConfigPageInfo (tmp,
                           i18n("Shortcuts"),
                           i18n("Shortcuts Plugin"),
                           "preferences-desktop-keyboard"
                          );
}

void Shortcuts::slotConfigPageDestroyed(QObject *o)
{
    // IMPORTANT: if this slot is called, the descructor of
    // ShortcutsConfiguration is already finished! Thus the object
    // is officially no ShortcutsConfiguration any more, dynamic_cast would fail.
    // But we need only the pointer here. That's ok.
    ShortcutsConfiguration *scc = static_cast<ShortcutsConfiguration*>(o);
    if (o && m_ShortcutsEditors.contains(scc)) {
        m_ShortcutsEditors.removeAll(scc);
    }
}

void Shortcuts::updateShortcutsEditor(ShortcutsConfiguration *c)
{
    if (c) {
        c->clearCollections();
        c->addCollection(m_stdActions    ->collection(), "KRadio4");
        c->addCollection(m_stationActions->collection(), "KRadio4");
    }
}

void Shortcuts::updateShortcutsEditors()
{
    ShortcutsConfiguration *sce = NULL;
    foreach(sce, m_ShortcutsEditors) {
        updateShortcutsEditor(sce);
    }
}

/*AboutPageInfo Shortcuts::createAboutPage()
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


void Shortcuts::slotActionTriggered(QAction *a)
{
    if (!a) {
        return;
    }
    int action = a->data().toInt();

    SoundStreamID streamSinkID = queryCurrentSoundStreamSinkID();

    bool        q      = false;
    SoundFormat sf;
    ISeekRadio *seeker = NULL;

    int digit = -1;
    switch (action) {
        case SHORTCUT_DIGIT_0 :
            digit = 0;
            break;
        case SHORTCUT_DIGIT_1 :
            digit = 1;
            break;
        case SHORTCUT_DIGIT_2 :
            digit = 2;
            break;
        case SHORTCUT_DIGIT_3 :
            digit = 3;
            break;
        case SHORTCUT_DIGIT_4 :
            digit = 4;
            break;
        case SHORTCUT_DIGIT_5 :
            digit = 5;
            break;
        case SHORTCUT_DIGIT_6 :
            digit = 6;
            break;
        case SHORTCUT_DIGIT_7 :
            digit = 7;
            break;
        case SHORTCUT_DIGIT_8 :
            digit = 8;
            break;
        case SHORTCUT_DIGIT_9 :
            digit = 9;
            break;
        case SHORTCUT_POWER_ON :
            if (!queryIsPowerOn()) {
                sendPowerOn();
            }
            break;
        case SHORTCUT_POWER_OFF :
            if (queryIsPowerOn()) {
                sendPowerOff();
            }
            break;
        case SHORTCUT_PAUSE :
            if (queryIsPowerOn()) {
                bool paused = false;
                queryIsPlaybackPaused(streamSinkID, paused);
                if (paused) {
                    sendResumePlayback(streamSinkID);
                } else {
                    sendPausePlayback(streamSinkID);
                }
            }
            break;
        case SHORTCUT_RECORD_START :
            queryIsRecordingRunning(streamSinkID, q = false, sf);
            if (!q) {
                sendStartRecording(streamSinkID);
            }
            break;
        case SHORTCUT_RECORD_STOP :
            queryIsRecordingRunning(streamSinkID, q = false, sf);
            if (q) {
                sendStopRecording(streamSinkID);
            }
            break;
        case SHORTCUT_VOLUME_INC :
            if (queryIsPowerOn()) {
                float oldVolume = 0;
                queryPlaybackVolume(streamSinkID, oldVolume);
                sendPlaybackVolume (streamSinkID, oldVolume + 1.0/32.0);
            }
            break;
        case SHORTCUT_VOLUME_DEC :
            if (queryIsPowerOn()) {
                float oldVolume = 0;
                queryPlaybackVolume(streamSinkID, oldVolume);
                sendPlaybackVolume (streamSinkID, oldVolume - 1.0/32.0);
            }
            break;
        case SHORTCUT_FREQ_INC :
            if (queryIsPowerOn()) {
                float oldfreq = queryFrequency();
                float freqinc = queryScanStep();
                sendFrequency(oldfreq + freqinc);
            }
            break;
        case SHORTCUT_FREQ_DEC :
            if (queryIsPowerOn()) {
                float oldfreq = queryFrequency();
                float freqinc = queryScanStep();
                sendFrequency(oldfreq - freqinc);
            }
            break;
        case SHORTCUT_STATION_NEXT :
            if (queryIsPowerOn()) {
                int k = queryCurrentStationIdx() + 1;
                if (k >= queryStations().count())
                    k = 0;
                sendActivateStation(k);
            }
            break;
        case SHORTCUT_STATION_PREV :
            if (queryIsPowerOn()) {
                int k = queryCurrentStationIdx() - 1;
                if (k < 0)
                    k = queryStations().count() - 1;
                sendActivateStation(k);
            }
            break;
        case SHORTCUT_SEARCH_NEXT :
            if (queryIsPowerOn()) {
                seeker = dynamic_cast<ISeekRadio*> (queryActiveDevice());
                if (seeker) {
                    seeker->startSeekUp();
                }
            }
            break;
        case SHORTCUT_SEARCH_PREV :
            if (queryIsPowerOn()) {
                seeker = dynamic_cast<ISeekRadio*> (queryActiveDevice());
                if (seeker) {
                    seeker->startSeekDown();
                }
            }
            break;
        case SHORTCUT_SLEEP :
            if (queryIsPowerOn()) {
                sendStartCountdown();
            }
            break;
        case SHORTCUT_APPLICATION_QUIT :
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
            m_kbdTimer->start(250);
        }
    }
}


void Shortcuts::noticePluginsChanged(const PluginList &l)
{
    PluginBase *p = NULL;
    foreach(p, l) {
        WidgetPluginBase *w = dynamic_cast<WidgetPluginBase*>(p);
        if (w) {
            m_stdCollection    ->addAssociatedWidget(w->getWidget());
            m_stationCollection->addAssociatedWidget(w->getWidget());
        }
    }
}


void Shortcuts::slotStationTriggered(QAction *a)
{
    if (!a) {
        return;
    }
    QString             StationID = a->data().toString();
    const StationList  &stations  = queryStations();
    const RadioStation &rs        = stations.stationWithID(StationID);
    const RadioStation &crs       = queryCurrentStation();
    QString rs_id  =  rs.stationID();
    QString crs_id = crs.stationID();
    sendActivateStation(rs);
/*    logDebug(QString("Shortcuts::slotStationTriggered: rs: %1, %2").arg(rs.longName()).arg(rs.stationID()));
    logDebug(QString("Shortcuts::slotStationTriggered: crs: %1, %2").arg(crs.longName()).arg(crs.stationID()));*/
    if (rs_id == crs_id) {
        if (queryIsPowerOn()) {
/*            logDebug("Shortcuts::slotStationTriggered: eqID, sendPowerOff()");*/
            sendPowerOff();
        } else {
/*            logDebug("Shortcuts::slotStationTriggered: eqID, sendPowerOn()");*/
            sendPowerOn();
        }
    } else {
        if (!queryIsPowerOn()) {
/*            logDebug("Shortcuts::slotStationTriggered: neqID, sendPowerOn()");*/
            sendPowerOn();
        }
    }
}


bool Shortcuts::noticeStationsChanged(const StationList &sl)
{
    QMap<QString, KAction *> oldActions;
    QAction *_a = NULL;
    foreach(_a, m_stationCollection->actions()) {
        KAction *a = dynamic_cast<KAction*>(_a);
        if (a) {
            m_stationCollection->takeAction(a);
            QString strdata = a->data().toString();
//             logDebug("Action Data: " + strdata + "\n");
            oldActions.insert(strdata, a);
        }
    }
    m_stationCollection->clear();

    int idx = 1;
    for (StationList::const_iterator it = sl.begin(); it != sl.end(); ++it, ++idx) {
        const RadioStation *s  = *it;
        const QString      &id = s->stationID();

        KAction *a = NULL;
        if (oldActions.contains(id)) {
            a = oldActions[id];
            m_stationActions->addAction(id, a);
            oldActions.remove(id);
        } else {
            KAction *_a = m_stationActions->addAction(id);
            _a->setGlobalShortcut(KShortcut());
            a = _a;
        }
        a->setData(id);
        a->setText(QString().sprintf("%02i) ", idx) + s->name());
        if (s->iconName().length()) {
            a->setIcon(KIcon(s->iconName()));
        }
    }
    KAction *a = NULL;
    foreach(a, oldActions.values()) {
        delete a;
    }
    updateShortcutsEditors();

    return true;
}

#include "shortcuts.moc"
