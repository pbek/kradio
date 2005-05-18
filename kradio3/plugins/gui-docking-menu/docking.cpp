/***************************************************************************
                          docking.cpp  -  description
                             -------------------
    begin                : Don Mär  8 21:57:17 CET 2001
    copyright            : (C) 2002 by Ernst Martin Witte
    email                : witte@kawo1.rwth-aachen.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kiconloader.h>
#include <qtooltip.h>
#include <kpopupmenu.h>
#include <kapplication.h>
#include <kaction.h>
#include <kdialogbase.h>
#include <kaboutdata.h>
#include <kconfig.h>

#include <kradio/interfaces/radiodevice_interfaces.h>
#include <kradio/libkradio/stationlist.h>
#include <kradio/libkradio/pluginmanager.h>
#include <kradio/libkradio/widgetplugins.h>
#include <kradio/radio-stations/radiostation.h>
#include <kradio/libkradio-gui/aboutwidget.h>

#include "docking.h"
#include "docking-configuration.h"

#define POPUP_ID_START_RECORDING_DEFAULT  0
#define POPUP_ID_STOP_RECORDING_BASE      100

///////////////////////////////////////////////////////////////////////

PLUGIN_LIBRARY_FUNCTIONS(RadioDocking, "Tray Menu for KRadio");

/////////////////////////////////////////////////////////////////////////////

RadioDocking::RadioDocking(const QString &name)
  : KSystemTray (NULL, name),
    PluginBase(name, i18n("Docking Plugin")),
    m_pluginMenu(NULL),
    m_recordingMenu(NULL),
    m_NextRecordingMenuID(POPUP_ID_STOP_RECORDING_BASE)
{
    setPixmap(BarIcon("kradio"));

    m_menu = contextMenu();
    QObject::connect(m_menu, SIGNAL(activated(int)),
                     this,   SLOT(slotMenuItemActivated(int)));

    buildContextMenu ();
    show();
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
    if (e) {
        getSoundStreamServer()->register4_sendStartRecordingWithFormat(this);
        getSoundStreamServer()->register4_sendStopRecording           (this);
        getSoundStreamServer()->register4_notifySoundStreamChanged    (this);
    }
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


bool RadioDocking::setStationSelection(const QStringList &sl)
{
    m_stationIDs = sl;
    buildContextMenu();
    notifyStationSelectionChanged(m_stationIDs);
    return true;
}


// PluginBase

void   RadioDocking::restoreState (KConfig *config)
{
    config->setGroup(QString("radiodocking-") + name());

    m_stationIDs.clear();
    int nStations = config->readNumEntry("nStations", 0);
    for (int i = 1; i <= nStations; ++i) {
        QString s = config->readEntry(QString("stationID-") + QString().setNum(i), QString::null);
        if (s.length())
            m_stationIDs += s;
    }

    buildContextMenu();
    notifyStationSelectionChanged(m_stationIDs);
}


void RadioDocking::saveState (KConfig *config) const
{
    config->setGroup(QString("radiodocking-") + name());

    config->writeEntry("nStations", m_stationIDs.size());
    int i = 1;
    for (QStringList::const_iterator it = m_stationIDs.begin(); it != m_stationIDs.end(); ++it, ++i) {
        config->writeEntry(QString("stationID-") + QString().setNum(i), *it);
    }
}


ConfigPageInfo RadioDocking::createConfigurationPage()
{
    DockingConfiguration *conf = new DockingConfiguration(NULL);
    connectI (conf);
    return ConfigPageInfo(
        conf,
        i18n("Docking Menu"),
        i18n("Docking Menu Configuration"),
        "kmenuedit"
    );
}

AboutPageInfo RadioDocking::createAboutPage()
{
    KAboutData aboutData("kradio",
                         NULL,
                         NULL,
                         I18N_NOOP("Docking Menu for KRadio"),
                         KAboutData::License_GPL,
                         "(c) 2002, 2003 Martin Witte, Klas Kalass",
                         0,
                         "http://sourceforge.net/projects/kradio",
                         0);
    aboutData.addAuthor("Martin Witte",  "", "witte@kawo1.rwth-aachen.de");
    aboutData.addAuthor("Klas Kalass",   "", "klas.kalass@gmx.de");

    return AboutPageInfo(
              new KRadioAboutWidget(aboutData, KRadioAboutWidget::AbtTabbed),
              i18n("Docking Menu"),
              i18n("Docking Menu Plugin"),
              "kmenuedit"
           );
}



void RadioDocking::buildContextMenu()
{
    m_menu->clear();
    m_pluginMenu    = NULL;
    m_recordingMenu = NULL;

    m_titleID  = m_menu->insertTitle ("title-dummy");

    buildStationList();

    m_alarmID  = m_menu->insertTitle ("alarm-dummy");
    noticeNextAlarmChanged(queryNextAlarm());

    m_sleepID  = m_menu->insertItem(SmallIcon("kradio-zzz"), "sleep-dummy",
                                    this, SLOT(slotSleepCountdown()));
    noticeCountdownStarted(queryCountdownEnd());

    m_seekfwID = m_menu->insertItem(SmallIcon("forward"), i18n("Search Next Station"),
                                    this, SLOT(slotSeekFwd()));
    m_seekbwID = m_menu->insertItem(SmallIcon("back"),    i18n("Search Previous Station"),
                                    this, SLOT(slotSeekBkwd()));

    // recording menu
    buildRecordingMenu();
    m_menu->insertItem(i18n("Recording"), m_recordingMenu);


    m_powerID = m_menu->insertItem(SmallIcon("kradio_muteoff"), "power-dummy",
                                   this, SLOT(slotPower()));
    m_pauseID = m_menu->insertItem(SmallIcon("player_pause"), "Pause Radio",
                                   this, SLOT(slotPause()));
    noticePowerChanged(queryIsPowerOn());

    m_menu->insertSeparator();

    m_menu->insertItem(SmallIcon("kradio"), i18n("&About"), this, SLOT(slotShowAbout()));

    // build list of widgets for hide/show items
    m_pluginMenu = new KPopupMenu(m_menu);
    if (m_manager) {
        m_manager->addWidgetPluginMenuItems(m_pluginMenu, m_widgetPluginIDs);
        m_menu->insertItem(SmallIcon("kdf"), i18n("Show/Hide Plugins"), m_pluginMenu);
    }

    m_menu->insertSeparator();
    m_menu->insertItem( SmallIcon("exit"), i18n("&Quit" ), kapp, SLOT(quit()) );


    noticeStationChanged(queryCurrentStation(), -1);
}


void RadioDocking::buildStationList()
{
    m_stationMenuIDs.clear();

    const RawStationList  &sl = queryStations().all();
    const RadioStation   &crs = queryCurrentStation();

    int k = 0;
    for (QStringList::iterator it = m_stationIDs.begin(); it != m_stationIDs.end(); ++it) {
        const RadioStation &rs = sl.stationWithID(*it);

        if (rs.isValid()) {

            int id = m_menu->insertItem(QString().setNum(++k) + " " + rs.longName());

            m_stationMenuIDs.push_back(id);
            m_menu->setItemChecked (id, rs.compare(crs) == 0);

        } else {
            m_stationMenuIDs.push_back(-1);
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



void RadioDocking::slotShowAbout()
{
    if (m_manager) {
        KDialogBase *d = m_manager->getAboutDialog();
        if (d) d->show();
    }
}


void RadioDocking::slotPower()
{
    if (queryIsPowerOn()) {
        sendPowerOff();
    } else {
        sendPowerOn();
    }
}


void RadioDocking::slotPause()
{
    if (queryIsPowerOn()) {
        sendPausePlayback(queryCurrentSoundStreamID());
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
    QDateTime d;
    if (a) d = a->nextAlarm();

    if (d.isValid())
        m_menu->changeTitle (m_alarmID, i18n("next alarm: %1").arg(d.toString()));
    else
        m_menu->changeTitle (m_alarmID, i18n("<no alarm pending>"));
    return true;
}


bool RadioDocking::noticeCountdownStarted(const QDateTime &end)
{
    if (end.isValid())
        m_menu->changeItem (m_sleepID, SmallIcon("kradio-zzz"), i18n("Stop Sleep Countdown (running until %1)").arg(end.toString()));
    else
        m_menu->changeItem (m_sleepID, SmallIcon("kradio-zzz"), i18n("Start Sleep Countdown"));
    return true;
}


bool RadioDocking::noticeCountdownStopped()
{
    m_menu->changeItem (m_sleepID, SmallIcon("kradio-zzz"), i18n("Start Sleep Countdown"));
    return true;
}


bool RadioDocking::noticeCountdownZero()
{
    m_menu->changeItem (m_sleepID, SmallIcon("kradio-zzz"), i18n("Start Sleep Countdown"));
    return true;
}


bool RadioDocking::noticePowerChanged(bool on)
{
    m_menu->changeItem(m_powerID, SmallIcon(on ? "kradio_muteon" : "kradio_muteoff"),
                       on ? i18n("Power Off") : i18n("Power On"));
    m_menu->setItemEnabled(m_pauseID, on);
    return true;
}

bool RadioDocking::noticeCountdownSecondsChanged(int /*n*/)
{
    return false;
}



