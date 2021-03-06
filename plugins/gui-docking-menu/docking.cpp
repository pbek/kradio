/***************************************************************************
                          docking.cpp  -  description
                             -------------------
    begin                : Don Mar  8 21:57:17 CET 2001
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

#include <KAboutData>
#include <QtGui/QIcon>
#include <QMouseEvent>
#include <QApplication>

#include <QtWidgets/QMenu>
#include <KAboutData>
#include <kconfiggroup.h>

#include "radiodevice_interfaces.h"
#include "seekradio_interfaces.h"
#include "stationlist.h"
#include "pluginmanager.h"
#include "plugin_configuration_dialog.h"
#include "widgetpluginbase.h"
#include "radiostation.h"

#include "docking.h"
#include "docking-configuration.h"

///////////////////////////////////////////////////////////////////////

static KAboutData aboutData()
{
    KAboutData about("RadioDocking",
                     i18nc("@title", "Docking Menu"),
                     KRADIO_VERSION,
                     i18nc("@title", "System Tray Icon/Menu"),
                     KAboutLicense::LicenseKey::GPL,
                     i18nc("@info:credit", "(c) 2002-2005 Martin Witte, Klas Kalass"),
                     NULL,
                     "http://sourceforge.net/projects/kradio",
                     "emw-kradio@nocabal.de");
    about.addAuthor(i18nc("@info:credit", "Martin Witte"), NULL, "emw-kradio@nocabal.de");
    about.addAuthor(i18nc("@info:credit", "Klas Kalass"),  NULL, "klas.kalass@gmx.de");
    return about;
}

KRADIO_EXPORT_PLUGIN(RadioDocking, aboutData())
#include "docking.moc"


/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

RadioDocking::RadioDocking(const QString &instanceID, const QString &name)
  : PluginBase(instanceID, name, i18n("System Tray Plugin")),
    m_menu(NULL),
    m_recordingMenu(NULL),
    m_helpMenu(NULL, KAboutData::applicationData()),
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
    m_stationsActionGroup(NULL)
//     m_inMenuAction(false),
//     m_scheduleMenuRebuild(false)
{
    // default click actions
    m_ClickActions[Qt::LeftButton]        = staShowHide;
    m_ClickActions[Qt::RightButton]       = staSystrayMenu;
    m_ClickActions[Qt::MidButton]         = staGuiPluginsMenu;
    m_DoubleClickActions[Qt::LeftButton]  = staPowerOnOff;
    m_DoubleClickActions[Qt::RightButton] = staRecord;
    m_DoubleClickActions[Qt::MidButton]   = staNone;
    m_WheelAction                         = swaChangeStation;

//     m_menuRebuildWorkaroundTimer.setInterval(100);
//     m_menuRebuildWorkaroundTimer.setSingleShot(true);
//     QObject::connect(&m_menuRebuildWorkaroundTimer, &QTimer::timeout, this, &RadioDocking::buildContextMenu);

    QObject::connect(this, &RadioDocking::activated,
                     this, &RadioDocking::slotActivated);


    m_menu = new QMenu();
    // do not delete the context menu set by KSystemTrayIcon:
    // as it does not track it using a smart pointer, deleting its
    // context menu will result in crashes later.
    //contextMenu()->deleteLater();
    // strangely the activated signals reach this class 
    // when running remotely on ubuntu, but not when running locally.
    // for now let's add the system tray menu
    setContextMenu(NULL);

    m_cachedNextAlarmString = generateAlarmTitle(queryNextAlarm());

    buildContextMenu ();

//     m_WorkaroundRecordingMenuUpdate.setInterval(100);
//     m_WorkaroundRecordingMenuUpdate.setSingleShot(true);
//     QObject::connect(&m_WorkaroundRecordingMenuUpdate, &QTimer::timeout, this, &RadioDocking::slotUpdateRecordingMenu);
}

RadioDocking::~RadioDocking()
{
    delete m_menu;
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
        QString s = config.readEntry(QString("stationID-") + QString::number(i), QString());
        if (s.length())
            m_stationIDs += s;
    }

    m_ClickActions[Qt::LeftButton]        = (SystrayClickAction)config.readEntry("left_click_action",           (int)m_ClickActions[Qt::LeftButton]);
    m_ClickActions[Qt::RightButton]       = (SystrayClickAction)config.readEntry("right_click_action",          (int)m_ClickActions[Qt::RightButton]);
    m_ClickActions[Qt::MidButton]         = (SystrayClickAction)config.readEntry("mid_click_action",            (int)m_ClickActions[Qt::MidButton]);
    m_DoubleClickActions[Qt::LeftButton]  = (SystrayClickAction)config.readEntry("left_doubleclick_action",     (int)m_DoubleClickActions[Qt::LeftButton]);
    m_DoubleClickActions[Qt::RightButton] = (SystrayClickAction)config.readEntry("right_doubleclick_action",    (int)m_DoubleClickActions[Qt::RightButton]);
    m_DoubleClickActions[Qt::MidButton]   = (SystrayClickAction)config.readEntry("mid_doubleclick_action",      (int)m_DoubleClickActions[Qt::MidButton]);
    m_WheelAction                         = (SystrayWheelAction)config.readEntry("wheel_action",                (int)m_WheelAction);

    buildContextMenu();
    notifyStationSelectionChanged(m_stationIDs);

    setContextMenu(m_ClickActions[Qt::RightButton] == staSystrayMenu ? m_menu : NULL);
    
    emit sigClickActionChanged      (Qt::LeftButton,  m_ClickActions      [Qt::LeftButton] );
    emit sigClickActionChanged      (Qt::RightButton, m_ClickActions      [Qt::RightButton]);
    emit sigClickActionChanged      (Qt::MidButton,   m_ClickActions      [Qt::MidButton]  );
    emit sigDoubleClickActionChanged(Qt::LeftButton,  m_DoubleClickActions[Qt::LeftButton] );
    emit sigDoubleClickActionChanged(Qt::RightButton, m_DoubleClickActions[Qt::RightButton]);
    emit sigDoubleClickActionChanged(Qt::MidButton,   m_DoubleClickActions[Qt::MidButton]  );
    emit sigWheelActionChanged      (m_WheelAction);
}


void RadioDocking::saveState (KConfigGroup &config) const
{
    PluginBase::saveState(config);

    config.writeEntry("nStations", m_stationIDs.size());
    int i = 1;
    QStringList::const_iterator end = m_stationIDs.end();
    for (QStringList::const_iterator it = m_stationIDs.begin(); it != end; ++it, ++i) {
        config.writeEntry(QString("stationID-") + QString::number(i), *it);
    }

    config.writeEntry("left_click_action",           (int)m_ClickActions[Qt::LeftButton]);
    config.writeEntry("right_click_action",          (int)m_ClickActions[Qt::RightButton]);
    config.writeEntry("mid_click_action",            (int)m_ClickActions[Qt::MidButton]);
    config.writeEntry("left_doubleclick_action",     (int)m_DoubleClickActions[Qt::LeftButton]);
    config.writeEntry("right_doubleclick_action",    (int)m_DoubleClickActions[Qt::RightButton]);
    config.writeEntry("mid_doubleclick_action",      (int)m_DoubleClickActions[Qt::MidButton]);
    config.writeEntry("wheel_action",                (int)m_WheelAction);
}


ConfigPageInfo RadioDocking::createConfigurationPage()
{
    DockingConfiguration *conf = new DockingConfiguration(this, NULL);
    connectI (conf);

    QObject::connect(this, &RadioDocking::sigClickActionChanged,
                     conf, &DockingConfiguration::slotClickActionChanged);
    QObject::connect(this, &RadioDocking::sigDoubleClickActionChanged,
                     conf, &DockingConfiguration::slotDoubleClickActionChanged);
    QObject::connect(this, &RadioDocking::sigWheelActionChanged,
                     conf, &DockingConfiguration::slotWheelActionChanged);

    return ConfigPageInfo(
        conf,
        i18n("System Tray Icon"),
        i18n("System Tray Icon Configuration"),
        "kmenuedit"
    );
}

void RadioDocking::buildContextMenu()
{
//     m_WorkaroundRecordingMenuActionsToBeDeleted.clear();

//     if (m_inMenuAction) {
//         m_scheduleMenuRebuild = true;
//         return;
//     }
//     m_scheduleMenuRebuild = false;

    m_menu->clear();
    if (m_recordingMenu) {
        m_recordingMenu->deleteLater();
    }
    m_recordingMenu = NULL;

    m_titleID  = m_menu->addSection (generateStationTitle());

    buildStationList(queryStations());


    m_alarmID  = m_menu->addSection (m_cachedNextAlarmString);

    m_sleepID  = m_menu->addAction(QIcon::fromTheme("kradio5_zzz"), "sleep-dummy");
    m_seekfwID = m_menu->addAction(QIcon::fromTheme("media-seek-forward"),  i18n("Search Next Station"));
    m_seekbwID = m_menu->addAction(QIcon::fromTheme("media-seek-backward"), i18n("Search Previous Station"));
    QObject::connect(m_sleepID,  &QAction::triggered, this, &RadioDocking::slotSleepCountdown);
    QObject::connect(m_seekfwID, &QAction::triggered, this, &RadioDocking::slotSeekFwd);
    QObject::connect(m_seekbwID, &QAction::triggered, this, &RadioDocking::slotSeekBkwd);
    noticeCountdownStarted(queryCountdownEnd());

    // recording menu
    buildRecordingMenu();
    m_recordingMenuAction = m_menu->addMenu(m_recordingMenu);
    m_recordingMenuAction->setText(i18n("Recording"));
    m_recordingMenuAction->setIcon(QIcon::fromTheme("media-record"));


    m_powerID = m_menu->addAction(QIcon::fromTheme("media-playback-start"), "power-dummy");
    m_pauseID = m_menu->addAction(QIcon::fromTheme("media-playback-pause"), i18n("Pause Radio"));
    QObject::connect(m_powerID,  &QAction::triggered, this, &RadioDocking::slotPower);
    QObject::connect(m_pauseID,  &QAction::triggered, this, &RadioDocking::slotPause);
    noticePowerChanged(queryIsPowerOn());

    m_menu->addSeparator();


    // build list of widgets for hide/show items
    if (m_manager) {
        QAction *a = m_menu->addMenu(m_manager->getPluginHideShowMenu());
        a->setText(i18n("Show/Hide Plugins"));
        a->setIcon(QIcon::fromTheme("preferences-plugin"));
    }

    QAction *a = m_menu->addMenu(m_helpMenu.menu());
    a->setIcon(QIcon::fromTheme("help-about"));

    m_menu->addSeparator();
    m_quitID = m_menu->addAction( QIcon::fromTheme("application-exit"), i18n("&Quit" ));
    QObject::connect(m_quitID, &QAction::triggered, qApp, &QApplication::quit);
}


void RadioDocking::buildStationList(const StationList &sl, QAction *before)
{
    m_stationMenuIDs.clear();
    if (!m_stationsActionGroup) {
        m_stationsActionGroup = new QActionGroup(this);
        m_stationsActionGroup->setExclusive(true);
        QObject::connect(m_stationsActionGroup, &QActionGroup::triggered,
                         this,                  &RadioDocking::slotMenuItemActivated);
    }
    const QList<QAction *> actions = m_stationsActionGroup->actions();
    foreach (QAction *a, actions) {
        m_stationsActionGroup->removeAction(a);
    }
    if (before) {
        qDeleteAll(actions);
    }

    const RadioStation   &crs = queryCurrentStation();

    int k = 0;
    foreach (const QString &id, m_stationIDs) {
        const RadioStation &rs = sl.stationWithID(id);

        if (rs.isValid()) {

            ++k;
            QString shortcut = k < 10 ? "&"+QString::number(k) : k == 10 ? "1&0" : QString::number(k);
            QString name     = rs.name();  name.replace("&", "&&");
            QString item     = shortcut + " " + name;
            QString iconName = rs.iconName();
            QAction *a = new QAction(item, m_menu);
            m_menu->insertAction(before, a);
            m_stationsActionGroup->addAction(a);
            a->setCheckable(true);
            if (iconName.length()) {
                a->setIcon(QIcon::fromTheme(iconName));
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
//     bool old_inMenuAction = m_inMenuAction;
//     m_inMenuAction = true;
    if (queryIsPowerOn()) {
        sendPowerOff();
    } else {
        sendPowerOn();
    }
//     buildContextMenu();
//     if (m_scheduleMenuRebuild && !old_inMenuAction) {
//         m_menuRebuildWorkaroundTimer.start();
//     }
//     m_inMenuAction = old_inMenuAction;
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


bool RadioDocking::noticeNextAlarmChanged(const Alarm *a)
{
    const QString newTitle = generateAlarmTitle(a);
    if (newTitle != m_cachedNextAlarmString) {
        m_cachedNextAlarmString = newTitle;
        buildContextMenu();
    }
    return true;
}


bool RadioDocking::noticeCountdownStarted(const QDateTime &end)
{
    if (end.isValid()) {
        m_sleepID->setIcon(QIcon::fromTheme("kradio5_zzz"));
        m_sleepID->setText(i18n("Stop Sleep Countdown (running until %1)", end.toString()));
    } else {
        m_sleepID->setIcon(QIcon::fromTheme("kradio5_zzz"));
        m_sleepID->setText(i18n("Start Sleep Countdown"));
    }
    return true;
}


bool RadioDocking::noticeCountdownStopped()
{
    m_sleepID->setIcon(QIcon::fromTheme("kradio5_zzz"));
    m_sleepID->setText(i18n("Start Sleep Countdown"));
    return true;
}


bool RadioDocking::noticeCountdownZero()
{
    m_sleepID->setIcon(QIcon::fromTheme("kradio5_zzz"));
    m_sleepID->setText(i18n("Start Sleep Countdown"));
    return true;
}


bool RadioDocking::noticePowerChanged(bool on)
{
    m_powerID->setIcon(QIcon::fromTheme(on ? "media-playback-stop" : "media-playback-start"));
    m_powerID->setText(on ? i18n("Power Off") : i18n("Power On"));
    m_pauseID->setEnabled(on);
    updateTrayIcon(true, true, false, false);
    return true;
}

bool RadioDocking::noticeCountdownSecondsChanged(int /*n*/, bool /*suspendOnSleep*/)
{
    return false;
}


