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

#include "soundstreamid.h"
#include "interfaces.h"

class RadioStation;
class KURL;

INTERFACE(IRadioDevice, IRadioDeviceClient)
{
public:
    IF_CON_DESTRUCTOR(IRadioDevice, -1)

RECEIVERS:
    IF_RECEIVER(  setPower(bool on)                                )
    IF_RECEIVER(  powerOn()                                        )
    IF_RECEIVER(  powerOff()                                       )
    IF_RECEIVER(  activateStation(const RadioStation &rs)          )

SENDERS:
    IF_SENDER  (  notifyPowerChanged(bool on)                        )
    IF_SENDER  (  notifyStationChanged (const RadioStation &)        )
    IF_SENDER  (  notifyDescriptionChanged (const QString &)         )
    IF_SENDER  (  notifyCurrentSoundStreamIDChanged(SoundStreamID id))

ANSWERS:
    IF_ANSWER  (  bool                   isPowerOn() const              );
    IF_ANSWER  (  bool                   isPowerOff() const             );
    IF_ANSWER  (  const RadioStation  &  getCurrentStation() const      );
    IF_ANSWER  (  const QString       &  getDescription() const         );

    IF_ANSWER  (  SoundStreamID          getCurrentSoundStreamID() const  );
};



INTERFACE(IRadioDeviceClient, IRadioDevice)
{
public :
    IF_CON_DESTRUCTOR(IRadioDeviceClient, 1);
    IRadioDeviceClient(int _maxConnections) : IRadioDeviceClient::BaseClass(_maxConnections) {}

SENDERS:
    IF_SENDER  (  sendPower(bool on)                                 )
    IF_SENDER  (  sendPowerOn()                                      )
    IF_SENDER  (  sendPowerOff()                                     )
    IF_SENDER  (  sendActivateStation (const RadioStation &rs)       )

RECEIVERS:
    IF_RECEIVER(  noticePowerChanged   (bool on, const IRadioDevice *sender = NULL)                     )
    IF_RECEIVER(  noticeStationChanged (const RadioStation &, const IRadioDevice *sender = NULL)        )
    IF_RECEIVER(  noticeDescriptionChanged (const QString &, const IRadioDevice *sender = NULL)         )
    IF_RECEIVER(  noticeCurrentSoundStreamIDChanged(SoundStreamID id, const IRadioDevice *sender = NULL))

QUERIES:
    IF_QUERY   (  bool                   queryIsPowerOn()             )
    IF_QUERY   (  bool                   queryIsPowerOff()            )
    IF_QUERY   (  const RadioStation  &  queryCurrentStation()        )
    IF_QUERY   (  const QString       &  queryDescription()           )

    IF_QUERY   (  SoundStreamID          queryCurrentSoundStreamID()    )

RECEIVERS:
    virtual void noticeConnectedI    (cmplInterface *, bool pointer_valid);
    virtual void noticeDisconnectedI (cmplInterface *, bool pointer_valid);
};


/////////////////////////////////////////////////////////////////////////////
// deprecated, use IRadioSoundStreamClient
/*
INTERFACE(IRadioSound, IRadioSoundClient)
{
public :
    IF_CON_DESTRUCTOR(IRadioSound, -1)

RECEIVERS:
    IF_RECEIVER(  setVolume (float v)                            )
    IF_RECEIVER(  setTreble (float v)                            )
    IF_RECEIVER(  setBass   (float v)                            )
    IF_RECEIVER(  setBalance (float v)                           )
    IF_RECEIVER(  mute (bool mute)                               )
    IF_RECEIVER(  unmute (bool unmute)                           )
    IF_RECEIVER(  setSignalMinQuality(float q)                   )
    IF_RECEIVER(  setStereo(bool s)                              )

SENDERS:
    IF_SENDER  (  notifyVolumeChanged(float v)                   )
    IF_SENDER  (  notifyTrebleChanged(float v)                   )
    IF_SENDER  (  notifyBassChanged(float v)                     )
    IF_SENDER  (  notifyBalanceChanged(float v)                  )
    IF_SENDER  (  notifyMuted(bool m)                            )
    IF_SENDER  (  notifySignalQualityChanged(float q)            )
    IF_SENDER  (  notifySignalQualityBoolChanged(bool good)          )
    IF_SENDER  (  notifySignalMinQualityChanged(float q)         )
    IF_SENDER  (  notifyStereoChanged(bool  s)                   )

ANSWERS:
    IF_ANSWER  (  float   getVolume() const                      )
    IF_ANSWER  (  float   getTreble() const                      )
    IF_ANSWER  (  float   getBass  () const                      )
    IF_ANSWER  (  float   getBalance () const                    )
    IF_ANSWER  (  bool    isMuted() const                        )
    IF_ANSWER  (  float   getSignalQuality() const               )
    IF_ANSWER  (  float   getSignalMinQuality() const            )
    IF_ANSWER  (  bool    hasGoodQuality() const                 )
    IF_ANSWER  (  bool    isStereo() const                       )
};


INTERFACE(IRadioSoundClient, IRadioSound)
{
public :
    IF_CON_DESTRUCTOR(IRadioSoundClient, 1)

SENDERS:
    IF_SENDER  (  sendVolume  (float v)                          )
    IF_SENDER  (  sendTreble  (float v)                          )
    IF_SENDER  (  sendBass    (float v)                          )
    IF_SENDER  (  sendBalance (float v)                          )
    IF_SENDER  (  sendMute (bool mute = true)                    )
    IF_SENDER  (  sendUnmute (bool unmute = true)                )
    IF_SENDER  (  sendSignalMinQuality (float q)                 )
    IF_SENDER  (  sendStereo(bool s)                             )

RECEIVERS:
    IF_RECEIVER(  noticeVolumeChanged(float v)                   )
    IF_RECEIVER(  noticeTrebleChanged(float v)                   )
    IF_RECEIVER(  noticeBassChanged(float v)                     )
    IF_RECEIVER(  noticeBalanceChanged(float v)                  )
    IF_RECEIVER(  noticeSignalQualityChanged(float q)            )
    IF_RECEIVER(  noticeSignalQualityChanged(bool good)          )
    IF_RECEIVER(  noticeSignalMinQualityChanged(float q)         )
    IF_RECEIVER(  noticeStereoChanged(bool  s)                   )
    IF_RECEIVER(  noticeMuted(bool m)                            )

QUERIES:
    IF_QUERY   (  float   queryVolume()                          )
    IF_QUERY   (  float   queryTreble()                          )
    IF_QUERY   (  float   queryBass()                            )
    IF_QUERY   (  float   queryBalance ()                        )
    IF_QUERY   (  float   querySignalQuality()                   )
    IF_QUERY   (  float   querySignalMinQuality()                )
    IF_QUERY   (  bool    queryHasGoodQuality()                  )
    IF_QUERY   (  bool    queryIsStereo()                        )
    IF_QUERY   (  bool    queryIsMuted()                         )

RECEIVERS:
    virtual void noticeConnectedI    (cmplInterface *, bool pointer_valid);
    virtual void noticeDisconnectedI (cmplInterface *, bool pointer_valid);
};
*/

