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
//#include "countdown.h"

class TimeControl : public QObject
{
Q_OBJECT

protected:
    QTimer          *alarmTimer;

    AlarmVector     Alarms;

    int				countdownSeconds;   // in seconds
    QDateTime		countdownStart;
    

public:
	TimeControl (QObject *parent, const QString &name);
	~TimeControl();


    // alarm management

    virtual void                addAlarms   (const AlarmVector &sl);
    virtual void                setAlarms   (const AlarmVector &sl);
    virtual QDateTime           nextAlarm	() const;
    virtual const AlarmVector   &getAlarms  () const { return Alarms; }


    // sleep (countdown) function

    virtual void setCountdownSeconds (int n);
    virtual int  getCountdownSeconds () const { return countdownSeconds; }
    virtual const QDateTime &getCountdownStart () const { return countdownStart; }
    

public slots:
    virtual void    slotAlarm(Alarm *);

    virtual void    startCountdown();
    virtual void    stopCountdown();
    virtual void    startStopCountdown(); // start if not running, stop if running
    virtual void    slotPoll();

signals:
    void    sigConfigChanged();
    void	sigAlarm(Alarm *);
    void	sigCountdownZero();
    void	sigStartCountdown();
    void	sigStopCountdown();

};


#endif
