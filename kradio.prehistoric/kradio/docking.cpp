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

#include <kdebug.h>
#include <kiconloader.h>
#include <qtooltip.h>
#include <kpopupmenu.h>
#include <kapplication.h>
#include <kaction.h>


#include "docking.h"
#include "stationlist.h"
#include "radiostation.h"
#include "radiodevice_interfaces.h"
#include "docking-configuration.h"
#include "pluginmanager.h"

RadioDocking::RadioDocking(const QString &name)
  : KSystemTray (NULL, name),
    PluginBase(name)
{
	setPixmap(BarIcon("kradio"));
	m_widgetPluginIDs.setAutoDelete(true);

	m_menu = contextMenu();
	QObject::connect(m_menu, SIGNAL(activated(int)),
	                 this, SLOT(slotMenuItemActivated(int)));

	buildContextMenu ();
}

RadioDocking::~RadioDocking()
{
}


bool RadioDocking::connect (Interface *i)
{
	bool a = IRadioClient::connect(i);
	bool b = ITimeControlClient::connect(i);
	bool c = IRadioDevicePoolClient::connect(i);
	bool d = IStationSelection::connect(i);
/*
    if (a) kdDebug() << "V4LRadio: IRadioClient connected\n";
    if (b) kdDebug() << "V4LRadio: ITimeControlClient connected\n";
    if (c) kdDebug() << "V4LRadio: IRadioDevicePoolClient connected\n";
*/
	return a || b || c || d;
}


bool RadioDocking::disconnect (Interface *i)
{
	bool a = IRadioClient::disconnect(i);
	bool b = ITimeControlClient::disconnect(i);
	bool c = IRadioDevicePoolClient::disconnect(i);
	bool d = IStationSelection::disconnect(i);
/*
    if (a) kdDebug() << "V4LRadio: IRadioClientClient disconnected\n";
    if (b) kdDebug() << "V4LRadio: ITimeControlClient disconnected\n";
    if (c) kdDebug() << "V4LRadio: IRadioDevicePoolClient disconnected\n";
*/
	return a || b || c || d;
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

	int nStations = config->readNumEntry("nStations", 0);
	for (int i = 1; i <= nStations; ++i) {
		QString s = config->readEntry(QString("stationID-") + QString().setNum(i), "");
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
	connect (conf);
	return ConfigPageInfo(
		conf,
		"Docking Menu",
		"Docking Menu Configuration",
		"kmenuedit"
	);
}

QWidget *RadioDocking::createAboutPage()
{
	// FIXME
	return NULL;
}



void RadioDocking::buildContextMenu()
{
	m_menu->clear();

	m_titleID  = m_menu->insertTitle ("title-dummy");
	noticeStationChanged(queryCurrentStation(), -1);

	buildStationList();

	m_alarmID  = m_menu->insertTitle ("alarm-dummy");
	noticeNextAlarmChanged(queryNextAlarm());
	
	m_sleepID  = m_menu->insertItem("sleep-dummy", this, SLOT(slotSleepCountdown()));
	noticeCountdownStarted(queryCountdownEnd());

	m_seekfwID = m_menu->insertItem(SmallIcon("forward"), i18n("Search Next Station"),
	                              this, SLOT(slotSeekFwd()));
	m_seekbwID = m_menu->insertItem(SmallIcon("back"),    i18n("Search Previous Station"),
	                              this, SLOT(slotSeekBkwd()));
	// FIXME: no seek callback function yet

	m_powerID = m_menu->insertItem("power-dummy", this, SLOT(slotPower()));
	noticePowerChanged(queryIsPowerOn());

	m_menu->insertSeparator();

	m_menu->insertItem(SmallIcon("kradio"), i18n("&About"), this, SLOT(slotShowAbout()));


	// build list of widgets for hide/show items
	m_widgetPluginIDs.clear();
    if (m_manager) {
		m_menu->insertSeparator();

		const PluginList &plugins = m_manager->plugins();
		for (PluginIterator it(plugins); it.current(); ++it) {
            WidgetPluginBase *b = dynamic_cast<WidgetPluginBase*>(it.current());
            if (!b) continue;
			
			const QString &name = b->name();
			QWidget *w = b->getWidget();
			bool h = w->isHidden();
			
			int id = m_menu->insertItem(QIconSet(SmallIconSet(h ? "1uparrow" : "1downarrow")),
			                            i18n(h ? "show " : "hide ") + name,
			                            w, SLOT(toggleShown()));
			m_widgetPluginIDs.insert(b, new int(id));
		}
	}

	m_menu->insertSeparator();
	m_menu->insertItem( SmallIcon("exit"), i18n("&Quit" ), kapp, SLOT(quit()) );
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
	// FIXME
}


void RadioDocking::slotPower()
{
	if (queryIsPowerOn()) {
		sendPowerOff();
	} else {
		sendPowerOn();
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
		m_menu->changeTitle (m_alarmID, i18n("next alarm: ") + d.toString());
	else
		m_menu->changeTitle (m_alarmID, i18n("<no alarm pending>"));
	return true;
}


bool RadioDocking::noticeCountdownStarted(const QDateTime &end)
{
	if (end.isValid())
		m_menu->changeItem (m_sleepID, i18n("stop sleep (running until ") + end.toString() + ")");
	else
		m_menu->changeItem (m_sleepID, i18n("start sleep countdown"));
	return true;
}


bool RadioDocking::noticeCountdownStopped()
{
	m_menu->changeItem (m_sleepID, i18n("start sleep countdown"));
	return true;
}


bool RadioDocking::noticeCountdownZero()
{
	m_menu->changeItem (m_sleepID, i18n("start sleep countdown"));
	return true;
}


bool RadioDocking::noticePowerChanged(bool on)
{
  	m_menu->changeItem(m_powerID, on ? i18n("Power Off") : i18n("Power On"));
	return true;
}

bool RadioDocking::noticeCountdownSecondsChanged(int /*n*/)
{
	return false;
}



bool RadioDocking::noticeStationChanged (const RadioStation &rs, int /*idx*/)
{
    QString s = "KRadio: invalid station";
    if (rs.isValid())
		s = rs.longName();

  	QToolTip::add(this, s);
    m_menu->changeTitle (m_titleID, i18n("KRadio: ") + s);
    // FIXME: title does not change in opened popupmenu

    QValueList<int>::iterator iit = m_stationMenuIDs.begin();
    QStringList::iterator     sit = m_stationIDs.begin();
	for (; iit != m_stationMenuIDs.end(); ++iit, ++sit) {
		if (*iit != -1) {
			bool on = rs.stationID() == *sit;
			m_menu->setItemChecked (*iit, on);
		}
	}

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
// FIXME: which gui-plugin to toggle ?
//		if (leftMouseTogglesQB) slotToggleQB ();
//		if (leftMouseTogglesUI) slotToggleUI ();
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
	if (!b) return;
	int *id = m_widgetPluginIDs.find(b);
	m_menu->changeItem(*id,
		               QIconSet(SmallIconSet(!shown ? "1uparrow" : "1downarrow")),
	                   i18n(!shown ? "show" : "hide") + " " + b->name());
}


void RadioDocking::noticePluginsChanged(const PluginList &/*l*/)
{
	buildContextMenu();
}