/////////////////////////////////////////////////////////////////////////////
INTERFACE(ISeekRadio, ISeekRadioClient)
{
    friend class SeekHelper;

public :
    IF_CON_DESTRUCTOR(ISeekRadio, -1)

RECEIVERS:
    IF_RECEIVER(  toBeginning()                                  )
    IF_RECEIVER(  toEnd()                                        )
    IF_RECEIVER(  startSeek (bool up)                            )
    IF_RECEIVER(  startSeekUp()                                  )
    IF_RECEIVER(  startSeekDown()                                )
    IF_RECEIVER(  stopSeek()                                     )

SENDERS:
    IF_SENDER  (  notifySeekStarted (bool up)                    )
    IF_SENDER  (  notifySeekStopped ()                           )
    IF_SENDER  (  notifySeekFinished (const RadioStation &s, bool goodQuality)     )
    IF_SENDER  (  notifyProgress (float f)                       )

ANSWERS:
    IF_ANSWER  (  bool  isSeekRunning() const                    )
    IF_ANSWER  (  bool  isSeekUpRunning() const                  )
    IF_ANSWER  (  bool  isSeekDownRunning() const                )
    IF_ANSWER  (  float getProgress () const                     )
};


INTERFACE(ISeekRadioClient, ISeekRadio)
{
public :
    IF_CON_DESTRUCTOR(ISeekRadioClient, 1)

SENDERS:
    IF_SENDER  (  sendToBeginning()                                   )
    IF_SENDER  (  sendToEnd()                                         )
    IF_SENDER  (  sendStartSeek (bool up)                             )
    IF_SENDER  (  sendStartSeekUp()                                   )
    IF_SENDER  (  sendStartSeekDown()                                 )
    IF_SENDER  (  sendStopSeek()                                      )

RECEIVERS:
    IF_RECEIVER(  noticeSeekStarted (bool up)                         )
    IF_RECEIVER(  noticeSeekStopped ()                                )
    IF_RECEIVER(  noticeSeekFinished (const RadioStation &s, bool goodQuality)          )
    IF_RECEIVER(  noticeProgress (float f)                            )

QUERIES:
    IF_QUERY  (  bool  queryIsSeekRunning()                           )
    IF_QUERY  (  bool  queryIsSeekUpRunning()                         )
    IF_QUERY  (  bool  queryIsSeekDownRunning()                       )
    IF_QUERY  (  float queryProgress ()                               )

RECEIVERS:
    virtual void noticeConnectedI    (cmplInterface *, bool pointer_valid);
    virtual void noticeDisconnectedI (cmplInterface *, bool pointer_valid);
};



/////////////////////////////////////////////////////////////////////////////

INTERFACE(IFrequencyRadio, IFrequencyRadioClient)
{
public :
    IF_CON_DESTRUCTOR(IFrequencyRadio, -1)

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
    IF_CON_DESTRUCTOR(IFrequencyRadioClient, 1)

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

RECEIVERS:
    virtual void noticeConnectedI    (cmplInterface *, bool pointer_valid);
    virtual void noticeDisconnectedI (cmplInterface *, bool pointer_valid);
};


/////////////////////////////////////////////////////////////////////////////

INTERFACE(IInternetRadio, IInternetRadioClient)
{
public :
    IF_CON_DESTRUCTOR(IInternetRadio, -1)

RECEIVERS:
    IF_RECEIVER(  setURL(const KURL &url)                                        )

SENDERS:
    IF_SENDER  (  notifyURLChanged(const KURL &u)                                )

ANSWERS:
    IF_ANSWER  (  const KURL &          getURL() const                           )
};




INTERFACE(IInternetRadioClient, IInternetRadio)
{
public :
    IF_CON_DESTRUCTOR(IInternetRadioClient, 1)


SENDERS:
    IF_SENDER  (  sendURL(const KURL &url)                                       )

RECEIVERS:
    IF_RECEIVER(  noticeURLChanged(const KURL &url)                              )

QUERIES:
    IF_QUERY   (  const KURL &queryURL()                                         )

RECEIVERS:
    virtual void noticeConnectedI    (cmplInterface *, bool pointer_valid);
    virtual void noticeDisconnectedI (cmplInterface *, bool pointer_valid);
};



#endif

