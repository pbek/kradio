/***************************************************************************
                          timecontrol_interfaces.h  -  description
                             -------------------
    begin                : Mon M�r 10 2003
    copyright            : (C) 2003 by Martin Witte
    email                : emw-kradio@nocabal.de
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


#ifndef KRADIO_TIMECONTROL_INTERFACES_H
#define KRADIO_TIMECONTROL_INTERFACES_H

#include "interfaces.h"
#include "alarm.h"

INTERFACE(ITimeControl, ITimeControlClient)
{
public :
    IF_CON_DESTRUCTOR(ITimeControl, -1)

RECEIVERS:
    IF_RECEIVER(    setAlarms(const AlarmVector &sl)                 )
    IF_RECEIVER(    setCountdownSeconds(int n, bool suspendOnSleep)  )
    IF_RECEIVER(    startCountdown()                                 )
    IF_RECEIVER(    stopCountdown()                                  )


SENDERS:
    IF_SENDER  (    notifyAlarmsChanged(const AlarmVector &sl)       )
    IF_SENDER  (    notifyAlarm(const Alarm &)                       )
    IF_SENDER  (    notifyNextAlarmChanged(const Alarm *)            )
    IF_SENDER  (    notifyCountdownStarted(const QDateTime &end)     )
    IF_SENDER  (    notifyCountdownStopped()                         )
    IF_SENDER  (    notifyCountdownZero()                            )
    IF_SENDER  (    notifyCountdownSecondsChanged(int n, bool suspendOnSleep) )


ANSWERS:
    IF_ANSWER  (    QDateTime           getNextAlarmTime () const    )
    IF_ANSWER  (    const Alarm*        getNextAlarm () const        )
    IF_ANSWER  (    const AlarmVector & getAlarms () const           )
    IF_ANSWER  (    int                 getCountdownSeconds () const )
    IF_ANSWER  (    bool                getSuspendOnSleep() const    )
    IF_ANSWER  (    QDateTime           getCountdownEnd () const     )

};


INTERFACE(ITimeControlClient, ITimeControl)
{
public :
    IF_CON_DESTRUCTOR(ITimeControlClient, 1)

SENDERS:
    IF_SENDER  (    sendAlarms(const AlarmVector &sl)                )
    IF_SENDER  (    sendCountdownSeconds(int n, bool suspendOnSleep) )
    IF_SENDER  (    sendStartCountdown()                             )
    IF_SENDER  (    sendStopCountdown()                              )


RECEIVERS:
    IF_RECEIVER(    noticeAlarmsChanged(const AlarmVector &sl)       )
    IF_RECEIVER(    noticeAlarm(const Alarm &)                       )
    IF_RECEIVER(    noticeNextAlarmChanged(const Alarm *)            )
    IF_RECEIVER(    noticeCountdownStarted(const QDateTime &end)     )
    IF_RECEIVER(    noticeCountdownStopped()                         )
    IF_RECEIVER(    noticeCountdownZero()                            )
    IF_RECEIVER(    noticeCountdownSecondsChanged(int n, bool suspendOnSleep) )


QUERIES:
    IF_QUERY   (    QDateTime           queryNextAlarmTime()         )
    IF_QUERY   (    const Alarm*        queryNextAlarm ()            )
    IF_QUERY   (    const AlarmVector & queryAlarms ()               )
    IF_QUERY   (    int                 queryCountdownSeconds ()     )
    IF_QUERY   (    bool                querySuspendOnSleep ()       )
    IF_QUERY   (    QDateTime           queryCountdownEnd ()         )

RECEIVERS:
    virtual void noticeConnectedI    (cmplInterface *, bool /*pointer_valid*/) override;
    virtual void noticeDisconnectedI (cmplInterface *, bool /*pointer_valid*/) override;
};




#endif
