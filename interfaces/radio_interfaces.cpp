/***************************************************************************
                          radio_interfaces.cpp  -  description
                             -------------------
    begin                : Don Apr 17 2003
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

#include "radio_interfaces.h"
#include "stationlist.h"
#include "radiostation.h"

#include <QUrl>

static QString emptyString;
static QUrl    emptyUrl;

// IRadio

IF_IMPL_SENDER  (  IRadio::notifyPowerChanged(bool on),
                   noticePowerChanged(on)                                         );
IF_IMPL_SENDER  (  IRadio::notifyStationChanged (const RadioStation &s, int idx),
                   noticeStationChanged (s, idx)                                  );
IF_IMPL_SENDER  (  IRadio::notifyStationsChanged(const StationList &sl),
                   noticeStationsChanged(sl)                                      );
IF_IMPL_SENDER  (  IRadio::notifyPresetFileChanged(const QUrl &f),
                   noticePresetFileChanged(f)                                     );
IF_IMPL_SENDER  (  IRadio::notifyCurrentSoundStreamSinkIDChanged(SoundStreamID id),
                   noticeCurrentSoundStreamSinkIDChanged(id)                      );
IF_IMPL_SENDER  (  IRadio::notifyCurrentSoundStreamSourceIDChanged(SoundStreamID id),
                   noticeCurrentSoundStreamSourceIDChanged(id)                    );

IF_IMPL_SENDER  (  IRadio::notifyRDSStateChanged      (bool enabled),
                   noticeRDSStateChanged(enabled)                                 );
IF_IMPL_SENDER  (  IRadio::notifyRDSRadioTextChanged  (const QString &s),
                   noticeRDSRadioTextChanged(s)                                   );
IF_IMPL_SENDER  (  IRadio::notifyRDSStationNameChanged(const QString &s),
                   noticeRDSStationNameChanged(s)                                 );

// IRadioClient

IF_IMPL_SENDER  (  IRadioClient::sendPowerOn(),
                   powerOn()                                      );
IF_IMPL_SENDER  (  IRadioClient::sendPowerOff(),
                   powerOff()                                     );
IF_IMPL_SENDER  (  IRadioClient::sendActivateStation(const RadioStation &rs),
                   activateStation(rs)                            );
IF_IMPL_SENDER  (  IRadioClient::sendActivateStation(int index),
                   activateStation(index)                         );
IF_IMPL_SENDER  (  IRadioClient::sendStations(const StationList &sl),
                   setStations(sl)                                );
IF_IMPL_SENDER  (  IRadioClient::sendPresetFile(const QUrl &f),
                   setPresetFile(f)                               );

IF_IMPL_QUERY   (  bool IRadioClient::queryIsPowerOn(),
                   isPowerOn(),
                   false
                );

IF_IMPL_QUERY   (  bool IRadioClient::queryIsPowerOff(),
                   isPowerOff(),
                   true
                );

IF_IMPL_QUERY   (  const RadioStation  &  IRadioClient::queryCurrentStation(),
                   getCurrentStation(),
                   undefinedRadioStation
                );

IF_IMPL_QUERY   (  int IRadioClient::queryCurrentStationIdx(),
                   getCurrentStationIdx(),
                   -1
                );

IF_IMPL_QUERY   (  int IRadioClient::queryStationIdx(const RadioStation &rs),
                   getStationIdx(rs),
                   -1
                 );

IF_IMPL_QUERY   (  const StationList &  IRadioClient::queryStations(),
                   getStations(),
                   emptyStationList
                );

IF_IMPL_QUERY   (  const QUrl &  IRadioClient::queryPresetFile(),
                   getPresetFile(),
                   emptyUrl
                );

IF_IMPL_QUERY   (  SoundStreamID IRadioClient::queryCurrentSoundStreamSourceID(),
                   getCurrentSoundStreamSourceID(),
                   SoundStreamID::InvalidID
                );

IF_IMPL_QUERY   (  SoundStreamID IRadioClient::queryCurrentSoundStreamSinkID(),
                   getCurrentSoundStreamSinkID(),
                   SoundStreamID::InvalidID
                );

IF_IMPL_QUERY   (  bool IRadioClient::queryRDSState (),
                   getRDSState(),
                   false
                );

IF_IMPL_QUERY   (  const QString &IRadioClient::queryRDSRadioText(),
                   getRDSRadioText(),
                   emptyString
                );

IF_IMPL_QUERY   (  const QString &IRadioClient::queryRDSStationName(),
                   getRDSStationName(),
                   emptyString
                );


void IRadioClient::noticeConnectedI    (cmplInterface *, bool /*pointer_valid*/)
{
    noticeStationsChanged      (queryStations());
    noticeStationChanged       (queryCurrentStation(), queryCurrentStationIdx());
    noticePowerChanged         (queryIsPowerOn());
    noticeRDSStateChanged      (queryRDSState());
    noticeRDSStationNameChanged(queryRDSStationName());
    noticeRDSRadioTextChanged  (queryRDSRadioText());
}

void IRadioClient::noticeDisconnectedI   (cmplInterface *, bool /*pointer_valid*/)
{
    noticeStationsChanged      (queryStations());
    noticeStationChanged       (queryCurrentStation(), queryCurrentStationIdx());
    noticePowerChanged         (queryIsPowerOn());
    noticeRDSStateChanged      (queryRDSState());
    noticeRDSStationNameChanged(queryRDSStationName());
    noticeRDSRadioTextChanged  (queryRDSRadioText());
}

