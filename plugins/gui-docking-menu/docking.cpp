/***************************************************************************
                          docking.cpp  -  description
                             -------------------
    begin                : Don Mär  8 21:57:17 CET 2001
    copyright            : (C) 2002 by Ernst Martin Witte
    email                : emw-kradio@nocabal.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kcmdlineargs.h>
#include <kiconloader.h>
#include <QtGui/QToolTip>
#include <QtGui/QDragEnterEvent>

#include <kmenu.h>
#include <kapplication.h>
#include <kaction.h>
#include <kaboutdata.h>
#include <kconfiggroup.h>
#include <kwindowsystem.h>
#include <kwindowinfo.h>

#include "radiodevice_interfaces.h"
#include "seekradio_interfaces.h"
#include "stationlist.h"
#include "pluginmanager.h"
#include "widgetpluginbase.h"
#include "radiostation.h"
//#include "aboutwidget.h"
#include "station-drag-object.h"

#include "docking.h"
#include "docking-configuration.h"

///////////////////////////////////////////////////////////////////////

PLUGIN_LIBRARY_FUNCTIONS(RadioDocking, PROJECT_NAME, i18n("System Tray Icon/Menu for KRadio"));

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

RadioDocking::RadioDocking(const QString &instanceID, const QString &name)
  : PluginBase(instanceID, name, i18n("System Tray Plugin")),
    m_menu(NULL),
    m_recordingMenu(NULL),
    m_helpMenu(NULL, KCmdLineArgs::aboutData()),
    m_quitID(NULL),
    m_titleID(NULL),
    m_alarmID(NULL),
    m_recordingID(NULL),
    m_recordingMenuAction(NULL),
    m_powerID(NULL),
    m_pauseID(NULL),
    m_sleepID(NULL),
    m_seekfwID(NULL),
    m_seekbwID(NULL),
    m_stationsActionGroup(NULL),
    m_leftClickAction(lcaShowHide),
    m_inMenuAction(false),
    m_scheduleMenuRebuild(false)
{
    m_menuRebuildWorkaroundTimer.setInterval(100);
    m_menuRebuildWorkaroundTimer.setSingleShot(true);
    QObject::connect(&m_menuRebuildWorkaroundTimer, SIGNAL(timeout()), this, SLOT(buildContextMenu()));

    QObject::connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                     this, SLOT(slotActivated(QSystemTrayIcon::ActivationReason)));


    m_menu = new KMenu();
    setContextMenu(m_menu);

    QObject::connect(m_menu, SIGNAL(triggered(QAction *)),
                     this,   SLOT(slotMenuItemActivated(QAction *)));

    buildContextMenu ();

    #ifdef KRADIO_ENABLE_FIXMES
        #warning "FIXME: enable drops again for systray icon"
    #endif
    //setAcceptDrops(true);

    m_WorkaroundRecordingMenuUpdate.setInterval(100);
    m_WorkaroundRecordingMenuUpdate.setSingleShot(true);
    QObject::connect(&m_WorkaroundRecordingMenuUpdate, SIGNAL(timeout()), this, SLOT(slotUpdateRecordingMenu()));
}

RadioDocking::~RadioDocking()
{
}


bool RadioDocking::connectI (Interface *i)
{
    bool a = IRadioClient::connectI(i);
    bool b = ITimeControlClient::connectI(i);
    bool c = IRadioDevicePoolClient::connectI(i);
    bool d = IStationSelection::connectI(i);
    bool e = ISoundStreamClient::connectI(i);
    bool f = PluginBase::connectI(i);
    return a || b || c || d || e || f;
}


bool RadioDocking::disconnectI (Interface *i)
{
    bool a = IRadioClient::disconnectI(i);
    bool b = ITimeControlClient::disconnectI(i);
    bool c = IRadioDevicePoolClient::disconnectI(i);
    bool d = IStationSelection::disconnectI(i);
    bool e = ISoundStreamClient::disconnectI(i);
    bool f = PluginBase::disconnectI(i);
    return a || b || c || d || e || f;
}


void RadioDocking::noticeConnectedI (ISoundStreamServer *s, bool pointer_valid)
{
    ISoundStreamClient::noticeConnectedI(s, pointer_valid);
    if (s && pointer_valid) {
        s->register4_sendStartRecordingWithFormat(this);
        s->register4_sendStopRecording           (this);
        s->register4_sendPausePlayback           (this);
        s->register4_sendResumePlayback          (this);
        s->register4_notifySoundStreamChanged    (this);

        updatePauseMenuItem(/*run query*/true, /*ignored*/false);
        updateTrayIcon(true, true, false, false);
    }
}