QString RadioDocking::generateStationTitle() const
{
    const RadioStation &rs = queryCurrentStation();
    const QString s = rs.isValid() ? rs.name() : i18n("invalid station");
    return i18n("KRadio: %1", s);
}

QString RadioDocking::generateAlarmTitle(const Alarm *a) const
{
    QDateTime d;
    if (a) d = a->nextAlarm();

    if (d.isValid()) {
        return i18n("next alarm: %1", d.toString());
    } else {
        return i18n("<no alarm pending>");
    }
}

bool RadioDocking::noticeStationChanged (const RadioStation &rs, int /*idx*/)
{
    const QString s = rs.isValid() ? rs.longName() : i18n("invalid station");

    const QString &rt = queryRDSRadioText();
    QString tooltip = s;
    if (!rt.isEmpty()) {
        tooltip += QString::fromLatin1("\n") + rt;
    }
    setToolTip(tooltip);

    bool r = false;
    SoundFormat sf;
    queryIsRecordingRunning(queryCurrentSoundStreamSinkID(), r, sf);
    m_recordingID->setEnabled(!r);

    QAction *act = rs.isValid() ? m_stationMenuIDs.value(rs.stationID()) : 0;
    if (act) {
        act->setChecked(true);
    } else {
        act = m_stationsActionGroup->checkedAction();
        if (act) {
            act->setChecked(false);
        }
    }

    m_titleID->setText(generateStationTitle());

    return true;
}


