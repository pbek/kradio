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

#include <QObject>
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

    virtual bool    connectI    (Interface *);
    virtual bool    disconnectI (Interface *);

    virtual void    startPlugin();

    virtual QString pluginClassName() const { return QString::fromLatin1("LircSupport"); }

//     virtual const QString &name() const { return PluginBase::name(); }
//     virtual       QString &name()       { return PluginBase::name(); }


    virtual void                               setActions(const QMap<LIRC_Actions, QString> &actions, const QMap<LIRC_Actions, QString> &alt_actions);
    virtual const QMap<LIRC_Actions, QString> &getActions()            const { return m_Actions; }
    virtual const QMap<LIRC_Actions, QString> &getAlternativeActions() const { return m_AlternativeActions; }

    const QString                             &getLIRCConfigurationFile() const { return m_lirc_config_file; }
    void                                       setLIRCConfigurationFile(const QString &f);
    int                                        getLIRC_fd() const { return m_fd_lirc; }

    const QString                             &getPowerOnMode () const { return m_LIRCPowerOnMode;  }
    const QString                             &getPowerOffMode() const { return m_LIRCPowerOffMode; }
    void                                       getLIRCModeSync(bool &at_startup, bool &at_runtime) { at_startup = m_LIRCModeSyncAtStartup; at_runtime = m_LIRCModeSyncAtRuntime; }
    void                                       setPowerOnMode (const QString &m);
    void                                       setPowerOffMode(const QString &m);
    void                                       setLIRCModeSync(bool at_startup, bool at_runtime);

    // PluginBase

public:
    virtual void   saveState    (      KConfigGroup &) const;
    virtual void   restoreState (const KConfigGroup &);

    virtual ConfigPageInfo  createConfigurationPage();



    // IRadioClient methods

RECEIVERS:
    bool noticePowerChanged(bool on);
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
    bool noticeCountdownSecondsChanged(int /*n*/, bool /*resumeOnSuspend*/)      { return false; }

    // IRadioDevicePoolClient

RECEIVERS:
    bool noticeActiveDeviceChanged(IRadioDevice *)           { return false; }
    bool noticeDevicesChanged(const QList<IRadioDevice*> &)  { return false; }
    bool noticeDeviceDescriptionChanged(const QString &)     { return false; }


protected:
    void     activateStation(int i);
    bool     checkActions(const QString &string, int repeat_counter, const QMap<LIRC_Actions, QString> &map);
    void     processLIRCCode(const QString &c, bool event_map, bool is_raw);

    void     setLIRCMode(const QString &m);
    bool     doLIRCModeSync() const;

    void     checkLIRCConfigurationFile(const QString &fname);

    void     LIRC_init_fd();
    void     LIRC_init_config();
    void     LIRC_close_fd();
    void     LIRC_close_config();

protected slots:
    void slotLIRC(int socket);
    void slotKbdTimedOut();

signals:

    void sigUpdateConfig();

    void sigRawLIRCSignal(const QString &what, int repeat_counter, bool &consumed);

protected:

    QString                 m_lirc_config_file;

    QSocketNotifier        *m_lirc_notify;
    int                     m_fd_lirc;
    struct lirc_config     *m_lircConfig;
    QString                 m_LIRCPowerOnMode;
    QString                 m_LIRCPowerOffMode;
    bool                    m_LIRCModeSyncAtStartup;
    bool                    m_LIRCModeSyncAtRuntime;

    QString                 m_lircrc_startup_mode;

    QTimer                 *m_kbdTimer;
    int                     m_addIndex;
//     bool                    m_TakeRawLIRC;

    QMap<LIRC_Actions, QString>  m_Actions;
    QMap<LIRC_Actions, QString>  m_AlternativeActions;

    bool                         m_inStartupPhase;
    bool                         m_ignorePowerOnOff;

};



#endif
