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
}


void TimeControl::setAlarms (const AlarmVector &al)
{
	waitingFor = NULL;
	Alarms.clear();
	addAlarms(al);
}


void TimeControl::addAlarms (const AlarmVector &al)
{
	Alarms.insert(Alarms.end(), al.begin(), al.end());

	waitingFor = NULL;
	slotAlarmTimeout();
}


QDateTime TimeControl::nextAlarm (Alarm **save) const
{
	QDateTime now = QDateTime::currentDateTime(),
	          next;
	if (save) *save = 0;
	for (ciAlarmVector i = Alarms.begin(); i != Alarms.end(); ++i) {
		QDateTime n = i->nextAlarm();
		if (n.isValid() && n >= now && (!next.isValid() || n < next)) {
			next = n;
			if (save) *save = &(*i);
		}
	}
	return next;
}


void TimeControl::setCountdownSeconds(int n)
{
	countdownSeconds = n;
}


void TimeControl::startSleepCountdown()
{
	countdownEnd = QDateTime::currentDateTime().addSecs(countdownSeconds);
	countdownTimer.start(countdownSeconds * 1000, true);
	emit sigSleepCountdownStarted(countdownEnd);
}


void TimeControl::stopSleepCountdown()
{
	countdownTimer.stop();
	countdownEnd = QDateTime();
	emit sigSleepCountdownStopped();
}


void TimeControl::startStopSleepCountdown()
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
	emit sigSleepCountdownZero();
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


void    TimeControl::connectInterface(QObjectList &ol)
{
	for (QObject *i = objects.first(); i; i = objects.next()) {
		if (this == i)
			continue;

		// configuration

		quietconnect (i, SIGNAL(sigConfigurationChanged(const SetupData &)),
					  this, SLOT(configurationChanged(const SetupData &)));
        quietconnect (i, SIGNAL(sigSaveState(KConfig *)),
		              this, SLOT(saveState(KConfig *)));
    	quietconnect (i, SIGNAL(sigRestoreState(KConfig *)),
				      this, SLOT(restoreState(KConfig *)));

    	// commands

		quietconnect (i, SIGNAL(sigStartSleepCountdown()),     this, SLOT(startSleepCountdown()));
		quietconnect (i, SIGNAL(sigStopSleepCountdown()),      this, SLOT(stopSleepCountdown()));
		quietconnect (i, SIGNAL(sigStartStopSleepCountdown()), this, SLOT(startStopSleepCountdown()));

		// notifications

        quietconnect (this, SIGNAL(sigAlarm(Alarm*)),                            i, SLOT(alarm(Alarm *)));
        quietconnect (this, SIGNAL(sigSleepCountdownZero()),                     i, SLOT(sleepCountdownZero), ());
        quietconnect (this, SIGNAL(sigSleepCountDownStarted(const QDateTime &)), i, SLOT(sleepCountdownStarted(const QDateTime &)));
        quietconnect (this, SIGNAL(sigSleepCountDownStopped()),                  i, SLOT(sleepCountdownStopped), ());
    }
}

