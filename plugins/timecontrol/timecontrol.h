/***************************************************************************
                          timecontrol.h  -  description
                             -------------------
    begin                : Son Jan 12 2003
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

#ifndef KRADIO_TIMECONTROL_H
#define KRADIO_TIMECONTROL_H

#include <QObject>
#include <QTimer>

#include "alarm.h"
#include "pluginbase.h"
#include "timecontrol_interfaces.h"

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
    bool              m_suspendOnSleep;
    QDateTime         m_countdownEnd;

    QTimer            m_alarmTimer;
    QTimer            m_countdownTimer;

    mutable QDateTime m_nextAlarm_tmp;      // used to recognize nextAlarm changes

public:
    TimeControl (const QString &instanceID, const QString &name);
    ~TimeControl();

    virtual QString pluginClassName() const override { return QString::fromLatin1("TimeControl"); }

    virtual bool   connectI    (Interface *i) override;
    virtual bool   disconnectI (Interface *i) override;

    // PluginBase

public:
    virtual void   saveState    (      KConfigGroup &) const override;
    virtual void   restoreState (const KConfigGroup &)       override;
    
    virtual void   updateTimers ();

    virtual ConfigPageInfo  createConfigurationPage() override;


    // ITimeControl Interface methods

RECEIVERS:
    bool setAlarms(const AlarmVector &sl) override;
    bool setCountdownSeconds(int n, bool suspendOnSleep) override;
    bool startCountdown() override;
    bool stopCountdown () override;

ANSWERS:
    QDateTime           getNextAlarmTime    () const override;
    const Alarm*        getNextAlarm        () const override;
    const AlarmVector & getAlarms           () const override { return m_alarms; }
    int                 getCountdownSeconds () const override { return m_countdownSeconds; }
    bool                getSuspendOnSleep   () const override { return m_suspendOnSleep; }
    QDateTime           getCountdownEnd     () const override;


    // slots for receiving timeout messages of timers

protected slots:
    virtual void    slotQTimerAlarmTimeout();
    virtual void    slotQTimerCountdownTimeout();
    virtual void    slotResumingFromSuspend();

};


#endif
