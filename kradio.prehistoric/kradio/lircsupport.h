/***************************************************************************
                          lircsupport.h  -  description
                             -------------------
    begin                : Mon Feb 4 2002
    copyright            : (C) 2002 by Martin Witte / Frank Schwanz
    email                : witte@kawo1.rwth-aachen.de / schwanz@fh-brandenburg.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LIRCSUPPORT_H
#define LIRCSUPPORT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qobject.h>
#include "timecontrol_interfaces.h"
#include "radio_interfaces.h"
#include "radiodevicepool_interface.h"
#include "plugins.h"


struct lirc_config;
class QSocketNotifier;
class QTimer;

class LircSupport : public QObject,
                    public PluginBase,
                    public IRadioClient,
                    public ITimeControlClient,
                    public IRadioDevicePoolClient
{
Q_OBJECT
public:
    LircSupport(const QString &name);
    ~LircSupport();

    virtual bool connectI (Interface *);
    virtual bool disconnectI (Interface *);

    virtual const QString &name() const { return PluginBase::name(); }
    virtual       QString &name()       { return PluginBase::name(); }

    // PluginBase

public:
    virtual void   saveState (KConfig *) const;
    virtual void   restoreState (KConfig *);

    virtual ConfigPageInfo  createConfigurationPage();
    virtual AboutPageInfo   createAboutPage();

    // IRadioClient methods

RECEIVERS:
    bool noticePowerChanged(bool /*on*/)                          { return false; }
    bool noticeStationChanged (const RadioStation &, int /*idx*/) { return false; }
    bool noticeStationsChanged(const StationList &/*sl*/)         { return false; }
    bool noticePresetFileChanged(const QString &/*f*/)            { return false; }


    // ITimeControlClient

RECEIVERS:
    bool noticeAlarmsChanged(const AlarmVector &)      { return false; }
    bool noticeAlarm(const Alarm &)                    { return false; }
    bool noticeNextAlarmChanged(const Alarm *)         { return false; }
    bool noticeCountdownStarted(const QDateTime &/*end*/) { return false; }
    bool noticeCountdownStopped()                      { return false; }
    bool noticeCountdownZero()                         { return false; }
    bool noticeCountdownSecondsChanged(int /*n*/)      { return false; }

    // IRadioDevicePoolClient

RECEIVERS:
    bool noticeActiveDeviceChanged(IRadioDevice *)     { return false; }
    bool noticeDevicesChanged(const QPtrList<IRadioDevice> &)  { return false; }
    bool noticeDeviceDescriptionChanged(const QString &) { return false; }


protected:
    void     activateStation(int i);

protected slots:
    void slotLIRC(int socket);
    void slotKbdTimedOut();

protected:

#ifdef HAVE_LIRC_CLIENT
    QSocketNotifier        *m_lirc_notify;
    int                     m_fd_lirc;
    struct lirc_config    *m_lircConfig;
#endif

    QTimer                *m_kbdTimer;
    int                     m_addIndex;
};



#endif
