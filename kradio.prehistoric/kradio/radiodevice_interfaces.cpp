/***************************************************************************
                          radiodevice_interfaces.cpp  -  description
                             -------------------
    begin                : Sam Apr 19 2003
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


#include "radiodevice_interfaces.h"
#include "radiostation.h" 
#include <kurl.h>

// IRadioDevice

IF_IMPL_SENDER  (  IRadioDevice::notifyPowerChanged(bool on),
                   noticePowerChanged(on, this)                                )
IF_IMPL_SENDER  (  IRadioDevice::notifyStationChanged (const RadioStation &s),
                   noticeStationChanged (s, this)                              )

// IRadioDeviceClient

IF_IMPL_SENDER  (  IRadioDeviceClient::sendPower(bool on),
                   setPower(on)                                   )
IF_IMPL_SENDER  (  IRadioDeviceClient::sendPowerOn(),
                   powerOn()                                      )
IF_IMPL_SENDER  (  IRadioDeviceClient::sendPowerOff(),
                   powerOff()                                     )
IF_IMPL_SENDER  (  IRadioDeviceClient::sendActivateStation(const RadioStation &rs),
                   activateStation(rs)                            )

IF_IMPL_QUERY   (  bool IRadioDeviceClient::queryIsPowerOn(),
                   isPowerOn(),
                   false                     )
IF_IMPL_QUERY   (  bool IRadioDeviceClient::queryIsPowerOff(),
                   isPowerOff(),
                   true                      )
IF_IMPL_QUERY   (  const RadioStation  &  IRadioDeviceClient::queryCurrentStation(),
                   getCurrentStation(),
                   undefinedRadioStation          )

// IRadioSound

IF_IMPL_SENDER  (  IRadioSound::notifyVolumeChanged(float v),
                   noticeVolumeChanged(v)                   )
IF_IMPL_SENDER  (  IRadioSound::notifySignalQualityChanged(float q),
                   noticeSignalQualityChanged(q)           )
IF_IMPL_SENDER  (  IRadioSound::notifySignalQualityChanged(bool good),
                   noticeSignalQualityChanged(good)        )
IF_IMPL_SENDER  (  IRadioSound::notifyStereoChanged(bool  s),
                   noticeStereoChanged(s)                   )
IF_IMPL_SENDER  (  IRadioSound::notifyMuted(bool m),
                   noticeMuted(m)                           )


// IRadioSoundClient

IF_IMPL_SENDER  (  IRadioSoundClient::sendVolume (float v),
                   setVolume (v)                            )
IF_IMPL_SENDER  (  IRadioSoundClient::sendMute (bool mute),
                   mute (mute)                              )
IF_IMPL_SENDER  (  IRadioSoundClient::sendUnmute (bool unmute),
                   unmute (unmute)                            )
IF_IMPL_SENDER  (  IRadioSoundClient::sendSignalMinQuality (float q),
                   setSignalMinQuality (q)                  )

IF_IMPL_QUERY   (  float  IRadioSoundClient::queryVolume(),
                   getVolume(),
                   0.0            )
IF_IMPL_QUERY   (  float  IRadioSoundClient::querySignalQuality(),
                   getSignalQuality(),
                   0.0      )
IF_IMPL_QUERY   (  float  IRadioSoundClient::querySignalMinQuality(),
                   getSignalMinQuality(),
                   0.75     )
IF_IMPL_QUERY   (  bool   IRadioSoundClient::queryHasGoodQuality(),
                   hasGoodQuality(),
                   false              )
IF_IMPL_QUERY   (  bool   IRadioSoundClient::queryIsStereo(),
                   isStereo(),
                   false              )
IF_IMPL_QUERY   (  bool   IRadioSoundClient::queryIsMuted(),
                   isMuted(),
                   true               )


// ISeekRadio

IF_IMPL_SENDER  (  ISeekRadio::notifySeekStarted (bool up),
                   noticeSeekStarted (up)                         )
IF_IMPL_SENDER  (  ISeekRadio::notifySeekStopped (),
                   noticeSeekStopped ()                           )
IF_IMPL_SENDER  (  ISeekRadio::notifySeekFinished (const RadioStation &s),
                   noticeSeekFinished (s)                         )

// ISeekRadioClient

IF_IMPL_SENDER  (  ISeekRadioClient::sendStartSeek (bool up),
                   startSeek (up)                                 )
IF_IMPL_SENDER  (  ISeekRadioClient::sendStartSeekUp(),
                   startSeekUp()                                  )
IF_IMPL_SENDER  (  ISeekRadioClient::sendStartSeekDown(),
                   startSeekDown()                                )
IF_IMPL_SENDER  (  ISeekRadioClient::sendStopSeek(),
                   stopSeek()                                     )

IF_IMPL_QUERY   (  bool ISeekRadioClient::queryIsSeekRunning(),
                   isSeekRunning(),
                   false                                          )
IF_IMPL_QUERY   (  bool ISeekRadioClient::queryIsSeekUpRunning(),
                   isSeekUpRunning(),
                   false                                          )
IF_IMPL_QUERY   (  bool ISeekRadioClient::queryIsSeekDownRunning(),
                   isSeekDownRunning(),
                   false                                          )

// IFrequencyRadio

IF_IMPL_SENDER  (  IFrequencyRadio::notifyFrequencyChanged(float f, const RadioStation *s),
                   noticeFrequencyChanged(f, s)                   )
IF_IMPL_SENDER  (  IFrequencyRadio::notifyMinMaxFrequencyChanged(float min, float max),
                   noticeMinMaxFrequencyChanged(min, max)         )
IF_IMPL_SENDER  (  IFrequencyRadio::notifyDeviceMinMaxFrequencyChanged(float min, float max),
                   noticeDeviceMinMaxFrequencyChanged(min, max)         )
IF_IMPL_SENDER  (  IFrequencyRadio::notifyScanStepChanged(float s),
                    noticeScanStepChanged(s)                      )

// IFrequencyRadioClient

IF_IMPL_SENDER  (  IFrequencyRadioClient::sendFrequency(float f),
                   setFrequency(f)                                )
IF_IMPL_SENDER  (  IFrequencyRadioClient::sendMinFrequency(float mf),
                   setMinFrequency(mf)                            )
IF_IMPL_SENDER  (  IFrequencyRadioClient::sendMaxFrequency(float mf),
                   setMaxFrequency(mf)                            )
IF_IMPL_SENDER  (  IFrequencyRadioClient::sendScanStep(float s),
                   setScanStep(s)                                 )

IF_IMPL_QUERY   (  float IFrequencyRadioClient::queryFrequency(),
                   getFrequency(),
                   0                                              )
IF_IMPL_QUERY   (  float IFrequencyRadioClient::queryMinFrequency(),
                   getMinFrequency(),
                   0                                              )
IF_IMPL_QUERY   (  float IFrequencyRadioClient::queryMinDeviceFrequency(),
                   getMinDeviceFrequency(),
                   0                                              )
IF_IMPL_QUERY   (  float IFrequencyRadioClient::queryMaxFrequency(),
                   getMaxFrequency(),
                   0                                              )
IF_IMPL_QUERY   (  float IFrequencyRadioClient::queryMaxDeviceFrequency(),
                   getMaxDeviceFrequency(),
                   0                                              )
IF_IMPL_QUERY   (  float IFrequencyRadioClient::queryScanStep(),
                   getScanStep(),
                   1                                              )

// IInternetRadio

IF_IMPL_SENDER  (  IInternetRadio::notifyURLChanged(const KURL &u, const RadioStation &s),
                   noticeURLChanged(u, s)                         )


// IInternetRadioClient

IF_IMPL_SENDER  (  IInternetRadioClient::sendURL(const KURL &url),
                   setURL(url)                                    )


static KURL emptyURL;

IF_IMPL_QUERY   (  const KURL &IInternetRadioClient::queryURL(),
                   getURL(),
                   emptyURL                                       )