bool RadioDocking::setStationSelection(const QStringList &sl)
{
    if (m_stationIDs != sl) {
        m_stationIDs = sl;
        buildContextMenu();
        notifyStationSelectionChanged(m_stationIDs);
    }
    return true;
}


// PluginBase

void   RadioDocking::restoreState (const KConfigGroup &config)
{
    PluginBase::restoreState(config);

    show();
    m_stationIDs.clear();
    int nStations = config.readEntry("nStations", 0);
    for (int i = 1; i <= nStations; ++i) {
        QString s = config.readEntry(QString("stationID-") + QString().setNum(i), QString());
        if (s.length())
            m_stationIDs += s;
    }

    m_leftClickAction = (LeftClickAction)config.readEntry("left_click_action", (int)lcaShowHide);

    buildContextMenu();
    notifyStationSelectionChanged(m_stationIDs);

}


void RadioDocking::saveState (KConfigGroup &config) const
{
    PluginBase::saveState(config);

    config.writeEntry("nStations", m_stationIDs.size());
    int i = 1;
    QStringList::const_iterator end = m_stationIDs.end();
    for (QStringList::const_iterator it = m_stationIDs.begin(); it != end; ++it, ++i) {
        config.writeEntry(QString("stationID-") + QString().setNum(i), *it);
    }
    config.writeEntry("left_click_action", (int)m_leftClickAction);

}


ConfigPageInfo RadioDocking::createConfigurationPage()
{
    DockingConfiguration *conf = new DockingConfiguration(this, NULL);
    connectI (conf);

    QObject::connect(this, SIGNAL(sigLeftClickActionChanged(LeftClickAction)),
                     conf, SLOT(slotLeftClickActionChanged(LeftClickAction)));

    return ConfigPageInfo(
        conf,
        i18n("System Tray Icon"),
        i18n("System Tray Icon Configuration"),
        "kmenuedit"
    );
}

/*AboutPageInfo RadioDocking::createAboutPage()
{*/
/*    KAboutData aboutData("kradio4",
                         NULL,
                         NULL,
                         I18N_NOOP("Docking Menu for KRadio"),
                         KAboutData::License_GPL,
                         "(c) 2002-2005 Martin Witte, Klas Kalass",
                         0,
                         "http://sourceforge.net/projects/kradio",
                         0);
    aboutData.addAuthor("Martin Witte",  "", "emw-kradio@nocabal.de");
    aboutData.addAuthor("Klas Kalass",   "", "klas.kalass@gmx.de");

    return AboutPageInfo(
              new KRadioAboutWidget(aboutData, KRadioAboutWidget::AbtTabbed),
              i18n("Docking Menu"),
              i18n("Docking Menu Plugin"),
              "kmenuedit"
           );*/
//     return AboutPageInfo();
// }



