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

// ITimeControl

IF_IMPL_SENDER  (  ITimeControl::notifyAlarmsChanged(const AlarmVector &sl),
				   notifyAlarmsChanged(sl)			                         )

IF_IMPL_SENDER  (  ITimeControl::notifyAlarm(const Alarm &a),
                   notifyAlarm(a)                                            )

IF_IMPL_SENDER  (  ITimeControl::notifyNextAlarmChanged(const Alarm &a),
                   notifyNextAlarmChanged(a)                                 )

IF_IMPL_SENDER  (  ITimeControl::notifyCountdownStarted(const QDateTime &end),
                   notifyCountdownStarted(end)                               )

IF_IMPL_SENDER  (  ITimeControl::notifyCountdownStopped(),
                   notifyCountdownStopped()                                  )

IF_IMPL_SENDER  (  ITimeControl::notifyCountdownZero(),
                   notifyCountdownZero()                                     )

// ITimeControlClient

IF_IMPL_SENDER  (  ITimeControlClient::setAlarms(const AlarmVector &sl),
                   setAlarms(sl)                                             )
                   
IF_IMPL_SENDER  (  ITimeControlClient::setCountdownSeconds(int n),
                   setCountdownSeconds(n)                                    )
                   
IF_IMPL_SENDER  (  ITimeControlClient::startCountdown(),
                   startCountdown()                                          )
                   
IF_IMPL_SENDER  (  ITimeControlClient::stopCountdown(),
                   stopCountdown()                                           )


IF_IMPL_QUERY   (  QDateTime      ITimeControlClient::getNextAlarmTime (),
                   getNextAlarmTime(),
                   QDateTime()                                               )

IF_IMPL_QUERY   (  const Alarm *  ITimeControlClient::getNextAlarm (),
                   getNextAlarm(),
                   NULL                                                      )
                   
IF_IMPL_QUERY   (  int            ITimeControlClient::getCountdownSeconds (),
                   getCountdownSeconds(),
                   30*60                                                     )

IF_IMPL_QUERY   (  QDateTime      ITimeControlClient::getCountdownEnd (),
                   getCountdownEnd(),
                   QDateTime()                                               )

