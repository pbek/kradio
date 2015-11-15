/***************************************************************************
                          dbussupport.h  -  description
                             -------------------
    begin                : Tue Mar 3 2009
    copyright            : (C) 2009 by Martin Witte
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

#ifndef DBUSSUPPORT_H
#define DBUSSUPPORT_H

#include <QtCore/QObject>
#include "timecontrol_interfaces.h"
#include "radio_interfaces.h"
#include "radiodevicepool_interfaces.h"
#include "soundstreamclient_interfaces.h"
#include "pluginbase.h"


class DBusSupport : public QObject,
                    public PluginBase,
                    public IRadioClient,
                    public ITimeControlClient,
                    public ISoundStreamClient,
                    public IRadioDevicePoolClient
{
Q_OBJECT
Q_CLASSINFO("D-Bus Interface", "net.sourceforge.kradio")

public:
    DBusSupport(const QString &instanceID, const QString &name);
    ~DBusSupport();

    virtual bool connectI (Interface *);
    virtual bool disconnectI (Interface *);

    virtual QString pluginClassName() const { return "DBusSupport"; }

//     virtual const QString &name() const { return PluginBase::name(); }
//     virtual       QString &name()       { return PluginBase::name(); }

    virtual void           startPlugin();

    // PluginBase

public:
    virtual void   saveState    (      KConfigGroup &) const;
    virtual void   restoreState (const KConfigGroup &);

    virtual ConfigPageInfo  createConfigurationPage();


    // IRadioClient methods

RECEIVERS:
    bool noticePowerChanged(bool on)                                    { emit powerChanged(on);           return false; }
    bool noticeStationChanged (const RadioStation &, int idx)           { emit currentStationChanged(idx); return false; }
    bool noticeStationsChanged(const StationList &/*sl*/)               { return false; }
    bool noticePresetFileChanged(const QString &/*f*/)                  { return false; }

    bool noticeRDSStateChanged      (bool  enabled)                     { emit RDSStateChanged(enabled); return false; }
    bool noticeRDSRadioTextChanged  (const QString &s)                  { emit RDSRadioTextChanged(s);   return false; }
    bool noticeRDSStationNameChanged(const QString &s)                  { emit RDSStationNameChanged(s); return false; }

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/)  { return false; }
    bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID /*id*/)  { return false; }

    // ITimeControlClient

RECEIVERS:
    bool noticeAlarmsChanged(const AlarmVector &)                          { return false; }
    bool noticeAlarm(const Alarm &)                                        { return false; }
    bool noticeNextAlarmChanged(const Alarm *)                             { return false; }
    bool noticeCountdownStarted(const QDateTime &end)                      { emit sleepCountdownStarted(end.toTime_t());  return false; }
    bool noticeCountdownStopped()                                          { emit sleepCountdownStopped();     return false; }
    bool noticeCountdownZero()                                             { emit sleepCountdownZeroReached(); return false; }
    bool noticeCountdownSecondsChanged(int /*n*/, bool /*suspendOnSleep*/) { return false; }

    // IRadioDevicePoolClient

RECEIVERS:
    bool noticeActiveDeviceChanged(IRadioDevice *)                      { return false; }
    bool noticeDevicesChanged(const QList<IRadioDevice*> &)             { return false; }
    bool noticeDeviceDescriptionChanged(const QString &)                { return false; }



    // DBus support signals/slots

public Q_SLOTS:

    Q_SCRIPTABLE void      powerOn();
    Q_SCRIPTABLE void      powerOff();
    Q_SCRIPTABLE void      recordingStart();
    Q_SCRIPTABLE void      recordingStop();
    Q_SCRIPTABLE void      playbackPause();
    Q_SCRIPTABLE void      playbackResume();

    Q_SCRIPTABLE void      setVolume(float v);
    Q_SCRIPTABLE void      increaseVolume();
    Q_SCRIPTABLE void      decreaseVolume();

    Q_SCRIPTABLE void      nextStation();
    Q_SCRIPTABLE void      prevStation();
    Q_SCRIPTABLE void      setStation(int idx);
    Q_SCRIPTABLE void      setStation(const QString &stationid);
    Q_SCRIPTABLE void      searchNextStation();
    Q_SCRIPTABLE void      searchPrevStation();

    Q_SCRIPTABLE void      startSleepCountdown(int seconds, bool suspendOnSleep);
    Q_SCRIPTABLE void      stopSleepCountdown();

    Q_SCRIPTABLE void      showAllWidgets();
    Q_SCRIPTABLE void      hideAllWidgets();
    Q_SCRIPTABLE void      restoreAllWidgets();
    Q_SCRIPTABLE void      quitKRadio();

public Q_SLOTS:
    Q_SCRIPTABLE bool      isPowerOn()   const;
    Q_SCRIPTABLE bool      isPaused()    const;
    Q_SCRIPTABLE bool      isRecording() const;
    Q_SCRIPTABLE bool      isSleepCountdownRunning() const;
    Q_SCRIPTABLE time_t    getSleepCountdownEnd() const;

    Q_SCRIPTABLE float     getVolume() const;

    Q_SCRIPTABLE int       getStationsCount() const;
    Q_SCRIPTABLE int       getCurrentStationIndex() const;
    Q_SCRIPTABLE QString   getStationName(int idx) const;
    Q_SCRIPTABLE QString   getStationShortName(int idx) const;
    Q_SCRIPTABLE QString   getStationLongName(int idx) const;
    Q_SCRIPTABLE QString   getStationDescription(int idx) const;

Q_SIGNALS:
    Q_SCRIPTABLE void      powerChanged(bool on);
    Q_SCRIPTABLE void      currentStationChanged(int idx);

    Q_SCRIPTABLE void      RDSStateChanged      (bool enabled);
    Q_SCRIPTABLE void      RDSRadioTextChanged  (const QString &s);
    Q_SCRIPTABLE void      RDSStationNameChanged(const QString &s);

    Q_SCRIPTABLE void      sleepCountdownStarted(time_t time_until);
    Q_SCRIPTABLE void      sleepCountdownStopped();
    Q_SCRIPTABLE void      sleepCountdownZeroReached();


};



#endif
