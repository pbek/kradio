/***************************************************************************
                          timecontrol.h  -  description
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

#ifndef TIMECONTROL_H
#define TIMECONTROL_H
  
#include <qobject.h>
#include "alarm.h"
#include <qtimer.h>

class TimeControl : public QObject
{
Q_OBJECT

protected:
    QTimer          alarmTimer;
    QTimer          countdownTimer;

    AlarmVector     Alarms;
    Alarm           *waitingFor;

    int				countdownSeconds;   // in seconds
    QDateTime		countdownEnd;
    

public:
	TimeControl (QObject *parent, const QString &name);
	~TimeControl();


    // alarm management

    virtual void                addAlarms   (const AlarmVector &sl);
    virtual void                setAlarms   (const AlarmVector &sl);
    virtual QDateTime           nextAlarm	(Alarm **na = NULL) const;
    virtual const AlarmVector   &getAlarms  () const { return Alarms; }


    // sleep (countdown) function

    virtual void setCountdownSeconds (int n);
    virtual int  getCountdownSeconds () const { return countdownSeconds; }
    virtual const QDateTime getCountdownEnd () const;
    

public slots:
    virtual void    startCountdown();
    virtual void    stopCountdown();
    virtual void    startStopCountdown(); // start if not running, stop if running

protected slots:
    virtual void    slotAlarmTimeout();
    virtual void    slotCountdownTimeout();

signals:
    void    sigConfigChanged();
    void	sigAlarm(Alarm *);
    void	sigCountdownZero();
    void	sigStartCountdown();
    void	sigStopCountdown();

};


#endif
