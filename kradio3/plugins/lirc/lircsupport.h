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
#include "../../src/interfaces/timecontrol_interfaces.h"
#include "../../src/interfaces/radio_interfaces.h"
#include "../../src/interfaces/radiodevicepool_interfaces.h"
#include "../../src/interfaces/soundstreamclient_interfaces.h"
#include "../../src/libkradio/plugins.h"


enum LIRC_Actions {
    LIRC_DIGIT_0,
    LIRC_DIGIT_1,
    LIRC_DIGIT_2,
    LIRC_DIGIT_3,
    LIRC_DIGIT_4,
    LIRC_DIGIT_5,
    LIRC_DIGIT_6,
    LIRC_DIGIT_7,
    LIRC_DIGIT_8,
    LIRC_DIGIT_9,
    LIRC_POWER_ON,
    LIRC_POWER_OFF,
    LIRC_PAUSE,
    LIRC_RECORD_START,
    LIRC_RECORD_STOP,
    LIRC_VOLUME_INC,
    LIRC_VOLUME_DEC,
    LIRC_CHANNEL_NEXT,
    LIRC_CHANNEL_PREV,
    LIRC_SEARCH_NEXT,
    LIRC_SEARCH_PREV,
    LIRC_SLEEP,
    LIRC_APPLICATION_QUIT
};


struct lirc_config;
class QSocketNotifier;
class QTimer;

class LircSupport : public QObject,
                    public PluginBase,
                    public IRadioClient,
                    public ITimeControlClient,
                    public ISoundStreamClient,
                    public IRadioDevicePoolClient
{
Q_OBJECT
public:
    LircSupport(const QString &name);
    ~LircSupport();

    virtual bool connectI (Interface *);
    virtual bool disconnectI (Interface *);

    virtual QString pluginClassName() const { return "LircSupport"; }

    virtual const QString &name() const { return PluginBase::name(); }
    virtual       QString &name()       { return PluginBase::name(); }


    virtual void                               setActions(const QMap<LIRC_Actions, QString> &actions, const QMap<LIRC_Actions, QString> &alt_actions);
    virtual const QMap<LIRC_Actions, QString> &getActions()            const { return m_Actions; }
    virtual const QMap<LIRC_Actions, QString> &getAlternativeActions() const { return m_AlternativeActions; }

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

    bool noticeCurrentSoundStreamIDChanged(SoundStreamID /*id*/)  { return false; }

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
    bool     checkActions(const QString &string, int repeat_counter, const QMap<LIRC_Actions, QString> &map);

protected slots:
    void slotLIRC(int socket);
    void slotKbdTimedOut();

signals:

    void sigUpdateConfig();

    void sigRawLIRCSignal(const QString &what, int repeat_counter, bool &consumed);

protected:

#ifdef HAVE_LIRC
    QSocketNotifier        *m_lirc_notify;
    int                     m_fd_lirc;
    struct lirc_config     *m_lircConfig;
#endif

    QTimer                 *m_kbdTimer;
    int                     m_addIndex;
    bool                    m_TakeRawLIRC;

    QMap<LIRC_Actions, QString>  m_Actions;
    QMap<LIRC_Actions, QString>  m_AlternativeActions;
};



#endif