bool RadioDocking::noticeStationsChanged(const StationList &sl)
{
    buildStationList(sl, m_alarmID);
    return true;
}


bool RadioDocking::event(QEvent *e)
{
    bool                res         = false;

    QMouseEvent        *me          = NULL;
    QWheelEvent        *we          = NULL;
    SystrayClickAction  clickAction = staNone;
    SystrayWheelAction  wheelAction = swaNone;
    int                 wheelDir    = 0;

    switch(e->type()) {
        case QEvent::MouseButtonPress:
            me = (QMouseEvent*)e;
            if (m_ClickActions.contains(me->button())) {
                clickAction = m_ClickActions[me->button()];
            }
            break;
        case QEvent::MouseButtonDblClick:
            me = (QMouseEvent*)e;
            if (m_DoubleClickActions.contains(me->button())) {
                clickAction = m_DoubleClickActions[me->button()];
            }
            break;
        case QEvent::Wheel:
            we          = (QWheelEvent*)e;
            wheelDir    = (we->delta() > 0) ? 1 : ((we->delta() < 0) ? -1 : 0);
            wheelAction = m_WheelAction;
            break;
        default:
            break;
    }

    bool clickHandled = handleClickAction(clickAction);
    bool wheelHandled = handleWheelAction(wheelAction, wheelDir);

    if (wheelHandled || clickHandled) {
        e->accept();
        res = true;
    }


    if (!res) {
        res = QSystemTrayIcon::event(e);
    }

    return res;
}


