/***************************************************************************
                          alarm.cpp  -  description
                             -------------------
    begin                : Mon Feb 4 2002
    copyright            : (C) 2002 by Martin Witte / Frank Schwanz
    email                : witte@kawo1.rwth-aachen.de / schwanz@fh-brandenburg.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <stdio.h>
#include "alarm.h"

Alarm::Alarm(QObject *_parent, QDateTime _time, bool _daily, bool _enabled)
	: QObject (_parent)
{
	daily = _daily;
	enabled = _enabled;
	time = _time;
	done = QDateTime();
	volumePreset = -1;
	stationID = -1;
}


Alarm::Alarm (QObject *_parent)
	: QObject (_parent)
{
	daily = false;
	enabled = false;
	done = QDateTime();
	stationID = -1;
	volumePreset = -1;
	time = QDateTime (QDate(1800, 1,1), QTime(0,0,0));
}


Alarm::Alarm (const Alarm &a)
	: QObject (a.parent(), a.name())
{
	daily        = a.daily;
	enabled      = a.enabled;
	done         = a.done;
	time         = a.time;
	stationID    = a.stationID;
	volumePreset = a.volumePreset;
}


Alarm::Alarm (QObject *_parent, const Alarm &a)
	: QObject (_parent, a.name())
{
	daily   = a.daily;
	enabled = a.enabled;
	done    = a.done;
	time    = a.time;
	stationID    = a.stationID;
	volumePreset = a.volumePreset;
}


Alarm::~Alarm()
{
}


QDateTime Alarm::nextAlarm(bool ignoreEnable) const
{
	QDateTime now = QDateTime::currentDateTime(),
			  alarm = time;
	if (daily) {
		alarm.setDate (now.date());
		if ((done.isValid() && done >= alarm) || alarm.addSecs(60) < now)
			alarm = alarm.addDays(1);
	}
	return (enabled && (!done.isValid() || daily)) || ignoreEnable ? alarm : QDateTime();
}


void Alarm::poll ()
{
	QDateTime now  = QDateTime::currentDateTime(),
			  next = nextAlarm();
	if (next.isValid() && now >= next && now < next.addSecs(60)) {
//		time = time.addDays(daily);
		done = next;
		emit alarm(this);
	}
}

