/***************************************************************************
                          timecontrol.cpp  -  description
                             -------------------
    begin                : Son Jan 12 2003
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


#include "timecontrol.h"


#define ALARM_TIMER_INTERVALL 5

TimeControl::TimeControl (QObject *p, const QString &n)
	: QObject(p, n)
{
	alarmTimer = new QTimer(this);
	alarmTimer->start(ALARM_TIMER_INTERVALL * 1000);
	connect(alarmTimer, SIGNAL(timeout()), this, SLOT(slotPoll()));
	countdownSeconds = 0;	
}


TimeControl::~TimeControl ()
{
	for (iAlarmVector i = Alarms.begin(); i != Alarms.end(); ++i)
		delete *i;
}


void TimeControl::setAlarms (const AlarmVector &al)
{
	for (iAlarmVector i = Alarms.begin(); i != Alarms.end(); ++i) {
		delete *i;
	}
	Alarms.clear();

	addAlarms(al);
}


void TimeControl::addAlarms (const AlarmVector &al)
{
	for (ciAlarmVector i = al.begin(); i != al.end(); ++i) {
		Alarm *a = new Alarm (this, **i);
		Alarms.push_back(a);
		connect (a, SIGNAL(alarm(Alarm*)), this, SLOT(slotAlarm(Alarm*)));
		connect (alarmTimer, SIGNAL(timeout()), a, SLOT(poll()));
	}
	emit sigConfigChanged();
}


QDateTime TimeControl::nextAlarm () const
{
	QDateTime now = QDateTime::currentDateTime(),
	          next;
	for (ciAlarmVector i = Alarms.begin(); i != Alarms.end(); ++i) {
		QDateTime n = (*i)->nextAlarm();
		if (n.isValid() && n.addSecs(60) >= now && (!next.isValid() || n < next))
			next = n;
	}
	return next;
}


void TimeControl::slotAlarm(Alarm *a)
{
	emit sigAlarm(a);
}


void TimeControl::setCountdownSeconds(int n)
{
	countdownSeconds = n;
	emit sigConfigChanged();
}


void TimeControl::startCountdown()
{
	countdownStart = QDateTime::currentDateTime();
	emit sigStartCountdown();
}


void TimeControl::stopCountdown()
{
	countdownStart = QDateTime();
	emit sigStopCountdown();
}


void TimeControl::slotPoll()
{
	QDateTime now = QDateTime::currentDateTime();
	if (countdownStart.isValid() && countdownStart.addSecs(countdownSeconds) <= now) {
		stopCountdown();
		emit sigCountdownZero();
	}
}

void TimeControl::startStopCountdown()
{
    if (countdownStart.isValid())
		stopCountdown();
	else
		startCountdown();
}
