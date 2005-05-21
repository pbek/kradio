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

#include "../../src/libkradio/alarm.h"
#include "../../src/libkradio/plugins.h"
#include "../../src/interfaces/timecontrol_interfaces.h"

// well, it has to be a QObject :(  , but only for
// receiving QTimer - timeouts

class TimeControl : public QObject,
                    public PluginBase,
                    public ITimeControl
{
Q_OBJECT
protected:
    AlarmVector       m_alarms;
    Alarm const *     m_waitingFor;         // m_alarmTimer is exactly for this date/time

    int               m_countdownSeconds;   // in seconds
    QDateTime         m_countdownEnd;

    QTimer            m_alarmTimer;
    QTimer            m_countdownTimer;

    mutable QDateTime m_nextAlarm_tmp;      // used to recognize nextAlarm changes

public:
    TimeControl (const QString &name);
    ~TimeControl();

    virtual QString pluginClassName() const { return "TimeControl"; }

    virtual const QString &name() const { return PluginBase::name(); }
    virtual       QString &name()       { return PluginBase::name(); }

    virtual bool   connectI (Interface *i);
    virtual bool   disconnectI (Interface *i);

    // PluginBase

public:
    virtual void   saveState (KConfig *) const;
    virtual void   restoreState (KConfig *);

    virtual ConfigPageInfo  createConfigurationPage();
    virtual AboutPageInfo   createAboutPage();


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


    // slots for receiving timeout messages of timers

protected slots:
    virtual void    slotQTimerAlarmTimeout();
    virtual void    slotQTimerCountdownTimeout();

};


#endif