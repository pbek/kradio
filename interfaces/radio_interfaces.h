/***************************************************************************
                          radio_interfaces.h  -  description
                             -------------------
    begin                : Mon Mär 10 2003
    copyright            : (C) 2003 by Martin Witte
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

/***************************************************************************
 *                                                                         *
 *   Interfaces in this header:                                            *
 *                                                                         *
 *   IRadio(Client)                                                        *
 *                                                                         *
 ***************************************************************************/

#ifndef KRADIO_RADIO_INTERFACES_H
#define KRADIO_RADIO_INTERFACES_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "interfaces.h"
#include "soundstreamid.h"

class RadioStation;
class StationList;


///////////////////////////////////////////////////////////////////////


INTERFACE(IRadio, IRadioClient)
{
public :
    IF_CON_DESTRUCTOR(IRadio, -1)

RECEIVERS:
    IF_RECEIVER(  powerOn()                                      )
    IF_RECEIVER(  powerOff()                                     )
    IF_RECEIVER(  activateStation(const RadioStation &rs)        )
    IF_RECEIVER(  activateStation(int index)                     )
    IF_RECEIVER(  setStations(const StationList &sl)             )
    IF_RECEIVER(  setPresetFile(const QString &f)                )

SENDERS:
    IF_SENDER  (  notifyPowerChanged(bool on)                            )
    IF_SENDER  (  notifyStationChanged (const RadioStation &, int idx)   )
    IF_SENDER  (  notifyStationsChanged(const StationList &sl)           )
    IF_SENDER  (  notifyPresetFileChanged(const QString &sl)             )
    IF_SENDER  (  notifyCurrentSoundStreamSinkIDChanged(SoundStreamID id)  )
    IF_SENDER  (  notifyCurrentSoundStreamSourceIDChanged(SoundStreamID id))

    IF_SENDER  (  notifyRDSStateChanged      (bool enabled)              )
    IF_SENDER  (  notifyRDSRadioTextChanged  (const QString &s)          )
    IF_SENDER  (  notifyRDSStationNameChanged(const QString &s)          )

ANSWERS:
    IF_ANSWER  (  bool                   isPowerOn() const               )
    IF_ANSWER  (  bool                   isPowerOff() const              )
    IF_ANSWER  (  const RadioStation  &  getCurrentStation() const       )
    IF_ANSWER  (  int                    getStationIdx(const RadioStation &rs) const )
    IF_ANSWER  (  int                    getCurrentStationIdx() const    )
    IF_ANSWER  (  const StationList   &  getStations() const             )
    IF_ANSWER  (  const QString       &  getPresetFile() const           )

    IF_ANSWER  (  bool                   getRDSState () const            )
    IF_ANSWER  (  const QString       &  getRDSRadioText() const         )
    IF_ANSWER  (  const QString       &  getRDSStationName() const       )

    IF_ANSWER  (  SoundStreamID          getCurrentSoundStreamSourceID() const )
    IF_ANSWER  (  SoundStreamID          getCurrentSoundStreamSinkID() const )

};


INTERFACE(IRadioClient, IRadio)
{
friend class IRadio;

public :
    IF_CON_DESTRUCTOR(IRadioClient, 1)

SENDERS:
    IF_SENDER  (  sendPowerOn()                                                   )
    IF_SENDER  (  sendPowerOff()                                                  )
    IF_SENDER  (  sendActivateStation(const RadioStation &rs)                     )
    IF_SENDER  (  sendActivateStation(int index)                                  )
    IF_SENDER  (  sendStations(const StationList &sl)                             )
    IF_SENDER  (  sendPresetFile(const QString &f)                                )

RECEIVERS:
    IF_RECEIVER(  noticePowerChanged(bool on)                                     )
    IF_RECEIVER(  noticeStationChanged (const RadioStation &, int idx)            )
    IF_RECEIVER(  noticeStationsChanged(const StationList &sl)                    )
    IF_RECEIVER(  noticePresetFileChanged(const QString &f)                       )
    IF_RECEIVER(  noticeCurrentSoundStreamSinkIDChanged(SoundStreamID id)         )
    IF_RECEIVER(  noticeCurrentSoundStreamSourceIDChanged(SoundStreamID id)       )

    IF_RECEIVER(  noticeRDSStateChanged      (bool enabled)                       )
    IF_RECEIVER(  noticeRDSRadioTextChanged  (const QString &s)                   )
    IF_RECEIVER(  noticeRDSStationNameChanged(const QString &s)                   )


QUERIES:
    IF_QUERY   (  bool                   queryIsPowerOn()                         )
    IF_QUERY   (  bool                   queryIsPowerOff()                        )
    IF_QUERY   (  const RadioStation  &  queryCurrentStation()                    )
    IF_QUERY   (  int                    queryStationIdx(const RadioStation &rs)  )
    IF_QUERY   (  int                    queryCurrentStationIdx()                 )
    IF_QUERY   (  const StationList   &  queryStations()                          )
    IF_QUERY   (  const QString       &  queryPresetFile()                        )

    IF_QUERY   (  bool                   queryRDSState ()                         )
    IF_QUERY   (  const QString       &  queryRDSRadioText()                      )
    IF_QUERY   (  const QString       &  queryRDSStationName()                    )


    IF_QUERY   (  SoundStreamID          queryCurrentSoundStreamSinkID()          )
    IF_QUERY   (  SoundStreamID          queryCurrentSoundStreamSourceID()        )

RECEIVERS:
    virtual void noticeConnectedI    (cmplInterface *, bool pointer_valid);
    virtual void noticeDisconnectedI (cmplInterface *, bool pointer_valid);
};



#endif
