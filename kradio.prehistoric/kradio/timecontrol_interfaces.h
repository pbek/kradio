/***************************************************************************
                          timecontrol_interfaces.h  -  description
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

/***************************************************************************
 *                                                                         *
 *   Interfaces in this header:                                            *
 *                                                                         *
 *   ITimeControl(Client)                                                  *
 *                                                                         *
 ***************************************************************************/


#ifndef KRADIO_RADIO_INTERFACES_H
#define KRADIO_RADIO_INTERFACES_H

#include "interfaces.h"
#include "alarm.h"

INTERFACE(ITimeControl, ITimeControlClient)
{
public :
    virtual int   maxConnections () const { return -1; }

    // receiving commands

    IF_RECEIVER(    setAlarms(const AlarmVector &sl)                 )
    IF_RECEIVER(    setCountdownSeconds(int n)                       )
    IF_RECEIVER(    startCountdown()                                 )
    IF_RECEIVER(    stopCountdown()                                  )
    

    // sending notifications

    IF_SENDER  (    notifyAlarmsChanged(const AlarmVector &sl)       )
    IF_SENDER  (    notifyAlarm(const Alarm &)                       )
    IF_SENDER  (    notifyNextAlarmChanged(const Alarm &)            )
    IF_SENDER  (    notifyCountdownStarted(const QDateTime &end)     )
    IF_SENDER  (    notifyCountdownStopped()                         )
    IF_SENDER  (    notifyCountdownZero()                            )
    

    // answering queries		

    IF_ANSWER  (    QDateTime      getNextAlarmTime ()               )
    IF_ANSWER  (    const Alarm*   getNextAlarm ()                   )
    IF_ANSWER  (    int            getCountdownSeconds ()            )
    IF_ANSWER  (    QDateTime      getCountdownEnd ()                )

};


INTERFACE(ITimeControlClient, ITimeControl)
{
public :
    virtual int   maxConnections () const { return -1; }

    // sending commands

    IF_SENDER  (    setAlarms(const AlarmVector &sl)                 )
    IF_SENDER  (    setCountdownSeconds(int n)                       )
    IF_SENDER  (    startCountdown()                                 )
    IF_SENDER  (    stopCountdown()                                  )


    // receiving notifications

    IF_RECEIVER(    notifyAlarmsChanged(const AlarmVector &sl)       )
    IF_RECEIVER(    notifyAlarm(const Alarm &)                       )
    IF_RECEIVER(    notifyNextAlarmChanged(const Alarm &)            )
    IF_RECEIVER(    notifyCountdownStarted(const QDateTime &end)     )
    IF_RECEIVER(    notifyCountdownStopped()                         )
    IF_RECEIVER(    notifyCountdownZero()                            )


    // queries

    IF_QUERY   (    QDateTime      getNextAlarmTime()                )
    IF_QUERY   (    const Alarm*   getNextAlarm ()                   )
    IF_QUERY   (    int            getCountdownSeconds ()            )
    IF_QUERY   (    QDateTime      getCountdownEnd ()                )

};




#endif
