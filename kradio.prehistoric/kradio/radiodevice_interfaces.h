/***************************************************************************
                          radiodevice_interfaces.h  -  description
                             -------------------
    begin                : Fre Apr 18 2003
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

/***************************************************************************
 *                                                                         *
 *   Interfaces in this header:                                            *
 *                                                                         *
 *   IRadioDevice(Client)                                                  *
 *   IRadioSound(Client)                                                   *
 *   ISeekRadio(Client)                                                    *
 *   IFrequencyRadio(Client)                                               *
 *   IInternetRadio(Client)                                                *
 *                                                                         *
 ***************************************************************************/

#ifndef KRADIO_RADIODEVICE_INTERFACES_H
#define KRADIO_RADIODEVICE_INTERFACES_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "interfaces.h"

class RadioStation;
class KURL;

INTERFACE(IRadioDevice, IRadioDeviceClient)
{
public:
	IRadioDevice() : IRadioDevice::BaseClass(-1) {}

RECEIVERS:
	IF_RECEIVER(  setPower(bool on)                                )
	IF_RECEIVER(  powerOn()                                        )
	IF_RECEIVER(  powerOff()                                       )
    IF_RECEIVER(  activateStation(const RadioStation &rs)          )

SENDERS:
	IF_SENDER  (  notifyPowerChanged(bool on)                      )
	IF_SENDER  (  notifyStationChanged (const RadioStation &)      )

ANSWERS:
	IF_ANSWER  (  bool                   isPowerOn() const         )
	IF_ANSWER  (  bool                   isPowerOff() const        )
	IF_ANSWER  (  const RadioStation  &  getCurrentStation() const )
};



INTERFACE(IRadioDeviceClient, IRadioDevice)
{
public :
	IRadioDeviceClient(int _maxConnections = 1) : IRadioDeviceClient::BaseClass(_maxConnections) {}

SENDERS:
	IF_SENDER  (  sendPower(bool on)                                 )
	IF_SENDER  (  sendPowerOn()                                      )
	IF_SENDER  (  sendPowerOff()                                     )
    IF_SENDER  (  sendActivateStation (const RadioStation &rs)       )

RECEIVERS:
	IF_RECEIVER(  noticePowerChanged   (bool on, const IRadioDevice *sender = NULL)                 )
	IF_RECEIVER(  noticeStationChanged (const RadioStation &, const IRadioDevice *sender = NULL)    )

QUERIES:
	IF_QUERY   (  bool                   queryIsPowerOn()             )
	IF_QUERY   (  bool                   queryIsPowerOff()            )
	IF_QUERY   (  const RadioStation  &  queryCurrentStation()     )
};


/////////////////////////////////////////////////////////////////////////////

INTERFACE(IRadioSound, IRadioSoundClient)
{
public :
	IRadioSound() : IRadioSound::BaseClass(-1) {}

RECEIVERS:
	IF_RECEIVER(  setVolume (float v)                            )
	IF_RECEIVER(  mute (bool mute)                               )
	IF_RECEIVER(  unmute (bool unmute)                           )
	IF_RECEIVER(  setSignalMinQuality(float q)                   )

SENDERS:
	IF_SENDER  (  notifyVolumeChanged(float v)                   )
	IF_SENDER  (  notifySignalQualityChanged(float q)            )
	IF_SENDER  (  notifySignalQualityChanged(bool good)          )
	IF_SENDER  (  notifyStereoChanged(bool  s)                   )
	IF_SENDER  (  notifyMuted(bool m)                            )

ANSWERS:
	IF_ANSWER  (  float   getVolume() const                      )
	IF_ANSWER  (  float   getSignalQuality() const               )
	IF_ANSWER  (  float   getSignalMinQuality() const            )
	IF_ANSWER  (  bool    hasGoodQuality() const                 )
	IF_ANSWER  (  bool    isStereo() const                       )
	IF_ANSWER  (  bool    isMuted() const                        )
};


INTERFACE(IRadioSoundClient, IRadioSound)
{
public :
	IRadioSoundClient() : IRadioSoundClient::BaseClass(1) {}

SENDERS:
	IF_SENDER  (  sendVolume (float v)                           )
	IF_SENDER  (  sendMute (bool mute = true)                    )
	IF_SENDER  (  sendUnmute (bool unmute = true)                )
	IF_SENDER  (  sendSignalMinQuality (float q)                 )

RECEIVERS:
	IF_RECEIVER(  noticeVolumeChanged(float v)                   )
	IF_RECEIVER(  noticeSignalQualityChanged(float q)            )
	IF_RECEIVER(  noticeSignalQualityChanged(bool good)          )
	IF_RECEIVER(  noticeStereoChanged(bool  s)                   )
	IF_RECEIVER(  noticeMuted(bool m)                            )

QUERIES:
	IF_QUERY   (  float   queryVolume()                          )
	IF_QUERY   (  float   querySignalQuality()                   )
	IF_QUERY   (  float   querySignalMinQuality()                )
	IF_QUERY   (  bool    queryHasGoodQuality()                   )
	IF_QUERY   (  bool    queryIsStereo()                        )
	IF_QUERY   (  bool    queryIsMuted()                         )
};