bool RadioDocking::noticeStationChanged (const RadioStation &rs, int /*idx*/)
{
    QString s = i18n("invalid station");
    if (rs.isValid())
        s = rs.longName();

      QToolTip::add(this, s);
    m_menu->changeTitle (m_titleID, "KRadio: " + s);
    // FIXME: title does not change in opened popupmenu

    QValueList<int>::iterator iit = m_stationMenuIDs.begin();
    QStringList::iterator     sit = m_stationIDs.begin();
    for (; iit != m_stationMenuIDs.end(); ++iit, ++sit) {
        if (*iit != -1) {
            bool on = rs.stationID() == *sit;
            m_menu->setItemChecked (*iit, on);
        }
    }

    bool r = false;
    queryIsRecordingRunning(queryCurrentSoundStreamID(), r);
    m_recordingMenu->setItemEnabled(m_recordingID, !r);
    return true;
}


bool RadioDocking::noticeStationsChanged(const StationList &/*sl*/)
{
    buildContextMenu();
    return true;
}


void RadioDocking::mousePressEvent( QMouseEvent *e )
{
    KSystemTray::mousePressEvent(e);

    switch ( e->button() ) {
    case LeftButton:
        if (queryIsPowerOn())
            sendPowerOff();
        else
            sendPowerOn();
        break;
    default:
        // nothing
        break;
    }
}