void RadioDocking::buildContextMenu()
{
    if (m_inMenuAction) {
        m_scheduleMenuRebuild = true;
        return;
    }
    m_scheduleMenuRebuild = false;

    m_menu->clear();
    if (m_recordingMenu) {
        delete m_recordingMenu;
    }
    m_recordingMenu = NULL;

    m_titleID  = m_menu->addTitle (generateStationTitle());

    buildStationList();


    m_alarmID  = m_menu->addTitle (generateAlarmTitle());

    m_sleepID  = m_menu->addAction(SmallIcon("kradio_zzz"), "sleep-dummy");
    m_seekfwID = m_menu->addAction(SmallIcon("media-seek-forward"),  i18n("Search Next Station"));
    m_seekbwID = m_menu->addAction(SmallIcon("media-seek-backward"), i18n("Search Previous Station"));
    QObject::connect(m_sleepID,  SIGNAL(triggered()), this, SLOT(slotSleepCountdown()));
    QObject::connect(m_seekfwID, SIGNAL(triggered()), this, SLOT(slotSeekFwd()));
    QObject::connect(m_seekbwID, SIGNAL(triggered()), this, SLOT(slotSeekBkwd()));
    noticeCountdownStarted(queryCountdownEnd());

    // recording menu
    buildRecordingMenu();
    m_recordingMenuAction = m_menu->addMenu(m_recordingMenu);
    m_recordingMenuAction->setText(i18n("Recording"));
    m_recordingMenuAction->setIcon(KIcon("media-record"));


    m_powerID = m_menu->addAction(SmallIcon("media-playback-start"), "power-dummy");
    m_pauseID = m_menu->addAction(SmallIcon("media-playback-pause"), i18n("Pause Radio"));
    QObject::connect(m_powerID,  SIGNAL(triggered()), this, SLOT(slotPower()));
    QObject::connect(m_pauseID,  SIGNAL(triggered()), this, SLOT(slotPause()));
    noticePowerChanged(queryIsPowerOn());

    m_menu->addSeparator();


    // build list of widgets for hide/show items
    if (m_manager) {
        QAction *a = m_menu->addMenu(m_manager->getPluginHideShowMenu());
        a->setText(i18n("Show/Hide Plugins"));
        a->setIcon(KIcon("preferences-plugin"));
    }

    QAction *a = m_menu->addMenu(m_helpMenu.menu());
    a->setIcon(KIcon("help-about"));

    m_menu->addSeparator();
    m_quitID = m_menu->addAction( KIcon("application-exit"), i18n("&Quit" ));
    QObject::connect(m_quitID, SIGNAL(triggered()), kapp, SLOT(quit()) );
}


void RadioDocking::buildStationList()
{
    m_stationMenuIDs.clear();
    if (m_stationsActionGroup) {
        delete m_stationsActionGroup;
    }
    m_stationsActionGroup = new QActionGroup(this);
    m_stationsActionGroup->setExclusive(true);

    const StationList    &sl  = queryStations();
    const RadioStation   &crs = queryCurrentStation();

    int k = 0;
    QStringList::iterator end = m_stationIDs.end();
    for (QStringList::iterator it = m_stationIDs.begin(); it != end; ++it) {
        const RadioStation &rs = sl.stationWithID(*it);

        if (rs.isValid()) {

            ++k;
            QString shortcut = k < 10 ? "&"+QString().setNum(k) : k == 10 ? "1&0" : QString().setNum(k);
            QString name     = rs.name();  name.replace("&", "&&");
            QString item     = shortcut + " " + name;
            QString iconName = rs.iconName();
            QAction *a = m_menu->addAction(item);
            m_stationsActionGroup->addAction(a);
            a->setCheckable(true);
            if (iconName.length()) {
                a->setIcon(KIcon(iconName));
            }
            a->setData(rs.stationID());
            a->setChecked (rs.compare(crs) == 0);

            m_stationMenuIDs.insert(rs.stationID(), a);
        }
      }
}


void RadioDocking::slotSeekFwd()
{
    ISeekRadio *seeker = dynamic_cast<ISeekRadio*>(queryActiveDevice());
    if (seeker)
        seeker->startSeekUp();
}


void RadioDocking::slotSeekBkwd()
{
    ISeekRadio *seeker = dynamic_cast<ISeekRadio*>(queryActiveDevice());
    if (seeker)
        seeker->startSeekUp();
}



void RadioDocking::slotPower()
{
    bool old_inMenuAction = m_inMenuAction;
    m_inMenuAction = true;
    if (queryIsPowerOn()) {
        sendPowerOff();
    } else {
        sendPowerOn();
    }
    if (m_scheduleMenuRebuild && !old_inMenuAction) {
        m_menuRebuildWorkaroundTimer.start();
    }
    m_inMenuAction = old_inMenuAction;
}


void RadioDocking::slotPause()
{
    if (queryIsPowerOn()) {
        bool          paused = false;
        SoundStreamID sink_id     = queryCurrentSoundStreamSinkID();
//         SoundStreamID src_id      = queryCurrentSoundStreamSourceID();
        queryIsPlaybackPaused(sink_id, paused);
        if (paused) {
            sendResumePlayback(sink_id);
        } else {
            sendPausePlayback(sink_id);
        }
    }
}


