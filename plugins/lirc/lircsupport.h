/***************************************************************************
                          lircsupport.h  -  description
                             -------------------
    begin                : Mon Feb 4 2002
    copyright            : (C) 2002 by Martin Witte / Frank Schwanz
    email                : emw-kradio@nocabal.de / schwanz@fh-brandenburg.de
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

#include <QtCore/QObject>
#include "timecontrol_interfaces.h"
#include "radio_interfaces.h"
#include "radiodevicepool_interfaces.h"
#include "soundstreamclient_interfaces.h"
#include "pluginbase.h"


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
    LircSupport(const QString &instanceID, const QString &name);
    ~LircSupport();

    virtual bool connectI (Interface *);
    virtual bool disconnectI (Interface *);

    virtual QString pluginClassName() const { return "LircSupport"; }

//     virtual const QString &name() const { return PluginBase::name(); }
//     virtual       QString &name()       { return PluginBase::name(); }


    virtual void                               setActions(const QMap<LIRC_Actions, QString> &actions, const QMap<LIRC_Actions, QString> &alt_actions);
    virtual const QMap<LIRC_Actions, QString> &getActions()            const { return m_Actions; }
    virtual const QMap<LIRC_Actions, QString> &getAlternativeActions() const { return m_AlternativeActions; }

    const struct lirc_config                  *getLIRCConfig()          const { return m_lircConfig;              }
    const QString                             &getStartupPowerOnMode () const { return m_LIRCStartupPowerOnMode;  }
    const QString                             &getStartupPowerOffMode() const { return m_LIRCStartupPowerOffMode; }
    void                                       setStartupPowerOnMode (const QString &m);
    void                                       setStartupPowerOffMode(const QString &m);

    // PluginBase

public:
    virtual void   saveState    (      KConfigGroup &) const;
    virtual void   restoreState (const KConfigGroup &);

    virtual ConfigPageInfo  createConfigurationPage();
//     virtual AboutPageInfo   createAboutPage();



    // IRadioClient methods

RECEIVERS:
    bool noticePowerChanged(bool /*on*/)                          { return false; }
    bool noticeStationChanged (const RadioStation &, int /*idx*/) { return false; }
    bool noticeStationsChanged(const StationList &/*sl*/)         { return false; }
    bool noticePresetFileChanged(const QString &/*f*/)            { return false; }

    bool noticeRDSStateChanged      (bool  /*enabled*/)           { return false; }
    bool noticeRDSRadioTextChanged  (const QString &/*s*/)        { return false; }
    bool noticeRDSStationNameChanged(const QString &/*s*/)        { return false; }

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/)  { return false; }
    bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID /*id*/)  { return false; }

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
    bool noticeActiveDeviceChanged(IRadioDevice *)           { return false; }
    bool noticeDevicesChanged(const QList<IRadioDevice*> &)  { return false; }
    bool noticeDeviceDescriptionChanged(const QString &)     { return false; }


protected:
    void     activateStation(int i);
    bool     checkActions(const QString &string, int repeat_counter, const QMap<LIRC_Actions, QString> &map);
    void     processLIRCCode(const QString &c, bool event_map, bool is_raw);

protected slots:
    void slotLIRC(int socket);
    void slotKbdTimedOut();

signals:

    void sigUpdateConfig();

    void sigRawLIRCSignal(const QString &what, int repeat_counter, bool &consumed);

protected:

    QSocketNotifier        *m_lirc_notify;
    int                     m_fd_lirc;
    struct lirc_config     *m_lircConfig;
    QString                 m_LIRCStartupPowerOnMode;
    QString                 m_LIRCStartupPowerOffMode;

    QString                 m_lircrc_startup_mode;

    QTimer                 *m_kbdTimer;
    int                     m_addIndex;
//     bool                    m_TakeRawLIRC;

    QMap<LIRC_Actions, QString>  m_Actions;
    QMap<LIRC_Actions, QString>  m_AlternativeActions;

};



#endif