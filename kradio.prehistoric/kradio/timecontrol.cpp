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


TimeControl::TimeControl (QObject *p, const QString &n)
	: QObject(p, n),
	  alarmTimer(this),
	  countdownTimer(this)
{
	connect(&alarmTimer,     SIGNAL(timeout()),          this, SLOT(slotAlarmTimeout()));
	connect(&countdownTimer, SIGNAL(timeout()),          this, SLOT(slotCountdownTimeout()));
	countdownSeconds = 0;
	waitingFor = NULL;
}


TimeControl::~TimeControl ()
{
	waitingFor = NULL;
	for (iAlarmVector i = Alarms.begin(); i != Alarms.end(); ++i)
		delete *i;
}


void TimeControl::setAlarms (const AlarmVector &al)
{
	waitingFor = NULL;
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
	}

	waitingFor = NULL;
	slotAlarmTimeout();

	emit sigConfigChanged();
}


QDateTime TimeControl::nextAlarm (Alarm **save) const
{
	QDateTime now = QDateTime::currentDateTime(),
	          next;
	if (save) *save = 0;
	for (ciAlarmVector i = Alarms.begin(); i != Alarms.end(); ++i) {
		QDateTime n = (*i)->nextAlarm();
		if (n.isValid() && n >= now && (!next.isValid() || n < next)) {
			next = n;
			if (save) *save = *i;
		}
	}
	return next;
}


void TimeControl::setCountdownSeconds(int n)
{
	countdownSeconds = n;
	emit sigConfigChanged();
}


void TimeControl::startCountdown()
{
	countdownEnd = QDateTime::currentDateTime().addSecs(countdownSeconds);
	countdownTimer.start(countdownSeconds * 1000, true);
	emit sigStartCountdown();
}


void TimeControl::stopCountdown()
{
	countdownTimer.stop();
	countdownEnd = QDateTime();
	emit sigStopCountdown();
}


void TimeControl::startStopCountdown()
{
    if (countdownEnd.isValid())
		stopCountdown();
	else
		startCountdown();
}


const QDateTime TimeControl::getCountdownEnd () const
{
	if (countdownTimer.isActive())
		return countdownEnd;
	else
		return QDateTime();
}


void TimeControl::slotCountdownTimeout()
{
	stopCountdown();
	emit sigCountdownZero();
}


void TimeControl::slotAlarmTimeout()
{
	if (waitingFor) {
		waitingFor->raiseAlarm();
		emit sigAlarm(waitingFor);
	}

	Alarm     *n = NULL;
	QDateTime now          = QDateTime::currentDateTime();
    QDateTime na           = nextAlarm(&n);

	waitingFor = NULL;
	if (na.isValid()) {
	
		int days  = now.daysTo(na);
		int msecs = now.time().msecsTo(na.time());

		if (days > 1) {
			alarmTimer.start(24 * 3600 * 1000, true);
			
		} else if (days >= 0) {
		
			if (days > 0)
				msecs += days * 24 * 3600 * 1000;

			if (msecs >= 0) {
				waitingFor = n;
				alarmTimer.start(msecs, true);
			}
		}
	}
}


