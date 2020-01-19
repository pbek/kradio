/***************************************************************************
                          radio.h  -  description
                             -------------------
    begin                : Sat March 29 2003
    copyright            : (C) 2003 by Klas Kalass, Ernst Martin Witte
    email                : klas@kde.org, emw-kradio@nocabal.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRADIO_RADIO_H
#define KRADIO_RADIO_H

#include "radio_interfaces.h"
#include "radiodevicepool_interfaces.h"
#include "radiodevice_interfaces.h"
#include "timecontrol_interfaces.h"
#include "soundstreamclient_interfaces.h"
#include "stationlist.h"
#include "pluginbase.h"

/**
 * The main Radio class, which is used as the interface of the radio functionality
 * to the GUI parts of the application
 * @author Klas Kalass, Ernst Martin Witte
 */

/////////////////////////////////////////////////////////////////////////////

/*  A class, that is able to manage more than one radio device, one of those
    is active at a time. This class behaves represents the active device,
    the active devices can be changed either by selecting a station or by
    explicitly changing the devices.

    At any time a valid active device exists as long as any device is connected.

*/

class Radio : public PluginBase,
              public IRadio,
              public IRadioDevicePool,
              public IRadioDeviceClient,
              public ITimeControlClient,
              public ISoundStreamClient
{
public:
    Radio(const QString &instanceID, const QString &name);
    ~Radio();


    // PluginBase

public:
    virtual void   saveState    (      KConfigGroup &) const override;
    virtual void   restoreState (const KConfigGroup &)       override;
    virtual void   startPlugin() override;

    virtual QString pluginClassName() const override { return QString::fromLatin1("Radio"); }

    virtual ConfigPageInfo  createConfigurationPage() override;

    virtual void aboutToQuit() override;

    // IRadio methods

RECEIVERS:
    bool powerOn()   override      { return sendPowerOn()  > 0; }
    bool powerOff()  override      { return sendPowerOff() > 0; }
    bool activateStation(const RadioStation &rs) override;
    bool activateStation(int index) override;
    bool setStations(const StationList &sl) override;
    bool setPresetFile(const QString &presetFile) override;

ANSWERS:
    bool                   isPowerOn () const override { return queryIsPowerOn(); }
    bool                   isPowerOff() const override { return queryIsPowerOff(); }
    const RadioStation  &  getCurrentStation() const override { return queryCurrentStation(); }
    int                    getStationIdx(const RadioStation &) const override;
    int                    getCurrentStationIdx() const override;
    const StationList   &  getStations  () const override { return m_stationList; }
    const QString       &  getPresetFile() const override { return m_presetFile; }

    bool                   getRDSState      () const override;
    const QString       &  getRDSRadioText  () const override;
    const QString       &  getRDSStationName() const override;

    SoundStreamID          getCurrentSoundStreamSourceID() const override;
    SoundStreamID          getCurrentSoundStreamSinkID()   const override;


public:
    bool connectI    (Interface *i) override;
    bool disconnectI (Interface *i) override;

    virtual void noticeConnectedI   (IRadioDevice *i, bool pointer_valid) override;
    virtual void noticeDisconnectI  (IRadioDevice *i, bool pointer_valid) override;
    virtual void noticeDisconnectedI(IRadioDevice *i, bool pointer_valid) override;

    INLINE_IMPL_DEF_noticeConnectedI(IRadio);
    INLINE_IMPL_DEF_noticeConnectedI(IRadioDevicePool);
    INLINE_IMPL_DEF_noticeConnectedI(ITimeControlClient);
    INLINE_IMPL_DEF_noticeConnectedI(ISoundStreamClient);
    INLINE_IMPL_DEF_noticeConnectedI(IErrorLogClient);

    INLINE_IMPL_DEF_noticeDisconnectI(IRadio);
    INLINE_IMPL_DEF_noticeDisconnectI(IRadioDevicePool);
    INLINE_IMPL_DEF_noticeDisconnectI(ITimeControlClient);
    INLINE_IMPL_DEF_noticeDisconnectI(ISoundStreamClient);
    INLINE_IMPL_DEF_noticeDisconnectI(IErrorLogClient);

    INLINE_IMPL_DEF_noticeDisconnectedI(IRadio);
    INLINE_IMPL_DEF_noticeDisconnectedI(IRadioDevicePool);
    INLINE_IMPL_DEF_noticeDisconnectedI(ITimeControlClient);
    INLINE_IMPL_DEF_noticeDisconnectedI(ISoundStreamClient);
    INLINE_IMPL_DEF_noticeDisconnectedI(IErrorLogClient);

    // IRadioDevicePool methods

RECEIVERS:
    bool                           setActiveDevice(IRadioDevice *rd, bool keepPower = true) override;

ANSWERS:
    IRadioDevice                 * getActiveDevice     () const override;
    const QList<IRadioDevice*>   & getDevices          () const override;
    const QString                & getDeviceDescription() const override;



    // IRadioDeviceClient methods, even sending methods overwritten
    // to provide "1-of-N" functionality

SENDERS:
    IF_SENDER_OVR  (  sendPowerOn()                                      )
    IF_SENDER_OVR  (  sendPowerOff()                                     )
    IF_SENDER_OVR  (  sendActivateStation (const RadioStation &rs)       )

QUERIES:
    IF_QUERY_OVR   (  bool                   queryIsPowerOn()            )
    IF_QUERY_OVR   (  bool                   queryIsPowerOff()           )
    IF_QUERY_OVR   (  const RadioStation  &  queryCurrentStation()       )
    IF_QUERY_OVR   (  const QString       &  queryDescription()          )

    IF_QUERY_OVR   (  bool                   queryRDSState ()            )
    IF_QUERY_OVR   (  const QString       &  queryRDSRadioText()         )
    IF_QUERY_OVR   (  const QString       &  queryRDSStationName()       )

    IF_QUERY_OVR   (  SoundStreamID          queryCurrentSoundStreamSourceID() )
    IF_QUERY_OVR   (  SoundStreamID          queryCurrentSoundStreamSinkID()   )

RECEIVERS:
    virtual bool noticePowerChanged         (bool  on,               const IRadioDevice *sender = NULL) override;
    virtual bool noticeStationChanged       (const RadioStation &rs, const IRadioDevice *sender = NULL) override;
    virtual bool noticeDescriptionChanged   (const QString &,        const IRadioDevice *sender = NULL) override;

    virtual bool noticeRDSStateChanged      (bool  enabled,          const IRadioDevice *sender = NULL) override;
    virtual bool noticeRDSRadioTextChanged  (const QString &s,       const IRadioDevice *sender = NULL) override;
    virtual bool noticeRDSStationNameChanged(const QString &s,       const IRadioDevice *sender = NULL) override;

    virtual bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID id, const IRadioDevice *sender = NULL) override;
    virtual bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID id, const IRadioDevice *sender = NULL) override;

    // ITimeControlClient

RECEIVERS:
    bool noticeAlarmsChanged(const AlarmVector &)         override { return false; } // ignore
    bool noticeAlarm(const Alarm &)                       override;
    bool noticeNextAlarmChanged(const Alarm *)            override { return false; } // ignore
    bool noticeCountdownStarted(const QDateTime &/*end*/) override { return false; } // ignore
    bool noticeCountdownStopped()                         override { return false; } // ignore
    bool noticeCountdownZero()                            override;
    bool noticeCountdownSecondsChanged(int /*n*/, bool /*resumeOnSuspend*/) override { return false; } // ignore

    // ISoundStreamClient

RECEIVERS:

    // ...

protected:

    QString        m_presetFile;
    StationList    m_stationList;
    IRadioDevice  *m_activeDevice;
    QString        m_startup_LastActiveDeviceID;
};


#endif
