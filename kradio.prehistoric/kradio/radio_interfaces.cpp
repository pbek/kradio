/***************************************************************************
                          radio_interfaces.cpp  -  description
                             -------------------
    begin                : Don Apr 17 2003
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

#include "radio_interfaces.h"

// IRadio

IF_IMPL_SENDER  (  IRadio::notifyPowerOn(),
                   notifyPowerOn()                                         )
IF_IMPL_SENDER  (  IRadio::notifyPowerOff(),
                   notifyPowerOff()                                        )
IF_IMPL_SENDER  (  IRadio::notifyStationChanged (const RadioStation &s),
                   notifyStationChanged (s)                                )
IF_IMPL_SENDER  (  IRadio::notifyStationsChanged(const StationVector &sl),
                   notifyStationsChanged(sl)                               )

// IRadioClient

IF_IMPL_SENDER  (  IRadioClient::powerOn(),
                   powerOn()                                      )
IF_IMPL_SENDER  (  IRadioClient::powerOff(),
                   powerOff()                                     )
IF_IMPL_SENDER  (  IRadioClient::activateStation(const RadioStation &rs),
                   activateStation(rs)                            )
IF_IMPL_SENDER  (  IRadioClient::activateStation(int index),
                   activateStation(index)                         )
IF_IMPL_SENDER  (  IRadioClient::setStations(const StationVector &sl),
                   setStations(sl)                                )

IF_IMPL_QUERY   (  bool IRadioClient::isPowerOn(),
                   isPowerOn(),
                   false                     )
IF_IMPL_QUERY   (  bool IRadioClient::isPowerOff(),
                   isPowerOff(),
                   true                      )
IF_IMPL_QUERY   (  const RadioStation  &  IRadioClient::getCurrentStation(),
                   getCurrentStation(),
                   RadioStation()            )
IF_IMPL_QUERY   (  const StationVector &  IRadioClient::getStations(),
                   getStations(),
                   StationVector()           )

// IRadioSound

IF_IMPL_SENDER  (  IRadioSound::notifyVolumeChanged(float v),
                   notifyVolumeChanged(v)                   )
IF_IMPL_SENDER  (  IRadioSound::notifySignalQualityChanged(float q),
                    notifySignalQualityChanged(q)           )
IF_IMPL_SENDER  (  IRadioSound::notifyStereoChanged(bool  s),
                   notifyStereoChanged(s)                   )
IF_IMPL_SENDER  (  IRadioSound::notifyMuted(bool m),
                   notifyMuted(m)                           )


// IRadioSoundClient

IF_IMPL_SENDER  (  IRadioSoundClient::setVolume (float v),
                   setVolume (v)                            )
IF_IMPL_SENDER  (  IRadioSoundClient::mute (bool mute),
                   mute (mute)                              )
IF_IMPL_SENDER  (  IRadioSoundClient::unmute (bool mute),
                   unmute (mute)                            )

// queries

IF_IMPL_QUERY   (  float  IRadioSoundClient::getVolume(),
                   getVolume(),
                   0.0            )
IF_IMPL_QUERY   (  float  IRadioSoundClient::getSignalQuality(),
                   getSignalQuality(),
                   0.0      )
IF_IMPL_QUERY   (  bool   IRadioSoundClient::isStereo(),
                   isStereo(),
                   false              )
IF_IMPL_QUERY   (  bool   IRadioSoundClient::isMuted(),
                   isMuted(),
                   true               )


// ISeekRadio

IF_IMPL_SENDER  (  ISeekRadio::notifySeekStarted (bool up),
                   notifySeekStarted (up)                         )
IF_IMPL_SENDER  (  ISeekRadio::notifySeekUpStarted(),
                   notifySeekUpStarted()                          )
IF_IMPL_SENDER  (  ISeekRadio::notifySeekDownStarted(),
                   notifySeekDownStarted()                        )
IF_IMPL_SENDER  (  ISeekRadio::notifySeekStopped (),
                   notifySeekStopped ()                           )
IF_IMPL_SENDER  (  ISeekRadio::notifySeekFinished (const RadioStation &s),
                   notifySeekFinished (s)                         )

// ISeekRadioClient

IF_IMPL_SENDER  (  ISeekRadioClient::startSeek (bool up),
                   startSeek (up)                                 )
IF_IMPL_SENDER  (  ISeekRadioClient::startSeekUp(),
                   startSeekUp()                                  )
IF_IMPL_SENDER  (  ISeekRadioClient::startSeekDown(),
                   startSeekDown()                                )
IF_IMPL_SENDER  (  ISeekRadioClient::stopSeek(),
                    stopSeek()                                    )

IF_IMPL_QUERY   (  bool ISeekRadioClient::isSeekRunning(),
                   isSeekRunning(),
                   false                                          )
IF_IMPL_QUERY   (  bool ISeekRadioClient::isSeekUpRunning(),
                   isSeekUpRunning(),
                   false                                          )
IF_IMPL_QUERY   (  bool ISeekRadioClient::isSeekDownRunning(),
                   isSeekDownRunning(),
                   false                                          )

// IWaveRadio

IF_IMPL_SENDER  (  IWaveRadio::notifyFrequencyChanged(float f, const RadioStation *s),
                   notifyFrequencyChanged(f, s)                   )
IF_IMPL_SENDER  (  IWaveRadio::notifyMinMaxFrequencyChanged(float min, float max),
                   notifyMinMaxFrequencyChanged(min, max)         )
IF_IMPL_SENDER  (  IWaveRadio::notifyScanStepChanged(float s),
                    notifyScanStepChanged(s)                      )

// IWaveRadioClient

IF_IMPL_SENDER  (  IWaveRadioClient::setFrequency(float f),
                   setFrequency(f)                                )
IF_IMPL_SENDER  (  IWaveRadioClient::setScanStep(float s),
                   setScanStep(s)                                 )

IF_IMPL_QUERY   (  float IWaveRadioClient::getFrequency(),
                   getFrequency(),
                   0                                              )
IF_IMPL_QUERY   (  float IWaveRadioClient::getMinFrequency(),
                   getMinFrequency(),
                   0                                              )
IF_IMPL_QUERY   (  float IWaveRadioClient::getMaxFrequency(),
                   getMaxFrequency(),
                   0                                              )
IF_IMPL_QUERY   (  float IWaveRadioClient::getScanStep(),
                   getScanStep(),
                   1                                              )


