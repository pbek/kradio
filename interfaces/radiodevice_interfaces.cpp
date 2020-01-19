/***************************************************************************
                          radiodevice_interfaces.cpp  -  description
                             -------------------
    begin                : Sam Apr 19 2003
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


#include "radiodevice_interfaces.h"
#include "radiostation.h"
#include "id-generator.h"

#include <QtCore/QUrl>
#include <kconfiggroup.h>

// IRadioDevice

IF_IMPL_SENDER  (  IRadioDevice::notifyPowerChanged(bool on),
                   noticePowerChanged(on, this)                                );
IF_IMPL_SENDER  (  IRadioDevice::notifyStationChanged (const RadioStation &s),
                   noticeStationChanged (s, this)                              );
IF_IMPL_SENDER  (  IRadioDevice::notifyDescriptionChanged (const QString&s),
                   noticeDescriptionChanged (s, this)                          );
IF_IMPL_SENDER  (  IRadioDevice::notifyCurrentSoundStreamSourceIDChanged(SoundStreamID id),
                   noticeCurrentSoundStreamSourceIDChanged(id, this)                 );
IF_IMPL_SENDER  (  IRadioDevice::notifyCurrentSoundStreamSinkIDChanged(SoundStreamID id),
                   noticeCurrentSoundStreamSinkIDChanged(id, this)                 );

IF_IMPL_SENDER  (  IRadioDevice::notifyRDSStateChanged      (bool enabled),
                   noticeRDSStateChanged(enabled, this)                        );
IF_IMPL_SENDER  (  IRadioDevice::notifyRDSRadioTextChanged  (const QString &s),
                   noticeRDSRadioTextChanged(s,   this)                        );
IF_IMPL_SENDER  (  IRadioDevice::notifyRDSStationNameChanged(const QString &s),
                   noticeRDSStationNameChanged(s, this)                        );


const QString &IRadioDevice::getRadioDeviceID()
{
    if (m_RadioDeviceID.length() == 0) {
        m_RadioDeviceID = createNewRadioDeviceID();
    }
    return m_RadioDeviceID;
}

void IRadioDevice::saveRadioDeviceID   (      KConfigGroup &c) const
{
    c.writeEntry("radio_device_id", m_RadioDeviceID);
}

void IRadioDevice::restoreRadioDeviceID(const KConfigGroup &c)
{
    m_RadioDeviceID = c.readEntry("radio_device_id", createNewRadioDeviceID());
}


QString IRadioDevice::createNewRadioDeviceID()
{
    return generateRandomID(70);
}



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

static QString emptyString;
static QString IRadioDeviceClient_unknown("unknown");

IF_IMPL_QUERY   (  const QString       &  IRadioDeviceClient::queryDescription(),
                   getDescription(),
                   IRadioDeviceClient_unknown
                );

IF_IMPL_QUERY   (  bool IRadioDeviceClient::queryRDSState (),
                   getRDSState(),
                   false
                );

IF_IMPL_QUERY   (  const QString &IRadioDeviceClient::queryRDSRadioText(),
                   getRDSRadioText(),
                   emptyString
                );

IF_IMPL_QUERY   (  const QString &IRadioDeviceClient::queryRDSStationName(),
                   getRDSStationName(),
                   emptyString
                );

IF_IMPL_QUERY   (  SoundStreamID IRadioDeviceClient::queryCurrentSoundStreamSourceID(),
                   getCurrentSoundStreamSourceID(),
                   SoundStreamID::InvalidID
                );

IF_IMPL_QUERY   (  SoundStreamID IRadioDeviceClient::queryCurrentSoundStreamSinkID(),
                   getCurrentSoundStreamSinkID(),
                   SoundStreamID::InvalidID
                );

void IRadioDeviceClient::noticeConnectedI      (cmplInterface *c, bool pointer_valid)
{
    noticePowerChanged         (queryIsPowerOn(),      pointer_valid ? c : NULL);
    noticeStationChanged       (queryCurrentStation(), pointer_valid ? c : NULL);
    noticeDescriptionChanged   (queryDescription(),    pointer_valid ? c : NULL);

    noticeRDSStateChanged      (queryRDSState(),       pointer_valid ? c : NULL);
    noticeRDSStationNameChanged(queryRDSStationName(), pointer_valid ? c : NULL);
    noticeRDSRadioTextChanged  (queryRDSRadioText(),   pointer_valid ? c : NULL);
}

void IRadioDeviceClient::noticeDisconnectedI    (cmplInterface *c, bool pointer_valid)
{
    noticePowerChanged         (queryIsPowerOn(),      pointer_valid ? c : NULL);
    noticeStationChanged       (queryCurrentStation(), pointer_valid ? c : NULL);
    noticeDescriptionChanged   (queryDescription(),    pointer_valid ? c : NULL);

    noticeRDSStateChanged      (queryRDSState(),       pointer_valid ? c : NULL);
    noticeRDSStationNameChanged(queryRDSStationName(), pointer_valid ? c : NULL);
    noticeRDSRadioTextChanged  (queryRDSRadioText(),   pointer_valid ? c : NULL);
}


