/***************************************************************************
                          radiodevice_interfaces.h  -  description
                             -------------------
    begin                : Fre Apr 18 2003
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
 *   IRadioDevice(Client)                                                  *
 *   ISeekRadio(Client)                                                    *
 *   IFrequencyRadio(Client)                                               *
 *   IInternetRadio(Client)                                                *
 *                                                                         *
 ***************************************************************************/

#ifndef KRADIO_RADIODEVICE_INTERFACES_H
#define KRADIO_RADIODEVICE_INTERFACES_H

#include "soundstreamid.h"
#include "interfaces.h"

class RadioStation;
class KConfigGroup;

INTERFACE(IRadioDevice, IRadioDeviceClient)
{
public:
    IF_CON_DESTRUCTOR(IRadioDevice, -1)

    const QString &getRadioDeviceID();
    void           saveRadioDeviceID   (      KConfigGroup &) const;
    void           restoreRadioDeviceID(const KConfigGroup &);

protected:
    static QString createNewRadioDeviceID();
    QString       m_RadioDeviceID;

RECEIVERS:
    IF_RECEIVER(  setPower(bool on)                                )
    IF_RECEIVER(  powerOn()                                        )
    IF_RECEIVER(  powerOff()                                       )
    IF_RECEIVER(  activateStation(const RadioStation &rs)          )

SENDERS:
    IF_SENDER  (  notifyPowerChanged(bool on)                            )
    IF_SENDER  (  notifyStationChanged (const RadioStation &)            )
    IF_SENDER  (  notifyDescriptionChanged (const QString &)             )
    IF_SENDER  (  notifyCurrentSoundStreamSinkIDChanged(SoundStreamID id))
    IF_SENDER  (  notifyCurrentSoundStreamSourceIDChanged(SoundStreamID id))

    IF_SENDER  (  notifyRDSStateChanged      (bool enabled)              )
    IF_SENDER  (  notifyRDSRadioTextChanged  (const QString &s)          )
    IF_SENDER  (  notifyRDSStationNameChanged(const QString &s)          )

ANSWERS:
    IF_ANSWER  (  bool                   isPowerOn() const               )
    IF_ANSWER  (  bool                   isPowerOff() const              )
    IF_ANSWER  (  const RadioStation  &  getCurrentStation() const       )
    IF_ANSWER  (  const QString       &  getDescription() const          )

    IF_ANSWER  (  bool                   getRDSState () const            )
    IF_ANSWER  (  const QString       &  getRDSRadioText() const         )
    IF_ANSWER  (  const QString       &  getRDSStationName() const       )

    IF_ANSWER  (  SoundStreamID          getCurrentSoundStreamSourceID() const );
    IF_ANSWER  (  SoundStreamID          getCurrentSoundStreamSinkID() const );
};



INTERFACE(IRadioDeviceClient, IRadioDevice)
{
public :
    IF_CON_DESTRUCTOR(IRadioDeviceClient, 1)
    IRadioDeviceClient(int _maxConnections) : IRadioDeviceClient::BaseClass(_maxConnections) {}

SENDERS:
    IF_SENDER  (  sendPower(bool on)                                 )
    IF_SENDER  (  sendPowerOn()                                      )
    IF_SENDER  (  sendPowerOff()                                     )
    IF_SENDER  (  sendActivateStation (const RadioStation &rs)       )

RECEIVERS:
    IF_RECEIVER(  noticePowerChanged         (bool on,                const IRadioDevice *sender = NULL))
    IF_RECEIVER(  noticeStationChanged       (const RadioStation &,   const IRadioDevice *sender = NULL))
    IF_RECEIVER(  noticeDescriptionChanged   (const QString &,        const IRadioDevice *sender = NULL))
    IF_RECEIVER(  noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID id, const IRadioDevice *sender = NULL))
    IF_RECEIVER(  noticeCurrentSoundStreamSourceIDChanged(SoundStreamID id, const IRadioDevice *sender = NULL))

    IF_RECEIVER(  noticeRDSStateChanged      (bool enabled,           const IRadioDevice *sender = NULL))
    IF_RECEIVER(  noticeRDSRadioTextChanged  (const QString &s,       const IRadioDevice *sender = NULL))
    IF_RECEIVER(  noticeRDSStationNameChanged(const QString &s,       const IRadioDevice *sender = NULL))

QUERIES:
    IF_QUERY   (  bool                   queryIsPowerOn()             )
    IF_QUERY   (  bool                   queryIsPowerOff()            )
    IF_QUERY   (  const RadioStation  &  queryCurrentStation()        )
    IF_QUERY   (  const QString       &  queryDescription()           )

    IF_QUERY   (  bool                   queryRDSState ()             )
    IF_QUERY   (  const QString       &  queryRDSRadioText()          )
    IF_QUERY   (  const QString       &  queryRDSStationName()        )

    IF_QUERY   (  SoundStreamID          queryCurrentSoundStreamSinkID()  )
    IF_QUERY   (  SoundStreamID          queryCurrentSoundStreamSourceID()  )

RECEIVERS:
    virtual void noticeConnectedI    (cmplInterface *, bool pointer_valid) override;
    virtual void noticeDisconnectedI (cmplInterface *, bool pointer_valid) override;
};




#endif

