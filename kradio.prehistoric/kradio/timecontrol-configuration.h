/***************************************************************************
                          timecontro-configuration.h  -  description
                             -------------------
    begin                : Sam Aug 2 2003
    copyright            : (C) 2003 by Martin Witte
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
#ifndef KRADIO_TIMECONTROL_CONFIGURATION_H
#define KRADIO_TIMECONTROL_CONFIGURATION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "timecontrol-configuration-ui.h"
#include "timecontrol_interfaces.h"
#include "radio_interfaces.h"

class TimeControl;
class QWidget;

class TimeControlConfiguration : public TimeControlConfigurationUI,
                                 public ITimeControlClient,
                                 public IRadioClient
{
Q_OBJECT
public :
	TimeControlConfiguration (QWidget *parent);
	~TimeControlConfiguration ();

	bool connect (Interface *i);
	bool disconnect (Interface *i);

// ITimeControlClient
	
	bool noticeAlarmsChanged(const AlarmVector &sl);
	bool noticeAlarm(const Alarm &);
	bool noticeNextAlarmChanged(const Alarm *);
	bool noticeCountdownStarted(const QDateTime &end);
	bool noticeCountdownStopped();
	bool noticeCountdownZero();
    bool noticeCountdownSecondsChanged(int n);

// IRadioClient

	bool noticePowerChanged(bool on);
	bool noticeStationChanged (const RadioStation &, int idx);
	bool noticeStationsChanged(const StationList &sl);

protected slots:

	void slotDailyChanged (bool daily);
	void slotEnabledChanged (bool enable);
	void slotStationChanged (int idx);
	void slotAlarmSelectChanged(int idx);
	void slotDateChanged(const QDate &d);
	void slotTimeChanged(const QTime &d);
	void slotVolumeChanged(int v);

	void slotNewAlarm();
	void slotDeleteAlarm();

	void slotOk();
	void slotCancel();

protected:

	AlarmVector     alarms;
	vector<QString> stationIDs;

	bool ignoreChanges;
};

#endif
