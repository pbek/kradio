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

#ifndef KRADIO_TIMECONTROL_H
#define KRADIO_TIMECONTROL_H

#include <qobject.h>
#include <qtimer.h>
  
#include "alarm.h"
#include "timecontrol_interfaces.h"

// well, it has to be a QObject :(  , but only for
// receiving QTimer - timeouts

class TimeControl : public QObject, public ITimeControl
{
protected:
    AlarmVector       m_alarms;
    Alarm const *     m_waitingFor;         // m_alarmTimer is exactly for this date/time

    int               m_countdownSeconds;   // in seconds
    QDateTime		  m_countdownEnd;

    QTimer            m_alarmTimer;
    QTimer            m_countdownTimer;

    mutable QDateTime m_nextAlarm_tmp;      // used to recognize nextAlarm changes
    
public:
	TimeControl (QObject *parent, const QString &name);
	~TimeControl();

    // ITimeControl Interface methods
	
RECEIVERS:
    bool setAlarms(const AlarmVector &sl);
    bool setCountdownSeconds(int n);
    bool startCountdown();
    bool stopCountdown();

ANSWERS:
    QDateTime           getNextAlarmTime () const;
    const Alarm*        getNextAlarm () const;
    const AlarmVector & getAlarms () const { return m_alarms; }
    int                 getCountdownSeconds () const { return m_countdownSeconds; }
    QDateTime           getCountdownEnd () const;


    // QT part for signals/slots
    
public slots:
/*
    // interface connection slot

    virtual void    connectPlugin(QObjectList &otherPlugins);

    // configuration slots
    
	virtual void    restoreState (KConfig *c);
	virtual void    saveState    (KConfig *c);
    virtual void    configurationChanged (const SetupData &sud);
*/


	// slots for receiving timeout messages of timers
	
protected slots:
    virtual void    slotQTimerAlarmTimeout();
    virtual void    slotQTimerCountdownTimeout();

};


#endif