bool RadioDocking::handleWheelAction(SystrayWheelAction wheelAction, int wheelDir)
{
    bool wheelHandled = true;

    switch (wheelAction) {
        case swaNone:
            wheelHandled = false;
            break;
        case swaChangeStation: {
                int k = queryCurrentStationIdx() - wheelDir;
                if (k >= queryStations().count())
                    k = 0;
                else if (k < 0)
                    k = queryStations().count() - 1;
                sendActivateStation(k);
            }
            break;
        case swaChangeVolume: {
                float vol = 0;
                SoundStreamID ssid = queryCurrentSoundStreamSinkID();
                queryPlaybackVolume(ssid, vol);
                sendPlaybackVolume (ssid, vol + wheelDir * 1.0/32.0);
            }
            break;
        case swaChangeFrequency:
            if (wheelDir > 0) {
                slotSeekFwd();
            } else if (wheelDir < 0) {
                slotSeekBkwd();
            }
            break;
        default:
            wheelHandled = false;
            break;
    }
    return wheelHandled;
}


bool RadioDocking::handleClickAction(SystrayClickAction clickAction)
{
    bool clickHandled = true;

    switch (clickAction) {
        case staNone:
            clickHandled = false;
            break;
        case staShowHide:
            ShowHideWidgetPlugins();
            break;
        case staPowerOnOff:
            slotPower();
            break;
        case staPause:
            slotPause();
            break;
        case staRecord: {
                SoundStreamID ssid = queryCurrentSoundStreamSinkID();
                bool          q    = false;
                SoundFormat   sf;
                queryIsRecordingRunning(ssid, q = false, sf);
                if (!q) {
                    if (!queryIsPowerOn())
                        sendPowerOn();
                    sendStartRecording(ssid);
                } else {
                    sendStopRecording(ssid);
                }
            }
            break;
        case staSystrayMenu:
            m_menu->popup(QCursor::pos());
            break;
        case staGuiPluginsMenu:
            m_manager->getPluginHideShowMenu()->popup(QCursor::pos());
            break;
        case staConfigDialog: {
                WidgetPluginBase *w = m_manager ? m_manager->getConfigDialog() : NULL;
                if (w) w->isReallyVisible() ? w->getWidget()->hide() : w->getWidget()->show();
            }
            break;
        default:
            clickHandled = false;
            break;
    }


    return clickHandled;
}