void RadioDocking::slotSleepCountdown()
{
    if (queryCountdownEnd().isValid()) {
        sendStopCountdown();
    } else {
        sendStartCountdown();
    }
}


bool RadioDocking::noticeNextAlarmChanged(const Alarm */*a*/)
{
    buildContextMenu();
    return true;
}


bool RadioDocking::noticeCountdownStarted(const QDateTime &end)
{
    if (end.isValid()) {
        m_sleepID->setIcon(SmallIcon("kradio_zzz"));
        m_sleepID->setText(i18n("Stop Sleep Countdown (running until %1)", end.toString()));
    } else {
        m_sleepID->setIcon(SmallIcon("kradio_zzz"));
        m_sleepID->setText(i18n("Start Sleep Countdown"));
    }
    return true;
}


bool RadioDocking::noticeCountdownStopped()
{
    m_sleepID->setIcon(SmallIcon("kradio_zzz"));
    m_sleepID->setText(i18n("Start Sleep Countdown"));
    return true;
}


bool RadioDocking::noticeCountdownZero()
{
    m_sleepID->setIcon(SmallIcon("kradio_zzz"));
    m_sleepID->setText(i18n("Start Sleep Countdown"));
    return true;
}


bool RadioDocking::noticePowerChanged(bool on)
{
    m_powerID->setIcon(SmallIcon(on ? "media-playback-stop" : "media-playback-start"));
    m_powerID->setText(on ? i18n("Power Off") : i18n("Power On"));
    m_pauseID->setEnabled(on);
    updateTrayIcon(true, true, false, false);
    return true;
}

bool RadioDocking::noticeCountdownSecondsChanged(int /*n*/)
{
    return false;
}


QString RadioDocking::generateStationTitle() const
{
    const RadioStation &rs = queryCurrentStation();
    QString s = i18n("invalid station");
    if (rs.isValid())
        s = rs.name();
    return i18n("KRadio: %1", s);
}

QString RadioDocking::generateAlarmTitle() const
{
    QDateTime d;
    const Alarm *a = queryNextAlarm();
    if (a) d = a->nextAlarm();

    if (d.isValid()) {
        return i18n("next alarm: %1", a->nextAlarm().toString());
    } else {
        return i18n("<no alarm pending>");
    }
}

bool RadioDocking::noticeStationChanged (const RadioStation &rs, int /*idx*/)
{
    QString s = i18n("invalid station");
    if (rs.isValid())
        s = rs.longName();

    const QString &rt = queryRDSRadioText();
    setToolTip(s + (rt.length() ? "\n" + rt : ""));

    bool r = false;
    SoundFormat sf;
    queryIsRecordingRunning(queryCurrentSoundStreamSinkID(), r, sf);
    m_recordingID->setEnabled(!r);

    m_scheduleMenuRebuild = true;
    return true;
}


bool RadioDocking::noticeStationsChanged(const StationList &/*sl*/)
{
    buildContextMenu();
    return true;
}


