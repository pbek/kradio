/***************************************************************************
                          docking.h  -  description
                             -------------------
    begin                : Mon Jan 14 2002
    copyright            : (C) 2001, 2002 by Frank Schwanz, Ernst Martin Witte
    email                : schwanz@fh-brandenburg.de, witte@kawo1.rwth-aachen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRADIO_DOCKING_H
#define KRADIO_DOCKING_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ksystemtray.h>
#include <qpixmap.h>

#include "timecontrol_interfaces.h"
#include "radio_interfaces.h"
#include "radiodevicepool_interface.h"
#include "stationselection_interfaces.h"
#include "plugins.h"

class RadioDocking : public KSystemTray,
                     public PluginBase,
                     public IRadioClient,
                     public ITimeControlClient,
                     public IRadioDevicePoolClient,
                     public IStationSelection
{
Q_OBJECT
public:
	RadioDocking (const QString &name);
	virtual ~RadioDocking();

	virtual bool connect (Interface *);
	virtual bool disconnect (Interface *);

	virtual const QString &name() const { return PluginBase::name(); }
	virtual       QString &name()       { return PluginBase::name(); }


	// PluginBase

public:
	virtual void   saveState (KConfig *) const;
	virtual void   restoreState (KConfig *);

	virtual ConfigPageInfo  createConfigurationPage();
	virtual QWidget        *createAboutPage();
    

	// IStationSelection

RECEIVERS:
    bool setStationSelection(const QStringList &sl);

ANSWERS:
    const QStringList & getStationSelection () const { return m_stationIDs; }

	
	// IRadioDevicePoolClient

RECEIVERS:
	bool noticeActiveDeviceChanged(IRadioDevice *)  { return false; }
	bool noticeDevicesChanged(const QPtrList<IRadioDevice> &)  { return false; }

	// ITimeControlClient

RECEIVERS:
    bool noticeAlarmsChanged(const AlarmVector &)   { return false; }
    bool noticeAlarm(const Alarm &)                 { return false; }
    bool noticeNextAlarmChanged(const Alarm *);
    bool noticeCountdownStarted(const QDateTime &/*end*/);
    bool noticeCountdownStopped();
    bool noticeCountdownZero();
    bool noticeCountdownSecondsChanged(int n);


	// IRadioClient

RECEIVERS:
    bool noticePowerChanged(bool on);
    bool noticeStationChanged (const RadioStation &, int idx);
    bool noticeStationsChanged(const StationList &sl);


protected slots:

	void slotSeekFwd();
	void slotSeekBkwd();

	void slotPower();
	void slotSleepCountdown();
	void slotShowAbout();

	void slotMenuItemActivated(int id);

protected:
    void mousePressEvent( QMouseEvent *e );

	void buildContextMenu();
	void buildStationList();


protected:

	KPopupMenu *m_menu;
	QStringList m_stationIDs;

	// menu Item IDs
	int			m_titleID;
	int 		m_alarmID;
	int			m_powerID;
	int			m_sleepID;
	int         m_seekfwID;
	int         m_seekbwID;
	QValueList<int> m_stationMenuIDs;

};

#endif