void RadioDocking::slotActivated ( QSystemTrayIcon::ActivationReason reason )
{
    SystrayClickAction  clickAction = staNone;
//     printf("slotActivated called: %i\n", reason);

    switch (reason) {
        case QSystemTrayIcon::Context:
            clickAction = m_ClickActions[Qt::RightButton];
            break;

        case QSystemTrayIcon::DoubleClick:
            clickAction = m_DoubleClickActions[Qt::LeftButton];
            break;

        case QSystemTrayIcon::MiddleClick:
            clickAction = m_ClickActions[Qt::MidButton];
            break;

        case QSystemTrayIcon::Trigger :
            clickAction = m_ClickActions[Qt::LeftButton];
            break;
        default:
            break;
    }
    handleClickAction(clickAction);
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
//     bool old_inMenuAction = m_inMenuAction;
//     m_inMenuAction = true;

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

//     buildContextMenu();
//     if (m_scheduleMenuRebuild && !old_inMenuAction) {
//         m_menuRebuildWorkaroundTimer.start();
//     }
//     m_inMenuAction = old_inMenuAction;
}


// ISoundStreamClient

bool RadioDocking::startRecordingWithFormat(
    SoundStreamID      id,
    const SoundFormat &/*proposed_format*/,
    SoundFormat       &/*real_format*/,
    const recordingTemplate_t &/*filenameTemplate*/)
{
    if (!id.isValid() || id != queryCurrentSoundStreamSinkID() || m_StreamID2MenuID.contains(id))
        return false;

    QString descr;
    querySoundStreamDescription(id, descr);
    QAction *a = m_recordingMenu->addAction(QIcon::fromTheme("media-record"), i18n("Stop Recording of %1", descr));
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
        a->deleteLater();
//         m_WorkaroundRecordingMenuActionsToBeDeleted.append(a);
//         m_WorkaroundRecordingMenuUpdate.start();
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
        setIcon(QIcon::fromTheme("kradio5_plus_pause"));
    } else if (recording) {
        setIcon(QIcon::fromTheme("kradio5_plus_record"));
    } else if (running) {
        setIcon(QIcon::fromTheme("kradio5_plus_play"));
    } else {
        setIcon(QIcon::fromTheme("kradio5"));
    }
}