void RadioDocking::slotActivated ( QSystemTrayIcon::ActivationReason reason )
{
    switch (reason) {
        case QSystemTrayIcon::Trigger :
            switch (m_leftClickAction) {
                case lcaShowHide :
                    ShowHideWidgetPlugins();
                    // FIXME: [mcamen] According the KDE usability guidelines a left
                    //                 click on the systray icon should show/hide the
                    //                 application window
                    // TODO: [mcamen] Use KSystemtray::toggleActive and friends once we
                    //                depend on KDE 3.3
                    break;
                case lcaPowerOnOff :
                    if (queryIsPowerOn())
                        sendPowerOff();
                    else
                        sendPowerOn();
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}


void RadioDocking::ShowHideWidgetPlugins()
{
    // nothing in cache => hide everything
    if (!m_manager) {
        return;
    }
    m_manager->slotHideRestoreAllWidgetPlugins();
}

void RadioDocking::slotMenuItemActivated(QAction *a)
{
    bool old_inMenuAction = m_inMenuAction;
    m_inMenuAction = true;

    const StationList &sl = queryStations();
    QString stationID = a->data().toString();
    const RadioStation &rs = sl.stationWithID(stationID);
    if (rs.isValid()) {
        const RadioStation &crs = queryCurrentStation();
        // power is off
        //    clicking on other station: switch to station + power on
        //    clicking on same station:  power on
        // power is on
        //    clicking on other station: switch station
        //    clicking on same station: power off
        if (rs.stationID() != crs.stationID()) {
            if (!queryIsPowerOn()) {
                sendPowerOn();
            }
            sendActivateStation(rs);
        } else {
            if (!queryIsPowerOn()) {
                sendPowerOn();
            } else {
                sendPowerOff();
            }
        }
    }


    if (m_scheduleMenuRebuild && !old_inMenuAction) {
        m_menuRebuildWorkaroundTimer.start();
    }
    m_inMenuAction = old_inMenuAction;
}


// ISoundStreamClient

bool RadioDocking::startRecordingWithFormat(
    SoundStreamID      id,
    const SoundFormat &/*proposed_format*/,
    SoundFormat       &/*real_format*/)
{
    if (!id.isValid() || id != queryCurrentSoundStreamSinkID() || m_StreamID2MenuID.contains(id))
        return false;

    QString descr;
    querySoundStreamDescription(id, descr);
    QAction *a = m_recordingMenu->addAction(KIcon("media-record"), i18n("Stop Recording of %1", descr));
    a->setData(QVariant::fromValue(id));
    m_StreamID2MenuID.insert(id, a);

    m_recordingID->setEnabled(false);

    updateTrayIcon(false, true, true, false);
    return false; // this is only a "hook" that does not initiate the recording so don't say that we handled the event
}


bool RadioDocking::stopRecording (SoundStreamID id)
{
    if (!id.isValid() || !m_StreamID2MenuID.contains(id))
        return false;

    if (m_StreamID2MenuID.contains(id)) {
        QAction *a = m_StreamID2MenuID[id];
        m_StreamID2MenuID.remove(id);
        m_WorkaroundRecordingMenuActionsToBeDeleted.append(a);
        m_WorkaroundRecordingMenuUpdate.start();
    }

    if (id == queryCurrentSoundStreamSinkID())
        m_recordingID->setEnabled(true);

    if (!m_StreamID2MenuID.count()) {
        updateTrayIcon(false, true, false, false);
    } else {
        updateTrayIcon(false, true, true, false);
    }

    return false;
}


void RadioDocking::slotRecordingMenu(QAction *a)
{
    const QVariant &data = a->data();
    if (!data.isNull() && data.isValid() && data.canConvert<SoundStreamID>()) {
        SoundStreamID id = data.value<SoundStreamID>();
        sendStopRecording(id);
    }
}

void RadioDocking::slotStartDefaultRecording()
{
    SoundStreamID id = queryCurrentSoundStreamSinkID();
    bool          r  = false;
    SoundFormat   sf;
    queryIsRecordingRunning(id, r, sf);
    if (!r) {
        if (!queryIsPowerOn())
            sendPowerOn();
        sendStartRecording(id);
    }
}


void RadioDocking::updateTrayIcon(bool run_query_rec, bool run_query_pause, bool recording, bool paused)
{
    SoundStreamID sink_id = queryCurrentSoundStreamSinkID();
    bool          running = queryIsPowerOn();
    if (run_query_pause) {
        queryIsPlaybackPaused  (sink_id, paused);
    }
    if (run_query_rec) {
        SoundFormat sf;
        queryIsRecordingRunning(sink_id, recording, sf);
    }
    if (paused) {
        setIcon(KIcon("kradio4_plus_pause"));
    } else if (recording) {
        setIcon(KIcon("kradio4_plus_record"));
    } else if (running) {
        setIcon(KIcon("kradio4_plus_play"));
    } else {
        setIcon(KIcon("kradio4"));
    }
}

void RadioDocking::buildRecordingMenu()
{
    QMap<QString, SoundStreamID> streams;
    queryEnumerateSourceSoundStreams(streams);

    KMenu *m = new KMenu(m_menu);

    m_recordingID = m->addAction(SmallIcon("media-record"), i18n("Start Recording"));
    QObject::connect(m_recordingID, SIGNAL(triggered()),          this, SLOT(slotStartDefaultRecording()));
    QObject::connect(m,             SIGNAL(triggered(QAction *)), this, SLOT(slotRecordingMenu(QAction *)));
    SoundStreamID currentSinkID = queryCurrentSoundStreamSinkID();

    QMap<QString, SoundStreamID>     ::const_iterator end = streams.end();
    for (QMap<QString, SoundStreamID>::const_iterator it  = streams.begin(); it != end; ++it) {

        SoundStreamID id    = *it;
        QString       descr = it.key();
        bool r = false;
        SoundFormat   sf;
        queryIsRecordingRunning(id, r, sf);
        if (r) {
            QAction *a = m->addAction(SmallIcon("media-record"), i18n("Stop Recording of %1", descr));
            m_StreamID2MenuID.insert(id, a);

            if (id == currentSinkID) {
                m_recordingID->setEnabled(false);
            }
        }
    }

    if (m_recordingMenu) {
        delete m_recordingMenu;
    }
    m_recordingMenu = m;
}

bool RadioDocking::pausePlayback(SoundStreamID id)
{
    if (queryCurrentSoundStreamSinkID() == id) {
        updatePauseMenuItem(/*run query*/false, /*pause state*/true);
        updateTrayIcon(true, false, false, true);
    }
    return false; // we always return false to indicate, that this is just a hook to get to know that something was paused
}

bool RadioDocking::resumePlayback(SoundStreamID id)
{
    if (queryCurrentSoundStreamSinkID() == id) {
        updatePauseMenuItem(/*run query*/false, /*pause state*/false);
        updateTrayIcon(true, false, false, false);
    }
    return false; // we always return false to indicate, that this is just a hook to get to know that something was paused
}


void RadioDocking::updatePauseMenuItem(bool run_query, bool known_pause_state)
{
    if (run_query) {
        queryIsPlaybackPaused(queryCurrentSoundStreamSinkID(), known_pause_state);
    }
    if (known_pause_state) {
        m_pauseID->setText(i18n("Resume playback"));
        m_pauseID->setIcon(KIcon("media-playback-start"));
    }
    else {
        m_pauseID->setText(i18n("Pause playback"));
        m_pauseID->setIcon(KIcon("media-playback-pause"));
    }
}



bool RadioDocking::noticeSoundStreamChanged(SoundStreamID id)
{
    if (m_StreamID2MenuID.contains(id)) {
        QAction *a = m_StreamID2MenuID[id];
        QString descr;
        querySoundStreamDescription(id, descr);
        a->setIcon(SmallIcon("media-record"));
        a->setText(i18n("Stop Recording of %1", descr));
        return true;
    }
    return false;
}


void RadioDocking::setLeftClickAction(LeftClickAction action)
{
    if (m_leftClickAction != action) {
        m_leftClickAction = action;
        emit sigLeftClickActionChanged(m_leftClickAction);
    }
}

void RadioDocking::dragEnterEvent(QDragEnterEvent* event)
{
    bool a = StationDragObject::canDecode(event);
    if (a)
        IErrorLogClient::staticLogDebug("contentsDragEnterEvent accepted");
    else
        IErrorLogClient::staticLogDebug("contentsDragEnterEvent rejected");
    if (a) {
        event->accept();
    }
}

void RadioDocking::dropEvent(QDropEvent* event)
{
    QStringList list;

    if ( StationDragObject::decode(event, list) ) {
        QStringList l = getStationSelection();
        for (QList<QString>::const_iterator it = list.begin(); it != list.end(); ++it)
            if (!l.contains(*it))
                l.append(*it);
        setStationSelection(l);
    }
}

bool RadioDocking::noticeRDSRadioTextChanged(const QString &s)
{
    const RadioStation &rs = queryCurrentStation();
    setToolTip(rs.longName() + (s.length() ? "\n" + s : ""));
    return true;
}


void RadioDocking::slotUpdateRecordingMenu()
{
    QAction *a = NULL;
    foreach(a, m_WorkaroundRecordingMenuActionsToBeDeleted) {
        delete a;
    }
    m_WorkaroundRecordingMenuActionsToBeDeleted.clear();
}


#include "docking.moc"
