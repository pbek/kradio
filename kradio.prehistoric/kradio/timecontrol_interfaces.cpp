/***************************************************************************
                          timecontrol_interfaces.cpp  -  description
                             -------------------
    begin                : Mon Mär 10 2003
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

#include "timecontrol_interfaces.h"

static AlarmVector emptyAlarms;

// ITimeControl

IF_IMPL_SENDER  (  ITimeControl::notifyAlarmsChanged(const AlarmVector &sl),
				   noticeAlarmsChanged(sl)			                         )

IF_IMPL_SENDER  (  ITimeControl::notifyAlarm(const Alarm &a),
                   noticeAlarm(a)                                            )

IF_IMPL_SENDER  (  ITimeControl::notifyNextAlarmChanged(const Alarm *a),
                   noticeNextAlarmChanged(a)                                 )

IF_IMPL_SENDER  (  ITimeControl::notifyCountdownStarted(const QDateTime &end),
                   noticeCountdownStarted(end)                               )

IF_IMPL_SENDER  (  ITimeControl::notifyCountdownStopped(),
                   noticeCountdownStopped()                                  )

IF_IMPL_SENDER  (  ITimeControl::notifyCountdownZero(),
                   noticeCountdownZero()                                     )

                   
// ITimeControlClient

IF_IMPL_SENDER  (  ITimeControlClient::sendAlarms(const AlarmVector &sl),
                   setAlarms(sl)                                             )
                   
IF_IMPL_SENDER  (  ITimeControlClient::sendCountdownSeconds(int n),
                   setCountdownSeconds(n)                                    )
                   
IF_IMPL_SENDER  (  ITimeControlClient::sendStartCountdown(),
                   startCountdown()                                          )
                   
IF_IMPL_SENDER  (  ITimeControlClient::sendStopCountdown(),
                   stopCountdown()                                           )


IF_IMPL_QUERY   (  QDateTime      ITimeControlClient::queryNextAlarmTime (),
                   getNextAlarmTime(),
                   QDateTime()                                               )

IF_IMPL_QUERY   (  const Alarm *  ITimeControlClient::queryNextAlarm (),
                   getNextAlarm(),
                   NULL                                                      )
                   
IF_IMPL_QUERY   (  const AlarmVector &ITimeControlClient::queryAlarms (),
                   getAlarms(),
                   emptyAlarms                                               )

IF_IMPL_QUERY   (  int            ITimeControlClient::queryCountdownSeconds (),
                   getCountdownSeconds(),
                   30*60                                                     )

IF_IMPL_QUERY   (  QDateTime      ITimeControlClient::queryCountdownEnd (),
                   getCountdownEnd(),
                   QDateTime()                                               )


void ITimeControlClient::noticeConnected(cmplInterface *)
{
	noticeAlarmsChanged(queryAlarms());
	noticeNextAlarmChanged(queryNextAlarm());
	QDateTime end = queryCountdownEnd();
	if (end > QDateTime::currentDateTime())
		noticeCountdownStarted(end);
	else
		noticeCountdownStopped();
}


void ITimeControlClient::noticeDisconnected(cmplInterface *)
{
	noticeAlarmsChanged(queryAlarms());
	noticeNextAlarmChanged(queryNextAlarm());
	QDateTime end = queryCountdownEnd();
	if (end > QDateTime::currentDateTime())
		noticeCountdownStarted(end);
	else
		noticeCountdownStopped();
}


