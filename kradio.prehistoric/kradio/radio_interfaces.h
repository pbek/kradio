/***************************************************************************
                          radio_interfaces.h  -  description
                             -------------------
    begin                : Mon Mär 10 2003
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
 *   IRadio(Client)                                                        *
 *   ISeekRadio(Client)                                                    *
 *   IWaveRadio(Client)                                                    *
 *   IRadioSound(Client)                                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRADIO_RADIO_INTERFACES_H
#define KRADIO_RADIO_INTERFACES_H

#include "interfaces.h"

INTERFACE(IRadio, IRadioClient)
{
public :
    virtual int   maxConnections () const { return -1; }

	// receiving messages/commands

	IF_RECEIVER(  powerOn()                                      )
	IF_RECEIVER(  powerOff()                                     )
    IF_RECEIVER(  activateStation(const RadioStation &rs)        )
    IF_RECEIVER(  activateStation(int index)                     )
	IF_RECEIVER(  setStations(const StationVector &sl)           )

	// sending notifications

	IF_SENDER  (  notifyPowerOn()                                )
	IF_SENDER  (  notifyPowerOff()                               )
	IF_SENDER  (  notifyStationChanged (const RadioStation &)    )
	IF_SENDER  (  notifyStationsChanged(const StationVector &sl) )

	// answering queries

	IF_ANSWER  (  bool                   isPowerOn()             )
	IF_ANSWER  (  bool                   isPowerOff()            )
	IF_ANSWER  (  const RadioStation  &  getCurrentStation()     )
	IF_ANSWER  (  const StationVector &  getStations()           )

};


INTERFACE(IRadioClient, IRadio)
{
public :
    virtual int   maxConnections () const { return 1; }

	// sending commands

	IF_SENDER  (  powerOn()                                      )
	IF_SENDER  (  powerOff()                                     )
    IF_SENDER  (  activateStation(const RadioStation &rs)        )
    IF_SENDER  (  activateStation(int index)                     )
	IF_SENDER  (  setStations(const StationVector &sl)           )

	// receiving notifications

	IF_RECEIVER(  notifyPowerOn()                                )
	IF_RECEIVER(  notifyPowerOff()                               )
	IF_RECEIVER(  notifyStationChanged (const RadioStation &)    )
	IF_RECEIVER(  notifyStationsChanged(const StationVector &sl) )

	// queries

	IF_QUERY   (  bool                   isPowerOn()             )
	IF_QUERY   (  bool                   isPowerOff()            )
	IF_QUERY   (  const RadioStation  &  getCurrentStation()     )
 	IF_QUERY   (  const RadioStation  &  getStation (float freq) )
	IF_QUERY   (  const StationVector &  getStations()           )

};

/////////////////////////////////////////////////////////////////////////////

INTERFACE(IRadioSound, IRadioSoundClient)
{
public :
    virtual int   maxConnections () const { return -1; }

	// receiving messages/commands

	IF_RECEIVER(  setVolume (float v)                            )
	IF_RECEIVER(  mute (bool true)                               )
	IF_RECEIVER(  unmute (bool true)                             )

	// sending notifications

	IF_SENDER  (  notifyVolumeChanged(float v)                   )
	IF_SENDER  (  notifySignalQualityChanged(float q)            )
	IF_SENDER  (  notifyStereoChanged(bool  s)                   )
	IF_SENDER  (  notifyMuted(bool m)                            )

	// answering queries

	IF_ANSWER  (  float                  getVolume()             )
	IF_ANSWER  (  float                  getSignalQuality()      )
	IF_ANSWER  (  bool                   isStereo()              )
	IF_ANSWER  (  bool                   isMuted()               )
};


INTERFACE(IRadioSoundClient, IRadioSound)
{
public :
    virtual int   maxConnections () const { return 1; }

	// sending commands

	IF_SENDER  (  setVolume (float v)                            )
	IF_SENDER  (  mute (bool true)                               )
	IF_SENDER  (  unmute (bool true)                             )

	// receiving notifications

	IF_RECEIVER(  notifyVolumeChanged(float v)                   )
	IF_RECEIVER(  notifySignalQualityChanged(float q)            )
	IF_RECEIVER(  notifyStereoChanged(bool  s)                   )
	IF_RECEIVER(  notifyMuted(bool m)                            )

	// queries

	IF_QUERY   (  float                  getVolume()             )
	IF_QUERY   (  float                  getSignalQuality()      )
	IF_QUERY   (  bool                   isStereo()              )
	IF_QUERY   (  bool                   isMuted()               )
};


/////////////////////////////////////////////////////////////////////////////

INTERFACE(ISeekRadio, ISeekRadioClient)
{
public :
    virtual int   maxConnections () const { return -1; }

	// receiving messages/commands

	IF_RECEIVER(  startSeek (bool up)                            )
	IF_RECEIVER(  startSeekUp()                                  )
	IF_RECEIVER(  startSeekDown()                                )
	IF_RECEIVER(  stopSeek()                                     )

	// sending notifications

	IF_SENDER  (  notifySeekStarted (bool up)                    )
	IF_SENDER  (  notifySeekUpStarted()                          )
	IF_SENDER  (  notifySeekDownStarted()                        )
	IF_SENDER  (  notifySeekStopped ()                           )
	IF_SENDER  (  notifySeekFinished (const RadioStation &s)     )

	// answering queries

    IF_ANSWER  (  bool isSeekRunning()                           )
    IF_ANSWER  (  bool isSeekUpRunning()                         )
    IF_ANSWER  (  bool isSeekDownRunning()                       )
};


INTERFACE(ISeekRadioClient, ISeekRadio)
{
public :
    virtual int   maxConnections () const { return 1; }

	// sending commands

	IF_SENDER  (  startSeek (bool up)                            )
	IF_SENDER  (  startSeekUp()                                  )
	IF_SENDER  (  startSeekDown()                                )
	IF_SENDER  (  stopSeek()                                     )

	// receiving notifications

	IF_RECEIVER(  notifySeekStarted (bool up)                    )
	IF_RECEIVER(  notifySeekUpStarted()                          )
	IF_RECEIVER(  notifySeekDownStarted()                        )
	IF_RECEIVER(  notifySeekStopped ()                           )
	IF_RECEIVER(  notifySeekFinished (const RadioStation &s)     )

	// queries

    IF_QUERY  (  bool isSeekRunning()                            )
    IF_QUERY  (  bool isSeekUpRunning()                          )
    IF_QUERY  (  bool isSeekDownRunning()                        )
};

/////////////////////////////////////////////////////////////////////////////

INTERFACE(IWaveRadio, IWaveRadioClient)
{
public :
    virtual int   maxConnections () const { return -1; }

	// receiving messages/commands

	IF_RECEIVER(        setFrequency(float f)                                    )
	IF_RECEIVER(        setScanStep(float s)                                     )

	// sending notifications

	IF_SENDER  (        notifyFrequencyChanged(float f, const RadioStation *s)   )
	IF_SENDER  (        notifyMinMaxFrequencyChanged(float min, float max)       )
	IF_SENDER  (        notifyScanStepChanged(float s)                           )

	// answering queries

	IF_ANSWER  (  float                getFrequency()                            )
	IF_ANSWER  (  float                getMinFrequency()                         )
	IF_ANSWER  (  float                getMaxFrequency()                         )
	IF_ANSWER  (  float                getScanStep()                             )
 	IF_ANSWER  (  const RadioStation&  getStation (float freq)                   )
};


INTERFACE(IWaveRadioClient, IWaveRadio)
{
public :
    virtual int   maxConnections () const { return 1; }

	// sending commands

	IF_SENDER  (        setFrequency(float f)                                    )
	IF_SENDER  (        setScanStep(float s)                                     )

	// receiving notifications

	IF_RECEIVER(        notifyFrequencyChanged(float f, const RadioStation *s)   )
	IF_RECEIVER(        notifyMinMaxFrequencyChanged(float min, float max)       )
	IF_RECEIVER(        notifyScanStepChanged(float s)                           )

	// queries

	IF_QUERY   (  float getFrequency()                                           )
	IF_QUERY   (  float getMinFrequency()                                        )
	IF_QUERY   (  float getMaxFrequency()                                        )
	IF_QUERY   (  float getScanStep()                                            )
};



#endif
