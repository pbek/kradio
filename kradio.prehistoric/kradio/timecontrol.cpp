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
	  m_waitingFor(NULL),
	  m_countdownSeconds(0),
	  m_alarmTimer(this),
	  m_countdownTimer(this)
{
	QObject::connect(&m_alarmTimer,     SIGNAL(timeout()), this, SLOT(slotQTimerAlarmTimeout()));
	QObject::connect(&m_countdownTimer, SIGNAL(timeout()), this, SLOT(slotQTimerCountdownTimeout()));
}


TimeControl::~TimeControl ()
{
	m_waitingFor = NULL;
}


bool TimeControl::setAlarms (const AlarmVector &al)
{
	m_waitingFor = NULL;

	m_alarms = al;
	
	slotQTimerAlarmTimeout();

	notifyAlarmsChanged(m_alarms);

	return true;
}


bool TimeControl::setCountdownSeconds(int n)
{
	m_countdownSeconds = n;	
	return true;
}


bool TimeControl::startCountdown()
{
	m_countdownEnd = QDateTime::currentDateTime().addSecs(m_countdownSeconds);
	m_countdownTimer.start(m_countdownSeconds * 1000, true);

	notifyCountdownStarted(getCountdownEnd());
	
	return true;
}


bool TimeControl::stopCountdown()
{
	m_countdownTimer.stop();
	m_countdownEnd = QDateTime();

	notifyCountdownStopped();

	return true;
}


QDateTime TimeControl::getNextAlarmTime() const
{
	const Alarm *a = getNextAlarm();
	if (a)
		return a->nextAlarm();
	else
		return QDateTime();
}


const Alarm *TimeControl::getNextAlarm () const
{
	QDateTime now = QDateTime::currentDateTime(),
	          next;

	const Alarm *retval = NULL;

	for (ciAlarmVector i = m_alarms.begin(); i != m_alarms.end(); ++i) {
		QDateTime n = i->nextAlarm();
		if (n.isValid() && n > now && ( ! next.isValid() || n < next)) {
			next = n;
			retval = &(*i);
		}
	}

	QDateTime old = m_nextAlarm_tmp;
	m_nextAlarm_tmp = next;
	if (old != m_nextAlarm_tmp) {
		notifyNextAlarmChanged(retval);
	}

	return retval;
}


QDateTime TimeControl::getCountdownEnd () const
{
	if (m_countdownTimer.isActive())
		return m_countdownEnd;
	else
		return QDateTime();
}





void TimeControl::slotQTimerCountdownTimeout()
{
	stopCountdown();

	notifyCountdownZero();
}


void TimeControl::slotQTimerAlarmTimeout()
{
	if (m_waitingFor) {
		notifyAlarm(*m_waitingFor);
	}

	QDateTime now  = QDateTime::currentDateTime();
	Alarm const *n = getNextAlarm();
    QDateTime na   = getNextAlarmTime();

	m_waitingFor = NULL;
	
	if (na.isValid()) {
	
		int days  = now.daysTo(na);
		int msecs = now.time().msecsTo(na.time());

		if (days > 1) {
			m_alarmTimer.start(24 * 3600 * 1000, true);
			
		} else if (days >= 0) {
		
			if (days > 0)
				msecs += days * 24 * 3600 * 1000;

			if (msecs > 0) {
				m_waitingFor = n;
				m_alarmTimer.start(msecs, true);
			}
		}
	}
}

/*
void    TimeControl::restoreState (KConfig *)
{
}


void    TimeControl::saveState    (KConfig *)
{
}


void    TimeControl::configurationChanged (const SetupData &d)
{
	setCountdownSeconds(d.sleep);
	setAlarms(d.alarms);
}


*/
