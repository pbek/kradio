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
#include "../radio-stations/radiostation.h"

#include <kurl.h>

// IRadioDevice

IF_IMPL_SENDER  (  IRadioDevice::notifyPowerChanged(bool on),
                   noticePowerChanged(on, this)                                );
IF_IMPL_SENDER  (  IRadioDevice::notifyStationChanged (const RadioStation &s),
                   noticeStationChanged (s, this)                              );
IF_IMPL_SENDER  (  IRadioDevice::notifyDescriptionChanged (const QString&s),
                   noticeDescriptionChanged (s, this)                          );
IF_IMPL_SENDER  (  IRadioDevice::notifyCurrentSoundStreamIDChanged(SoundStreamID id),
                   noticeCurrentSoundStreamIDChanged(id, this)                 );

// IRadioDeviceClient

IF_IMPL_SENDER  (  IRadioDeviceClient::sendPower(bool on),
                   setPower(on)                                   );
IF_IMPL_SENDER  (  IRadioDeviceClient::sendPowerOn(),
                   powerOn()                                      );
IF_IMPL_SENDER  (  IRadioDeviceClient::sendPowerOff(),
                   powerOff()                                     );
IF_IMPL_SENDER  (  IRadioDeviceClient::sendActivateStation(const RadioStation &rs),
                   activateStation(rs)                            );

IF_IMPL_QUERY   (  bool IRadioDeviceClient::queryIsPowerOn(),
                   isPowerOn(),
                   false                     );
IF_IMPL_QUERY   (  bool IRadioDeviceClient::queryIsPowerOff(),
                   isPowerOff(),
                   true                      );
IF_IMPL_QUERY   (  const RadioStation  &  IRadioDeviceClient::queryCurrentStation(),
                   getCurrentStation(),
                   undefinedRadioStation     );

static QString IRadioDeviceClient_unknown("unknown");
IF_IMPL_QUERY   (  const QString       &  IRadioDeviceClient::queryDescription(),
                   getDescription(),
                   IRadioDeviceClient_unknown                   );

IF_IMPL_QUERY   (  SoundStreamID IRadioDeviceClient::queryCurrentSoundStreamID(),
                   getCurrentSoundStreamID(),
                   SoundStreamID::InvalidID  );

void IRadioDeviceClient::noticeConnectedI    (cmplInterface *c, bool pointer_valid)
{
    noticePowerChanged(queryIsPowerOn());
    noticeStationChanged(queryCurrentStation(), pointer_valid ? c : NULL);
    noticeDescriptionChanged(queryDescription(), pointer_valid ? c : NULL);
}

void IRadioDeviceClient::noticeDisconnectedI    (cmplInterface *c, bool pointer_valid)
{
    noticePowerChanged(queryIsPowerOn());
    noticeStationChanged(queryCurrentStation(), pointer_valid ? c : NULL);
    noticeDescriptionChanged(queryDescription(), pointer_valid ? c : NULL);
}




/* Deprecated
// IRadioSound

IF_IMPL_SENDER  (  IRadioSound::notifyVolumeChanged(float v),
                   noticeVolumeChanged(v)                   )
IF_IMPL_SENDER  (  IRadioSound::notifyTrebleChanged(float v),
                   noticeTrebleChanged(v)                   )
IF_IMPL_SENDER  (  IRadioSound::notifyBassChanged(float v),
                   noticeBassChanged(v)                     )
IF_IMPL_SENDER  (  IRadioSound::notifyBalanceChanged(float v),
                   noticeBalanceChanged(v)                  )
IF_IMPL_SENDER  (  IRadioSound::notifySignalQualityChanged(float q),
                   noticeSignalQualityChanged(q)            )
IF_IMPL_SENDER  (  IRadioSound::notifySignalQualityBoolChanged(bool good),
                   noticeSignalQualityChanged(good)         )
IF_IMPL_SENDER  (  IRadioSound::notifySignalMinQualityChanged(float q),
                   noticeSignalMinQualityChanged(q)         )
IF_IMPL_SENDER  (  IRadioSound::notifyStereoChanged(bool  s),
                   noticeStereoChanged(s)                   )
IF_IMPL_SENDER  (  IRadioSound::notifyMuted(bool m),
                   noticeMuted(m)                           )

// IRadioSoundClient

IF_IMPL_SENDER  (  IRadioSoundClient::sendVolume (float v),
                   setVolume (v)                            )
IF_IMPL_SENDER  (  IRadioSoundClient::sendTreble (float v),
                   setTreble (v)                            )
IF_IMPL_SENDER  (  IRadioSoundClient::sendBass (float v),
                   setBass (v)                              )
IF_IMPL_SENDER  (  IRadioSoundClient::sendBalance (float v),
                   setBalance (v)                           )
IF_IMPL_SENDER  (  IRadioSoundClient::sendMute (bool mute),
                   mute (mute)                              )
IF_IMPL_SENDER  (  IRadioSoundClient::sendUnmute (bool unmute),
                   unmute (unmute)                          )
IF_IMPL_SENDER  (  IRadioSoundClient::sendSignalMinQuality (float q),
                   setSignalMinQuality (q)                  )
IF_IMPL_SENDER  (  IRadioSoundClient::sendStereo(bool s),
                   setStereo(s)                             )

IF_IMPL_QUERY   (  float  IRadioSoundClient::queryVolume(),
                   getVolume(),
                   0.0            )
IF_IMPL_QUERY   (  float  IRadioSoundClient::queryTreble(),
                   getTreble(),
                   0.0            )
IF_IMPL_QUERY   (  float  IRadioSoundClient::queryBass(),
                   getBass(),
                   0.0            )
IF_IMPL_QUERY   (  float  IRadioSoundClient::queryBalance(),
                   getBalance(),
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


void IRadioSoundClient::noticeConnectedI    (cmplInterface *, bool pointer_valid)
{
    noticeVolumeChanged          (queryVolume());
    noticeTrebleChanged          (queryTreble());
    noticeBassChanged            (queryBass());
    noticeBalanceChanged         (queryBalance());
    noticeSignalQualityChanged   (querySignalQuality());
    noticeSignalQualityChanged   (queryHasGoodQuality());
    noticeSignalMinQualityChanged(querySignalMinQuality());
    noticeStereoChanged          (queryIsStereo());
    noticeMuted                  (queryIsMuted());
}


void IRadioSoundClient::noticeDisconnectedI   (cmplInterface *, bool pointer_valid)
{
    noticeVolumeChanged          (queryVolume());
    noticeTrebleChanged          (queryTreble());
    noticeBassChanged            (queryBass());
    noticeBalanceChanged         (queryBalance());
    noticeSignalQualityChanged   (querySignalQuality());
    noticeSignalQualityChanged   (queryHasGoodQuality());
    noticeSignalMinQualityChanged(querySignalMinQuality());
    noticeStereoChanged          (queryIsStereo());
    noticeMuted                  (queryIsMuted());
}

*/