/////////////////////////////////////////////////////////////////////////////
INTERFACE(ISeekRadio, ISeekRadioClient)
{

    friend class SeekHelper;

public :
	ISeekRadio() : ISeekRadio::BaseClass(-1) {}

RECEIVERS:
	IF_RECEIVER(  startSeek (bool up)                            )
	IF_RECEIVER(  startSeekUp()                                  )
	IF_RECEIVER(  startSeekDown()                                )
	IF_RECEIVER(  stopSeek()                                     )

SENDERS:
	IF_SENDER  (  notifySeekStarted (bool up)                    )
	IF_SENDER  (  notifySeekStopped ()                           )
	IF_SENDER  (  notifySeekFinished (const RadioStation &s)     )

ANSWERS:
    IF_ANSWER  (  bool isSeekRunning() const                     )
    IF_ANSWER  (  bool isSeekUpRunning() const                   )
    IF_ANSWER  (  bool isSeekDownRunning() const                 )
};


INTERFACE(ISeekRadioClient, ISeekRadio)
{
public :
	ISeekRadioClient() : ISeekRadioClient::BaseClass(1) {}

SENDERS:
	IF_SENDER  (  sendStartSeek (bool up)                             )
	IF_SENDER  (  sendStartSeekUp()                                   )
	IF_SENDER  (  sendStartSeekDown()                                 )
	IF_SENDER  (  sendStopSeek()                                      )

RECEIVERS:
	IF_RECEIVER(  noticeSeekStarted (bool up)                         )
	IF_RECEIVER(  noticeSeekStopped ()                                )
	IF_RECEIVER(  noticeSeekFinished (const RadioStation &s)          )

QUERIES:
    IF_QUERY  (  bool queryIsSeekRunning()                            )
    IF_QUERY  (  bool queryIsSeekUpRunning()                          )
    IF_QUERY  (  bool queryIsSeekDownRunning()                        )
};



/////////////////////////////////////////////////////////////////////////////

INTERFACE(IFrequencyRadio, IFrequencyRadioClient)
{
public :
	IFrequencyRadio() : IFrequencyRadio::BaseClass(-1) {}

RECEIVERS:
	IF_RECEIVER(  setFrequency(float f)                                          )
	IF_RECEIVER(  setMinFrequency(float mf)                                      )
	IF_RECEIVER(  setMaxFrequency(float mf)                                      )
	IF_RECEIVER(  setScanStep(float s)                                           )

SENDERS:
	IF_SENDER  (  notifyFrequencyChanged(float f, const RadioStation *s)         )
	IF_SENDER  (  notifyMinMaxFrequencyChanged(float min, float max)             )
	IF_SENDER  (  notifyDeviceMinMaxFrequencyChanged(float min, float max)       )
	IF_SENDER  (  notifyScanStepChanged(float s)                                 )

ANSWERS:
	IF_ANSWER  (  float        getFrequency()     const                  )
	IF_ANSWER  (  float        getMinFrequency()  const                  )
    IF_ANSWER  (  float        getMinDeviceFrequency() const             )
	IF_ANSWER  (  float        getMaxFrequency()  const                  )
	IF_ANSWER  (  float        getMaxDeviceFrequency()  const            )
	IF_ANSWER  (  float        getScanStep()      const                  )
};


INTERFACE(IFrequencyRadioClient, IFrequencyRadio)
{
public :
	IFrequencyRadioClient() : IFrequencyRadioClient::BaseClass(1) {}

SENDERS:
	IF_SENDER  (  sendFrequency(float f)                                         )
	IF_SENDER  (  sendMinFrequency(float mf)                                     )
	IF_SENDER  (  sendMaxFrequency(float mf)                                     )
	IF_SENDER  (  sendScanStep(float s)                                          )

RECEIVERS:
	IF_RECEIVER(  noticeFrequencyChanged(float f, const RadioStation *s)         )
	IF_RECEIVER(  noticeMinMaxFrequencyChanged(float min, float max)             )
	IF_RECEIVER(  noticeDeviceMinMaxFrequencyChanged(float min, float max)       )
	IF_RECEIVER(  noticeScanStepChanged(float s)                                 )

QUERIES:
	IF_QUERY   (  float queryFrequency()                                         )
	IF_QUERY   (  float queryMinFrequency()                                      )
	IF_QUERY   (  float queryMinDeviceFrequency()                                )
	IF_QUERY   (  float queryMaxFrequency()                                      )
	IF_QUERY   (  float queryMaxDeviceFrequency()                                )
	IF_QUERY   (  float queryScanStep()                                          )
};


/////////////////////////////////////////////////////////////////////////////

INTERFACE(IInternetRadio, IInternetRadioClient)
{
public :
	IInternetRadio() : IInternetRadio::BaseClass(-1) {}

RECEIVERS:
	IF_RECEIVER(  setURL(const KURL &url)                                        )

SENDERS:
	IF_SENDER  (  notifyURLChanged(const KURL &u, const RadioStation &s)         )

ANSWERS:
	IF_ANSWER  (  const KURL &          getURL() const                           )
 	IF_ANSWER  (  const RadioStation &  getStation (const KURL &url) const       )
};




INTERFACE(IInternetRadioClient, IInternetRadio)
{
public :
	IInternetRadioClient() : IInternetRadioClient::BaseClass(1) {}


SENDERS:
	IF_SENDER  (  sendURL(const KURL &url)                                       )

RECEIVERS:
	IF_RECEIVER(  noticeURLChanged(const KURL &url, const RadioStation &s)       )

QUERIES:
	IF_QUERY   (  const KURL &queryURL()                                         )
};



#endif