void RadioDocking::slotMenuItemActivated(int id)
{
    const StationList &sl = queryStations();
    QValueList<int>::iterator iit = m_stationMenuIDs.begin();
    QStringList::iterator     sit = m_stationIDs.begin();
    for (; iit != m_stationMenuIDs.end(); ++iit, ++sit) {
        if (*iit == id) {
            const RadioStation &rs = sl.stationWithID(*sit);
            if (rs.isValid())
                sendActivateStation(rs);
        }
    }
}


void RadioDocking::noticeWidgetPluginShown(WidgetPluginBase *b, bool shown)
{
    if (!m_manager || !b || !m_widgetPluginIDs.contains(b))
        return;
    m_manager->updateWidgetPluginMenuItem(b, m_pluginMenu, m_widgetPluginIDs, shown);
}


void RadioDocking::noticePluginsChanged(const PluginList &/*l*/)
{
    buildContextMenu();
}


void RadioDocking::slotRecording()
{
/*    if (queryIsRecording()) {
         sendStopRecording();
    } else {
        sendStartRecording();
    }*/
}


// ISoundStreamClient

bool RadioDocking::startRecordingWithFormat(
    SoundStreamID      id,
    const SoundFormat &/*proposed_format*/,
    SoundFormat       &/*real_format*/)
{
    if (!id.isValid() || id != queryCurrentSoundStreamID() || m_StreamID2MenuID.contains(id))
        return false;

    QString descr;
    querySoundStreamDescription(id, descr);
    int menu_id = m_NextRecordingMenuID++;
    m_recordingMenu->insertItem(SmallIcon("kradio_record"),
                                i18n("Stop Recording of %1").arg(descr),
                                menu_id);
    m_MenuID2StreamID.insert(menu_id, id);
    m_StreamID2MenuID.insert(id, menu_id);

    if (id == queryCurrentSoundStreamID())
        m_recordingMenu->setItemEnabled(m_recordingID, false);

    return false; // this is only a "hook" that does not initiate the recording so don't say that we handled the event
}


bool RadioDocking::stopRecording (SoundStreamID id)
{
    if (!id.isValid() || !m_StreamID2MenuID.contains(id))
        return false;

    int menu_id = m_StreamID2MenuID[id];
    m_recordingMenu->removeItem(menu_id);
    m_MenuID2StreamID.remove(menu_id);
    m_StreamID2MenuID.remove(id);

    if (id == queryCurrentSoundStreamID())
        m_recordingMenu->setItemEnabled(m_recordingID, true);

    return false;
}


void RadioDocking::slotRecordingMenu(int i)
{
    if (i == POPUP_ID_START_RECORDING_DEFAULT) {
        SoundStreamID id = queryCurrentSoundStreamID();
        bool          r  = false;
        queryIsRecordingRunning(id, r);
        if (!r)
            sendStartRecording(id);
    } else if (m_MenuID2StreamID.contains(i)) {
        sendStopRecording(m_MenuID2StreamID[i]);
    }
}

void RadioDocking::buildRecordingMenu()
{
    QMap<QString, SoundStreamID> streams;
    queryEnumerateSoundStreams(streams);

    KPopupMenu *m = new KPopupMenu(m_menu);

    m_recordingID = m->insertItem(SmallIcon("kradio_record"), i18n("Start Recording"),
                                  POPUP_ID_START_RECORDING_DEFAULT);
    QObject::connect(m, SIGNAL(activated(int)),
                     this, SLOT(slotRecordingMenu(int)));
    SoundStreamID currentID = queryCurrentSoundStreamID();

    for (QMapIterator<QString, SoundStreamID> it = streams.begin(); it != streams.end(); ++it) {

        SoundStreamID id    = *it;
        QString       descr = it.key();
        bool r = false;
        queryIsRecordingRunning(id, r);
        if (r) {
            int menu_id = m_NextRecordingMenuID++;
            m->insertItem(SmallIcon("kradio_record"),
                          i18n("Stop Recording of %1").arg(descr),
                          menu_id);
            m_MenuID2StreamID.insert(menu_id, id);
            m_StreamID2MenuID.insert(id, menu_id);

            if (id == currentID)
                m_recordingMenu->setItemEnabled(m_recordingID, false);
        }
    }
    m_recordingMenu = m;
}


bool RadioDocking::noticeSoundStreamChanged(SoundStreamID id)
{
    if (m_StreamID2MenuID.contains(id)) {
        QString descr;
        querySoundStreamDescription(id, descr);
        m_recordingMenu->changeItem(m_StreamID2MenuID[id],
                                    SmallIcon("kradio_record"),
                                    i18n("Stop Recording of %1").arg(descr));
        return true;
    }
    return false;
}


#include "docking.moc"
