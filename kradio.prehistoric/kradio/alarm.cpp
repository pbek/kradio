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
	done = false;
}


Alarm::Alarm (QObject *_parent)
	: QObject (_parent)
{
	daily = false;
	enabled = false;
	done = false;
}


Alarm::~Alarm()
{
}


QDateTime Alarm::nextAlarm() const
{
	QDateTime now = QDateTime::currentDateTime(),
			  alarm = time;
	if (daily && alarm < now) {
		alarm.setDate (now.date());
		if (alarm.addSecs(60) < now)
			alarm = alarm.addDays(1);
	}
	const_cast<Alarm*>(this)->time = alarm;
	return enabled && !done ? time : QDateTime();
}


void Alarm::poll ()
{
	QDateTime now = QDateTime::currentDateTime(),
			  next = nextAlarm();	
	if (enabled && !done && next.isValid() && now > next && now < next.addSecs(60)) {
		emit alarm(this);
		time = time.addDays(daily);
		done = !daily;
	}
}