// ISeekRadio

IF_IMPL_SENDER  (  ISeekRadio::notifySeekStarted (bool up),
                   noticeSeekStarted (up)                         );
IF_IMPL_SENDER  (  ISeekRadio::notifySeekStopped (),
                   noticeSeekStopped ()                           );
IF_IMPL_SENDER  (  ISeekRadio::notifySeekFinished (const RadioStation &s, bool goodQuality),
                   noticeSeekFinished (s, goodQuality)            );
IF_IMPL_SENDER  (  ISeekRadio::notifyProgress (float f),
                   noticeProgress (f)                             );


// ISeekRadioClient

IF_IMPL_SENDER  (  ISeekRadioClient::sendToBeginning(),
                   toBeginning()                                  );
IF_IMPL_SENDER  (  ISeekRadioClient::sendToEnd(),
                   toEnd()                                        );
IF_IMPL_SENDER  (  ISeekRadioClient::sendStartSeek (bool up),
                   startSeek (up)                                 );
IF_IMPL_SENDER  (  ISeekRadioClient::sendStartSeekUp(),
                   startSeekUp()                                  );
IF_IMPL_SENDER  (  ISeekRadioClient::sendStartSeekDown(),
                   startSeekDown()                                );
IF_IMPL_SENDER  (  ISeekRadioClient::sendStopSeek(),
                   stopSeek()                                     );

IF_IMPL_QUERY   (  bool ISeekRadioClient::queryIsSeekRunning(),
                   isSeekRunning(),
                   false                                          );
IF_IMPL_QUERY   (  bool ISeekRadioClient::queryIsSeekUpRunning(),
                   isSeekUpRunning(),
                   false                                          );
IF_IMPL_QUERY   (  bool ISeekRadioClient::queryIsSeekDownRunning(),
                   isSeekDownRunning(),
                   false                                          );
IF_IMPL_QUERY   (  float ISeekRadioClient::queryProgress(),
                   getProgress(),
                   1.0                                            );


void ISeekRadioClient::noticeConnectedI    (cmplInterface *, bool /*pointer_valid*/)
{
    if (queryIsSeekRunning()) {
        noticeSeekStarted(queryIsSeekUpRunning());
    } else {
        noticeSeekStopped();
    }
    noticeProgress(queryProgress());
}


void ISeekRadioClient::noticeDisconnectedI   (cmplInterface *, bool /*pointer_valid*/)
{
    noticeSeekStopped();
    noticeProgress(queryProgress());
}


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
                   0.05                                           )

void IFrequencyRadioClient::noticeConnectedI    (cmplInterface *, bool /*pointer_valid*/)
{
    noticeFrequencyChanged(queryFrequency(), NULL);
    noticeMinMaxFrequencyChanged(queryMinFrequency(), queryMaxFrequency());
    noticeDeviceMinMaxFrequencyChanged(queryMinDeviceFrequency(), queryMaxDeviceFrequency());
    noticeScanStepChanged(queryScanStep());
}


void IFrequencyRadioClient::noticeDisconnectedI   (cmplInterface *, bool /*pointer_valid*/)
{
    noticeFrequencyChanged(queryFrequency(), NULL);
    noticeMinMaxFrequencyChanged(queryMinFrequency(), queryMaxFrequency());
    noticeDeviceMinMaxFrequencyChanged(queryMinDeviceFrequency(), queryMaxDeviceFrequency());
    noticeScanStepChanged(queryScanStep());
}



// IInternetRadio

IF_IMPL_SENDER  (  IInternetRadio::notifyURLChanged(const KURL &u),
                   noticeURLChanged(u)                            )

// IInternetRadioClient

IF_IMPL_SENDER  (  IInternetRadioClient::sendURL(const KURL &url),
                   setURL(url)                                    )


static KURL emptyURL;

IF_IMPL_QUERY   (  const KURL &IInternetRadioClient::queryURL(),
                   getURL(),
                   emptyURL                                       )

void IInternetRadioClient::noticeConnectedI    (cmplInterface *, bool /*pointer_valid*/)
{
    noticeURLChanged(queryURL());
}


void IInternetRadioClient::noticeDisconnectedI (cmplInterface *, bool /*pointer_valid*/)
{
    noticeURLChanged(queryURL());
}