void RadioDocking::buildRecordingMenu()
{
    QMap<QString, SoundStreamID> streams;
    queryEnumerateSourceSoundStreams(streams);

    QMenu *m = new QMenu(m_menu);

    m_recordingID = m->addAction(QIcon::fromTheme("media-record"), i18n("Start Recording"));
    QObject::connect(m_recordingID, &QAction::triggered,  this, &RadioDocking::slotStartDefaultRecording);
    QObject::connect(m,             &QMenu  ::triggered,  this, &RadioDocking::slotRecordingMenu);
    SoundStreamID currentSinkID = queryCurrentSoundStreamSinkID();

    QMap<QString, SoundStreamID>     ::const_iterator end = streams.constEnd();
    for (QMap<QString, SoundStreamID>::const_iterator it  = streams.constBegin(); it != end; ++it) {

        SoundStreamID id    = *it;
        QString       descr = it.key();
        bool r = false;
        SoundFormat   sf;
        queryIsRecordingRunning(id, r, sf);
        if (r) {
            QAction *a = m->addAction(QIcon::fromTheme("media-record"), i18n("Stop Recording of %1", descr));
            m_StreamID2MenuID.insert(id, a);

            if (id == currentSinkID) {
                m_recordingID->setEnabled(false);
            }
        }
    }

    if (m_recordingMenu) {
        m_recordingMenu->deleteLater();
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
        m_pauseID->setIcon(QIcon::fromTheme("media-playback-start"));
    }
    else {
        m_pauseID->setText(i18n("Pause playback"));
        m_pauseID->setIcon(QIcon::fromTheme("media-playback-pause"));
    }
}



bool RadioDocking::noticeSoundStreamChanged(SoundStreamID id)
{
    if (m_StreamID2MenuID.contains(id)) {
        QAction *a = m_StreamID2MenuID[id];
        QString descr;
        querySoundStreamDescription(id, descr);
        a->setIcon(QIcon::fromTheme("media-record"));
        a->setText(i18n("Stop Recording of %1", descr));
        return true;
    }
    return false;
}


void RadioDocking::setClickAction(Qt::MouseButton btn, SystrayClickAction action)
{
//     IErrorLogClient::staticLogDebug(QString("RadioDocking::setClickAction(%1, %2)").arg(btn).arg(action));
    if (m_ClickActions[btn] != action) {
        m_ClickActions[btn] = action;
        
        setContextMenu(m_ClickActions[Qt::RightButton] == staSystrayMenu ? m_menu : NULL);
        emit sigClickActionChanged(btn, action);
    }
}

void RadioDocking::setDoubleClickAction(Qt::MouseButton btn, SystrayClickAction action)
{
//     IErrorLogClient::staticLogDebug(QString("RadioDocking::setDoubleClickAction(%1, %2)").arg(btn).arg(action));
    if (m_DoubleClickActions[btn] != action) {
        m_DoubleClickActions[btn] = action;
        emit sigDoubleClickActionChanged(btn, action);
    }
}

void RadioDocking::setWheelAction(SystrayWheelAction action)
{
//     IErrorLogClient::staticLogDebug(QString("RadioDocking::setWheelAction(%1)").arg(action));
    if (m_WheelAction != action) {
        m_WheelAction = action;
        emit sigWheelActionChanged(action);
    }
}


bool RadioDocking::noticeRDSRadioTextChanged(const QString &s)
{
    const RadioStation &rs = queryCurrentStation();
    QString tooltip = rs.longName();
    if (!s.isEmpty()) {
        tooltip += QString::fromLatin1("\n") + s;
    }
    setToolTip(tooltip);
    return true;
}


// void RadioDocking::slotUpdateRecordingMenu()
// {
//     qDeleteAll(m_WorkaroundRecordingMenuActionsToBeDeleted);
//     m_WorkaroundRecordingMenuActionsToBeDeleted.clear();
// }


