/***************************************************************************
                          radio.h  -  description
                             -------------------
    begin                : Sat March 29 2003
    copyright            : (C) 2003 by Klas Kalass, Ernst Martin Witte
    email                : klas@kde.org, witte@kawo1.rwth-aachen.de
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include "../../src/interfaces/radio_interfaces.h"
#include "../../src/interfaces/radiodevicepool_interfaces.h"
#include "../../src/interfaces/radiodevice_interfaces.h"
#include "../../src/interfaces/timecontrol_interfaces.h"
#include "../../src/interfaces/soundstreamclient_interfaces.h"
#include "../../src/libkradio/stationlist.h"
#include "../../src/libkradio/plugins.h"

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
    Radio(const QString &name);
    ~Radio();


    // PluginBase

public:
    virtual void   saveState (KConfig *) const;
    virtual void   restoreState (KConfig *);

    virtual QString pluginClassName() const { return "Radio"; }

    virtual ConfigPageInfo  createConfigurationPage();
    virtual AboutPageInfo   createAboutPage();


    // IRadio methods

RECEIVERS:
    bool powerOn()        { return sendPowerOn()  > 0; }
    bool powerOff()       { return sendPowerOff() > 0; }
    bool activateStation(const RadioStation &rs);
    bool activateStation(int index);
    bool setStations(const StationList &sl);
    bool setPresetFile(const QString &presetFile);

ANSWERS:
    bool                   isPowerOn() const { return queryIsPowerOn(); }
    bool                   isPowerOff() const { return queryIsPowerOff(); }
    const RadioStation  &  getCurrentStation() const { return queryCurrentStation(); }
    int                    getStationIdx(const RadioStation &) const;
    int                    getCurrentStationIdx() const;
    const StationList   &  getStations() const { return m_stationList; }
    const QString       &  getPresetFile() const { return m_presetFile; }

    SoundStreamID          getCurrentSoundStreamID() const;


public:
    bool connectI    (Interface *i);
    bool disconnectI (Interface *i);

    void noticeConnectedI (IRadioDeviceClient::cmplInterface *i, bool pointer_valid);
    void noticeDisconnectI(IRadioDeviceClient::cmplInterface *i, bool pointer_valid);

    // IRadioDevicePool methods

RECEIVERS:
    bool                           setActiveDevice(IRadioDevice *rd, bool keepPower = true);

ANSWERS:
    IRadioDevice                 * getActiveDevice() const;
    const QPtrList<IRadioDevice> & getDevices() const;
    const QString                & getDeviceDescription() const;



    // IRadioDeviceClient methods, even sending methods overwritten
    // to provide "1-of-N" functionality

SENDERS:
    IF_SENDER  (  sendPowerOn()                                      )
    IF_SENDER  (  sendPowerOff()                                     )
    IF_SENDER  (  sendActivateStation (const RadioStation &rs)       )

QUERIES:
    IF_QUERY   (  bool                   queryIsPowerOn()            )
    IF_QUERY   (  bool                   queryIsPowerOff()           )
    IF_QUERY   (  const RadioStation  &  queryCurrentStation()       )
    IF_QUERY   (  const QString       &  queryDescription()          )
    IF_QUERY   (  SoundStreamID          queryCurrentSoundStreamID() )

RECEIVERS:
    virtual bool noticePowerChanged   (bool on, const IRadioDevice *sender = NULL);
    virtual bool noticeStationChanged (const RadioStation &rs, const IRadioDevice *sender = NULL);
    virtual bool noticeDescriptionChanged (const QString &, const IRadioDevice *sender = NULL);

    virtual bool noticeCurrentSoundStreamIDChanged(SoundStreamID id, const IRadioDevice *sender = NULL);

    // ITimeControlClient

RECEIVERS:
    bool noticeAlarmsChanged(const AlarmVector &)        { return false; } // ignore
    bool noticeAlarm(const Alarm &);
    bool noticeNextAlarmChanged(const Alarm *)           { return false; } // ignore
    bool noticeCountdownStarted(const QDateTime &/*end*/){ return false; } // ignore
    bool noticeCountdownStopped()                        { return false; } // ignore
    bool noticeCountdownZero();
    bool noticeCountdownSecondsChanged(int /*n*/)        { return false; } // ignore

    // ISoundStreamClient

RECEIVERS:

    // ...

protected:

    QString        m_presetFile;
    StationList    m_stationList;
    IRadioDevice  *m_activeDevice;
};


#endif