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

#include "stationlist.h"
#include "radiostation.h"

// IRadio

IF_IMPL_SENDER  (  IRadio::notifyPowerOn(),
                   noticePowerOn()                                         )
IF_IMPL_SENDER  (  IRadio::notifyPowerOff(),
                   noticePowerOff()                                        )
IF_IMPL_SENDER  (  IRadio::notifyStationChanged (const RadioStation &s),
                   noticeStationChanged (s)                                )
IF_IMPL_SENDER  (  IRadio::notifyStationsChanged(const StationList &sl),
                   noticeStationsChanged(sl)                               )

// IRadioClient

IF_IMPL_SENDER  (  IRadioClient::sendPowerOn(),
                   powerOn()                                      )
IF_IMPL_SENDER  (  IRadioClient::sendPowerOff(),
                   powerOff()                                     )
IF_IMPL_SENDER  (  IRadioClient::sendActivateStation(const RadioStation &rs),
                   activateStation(rs)                            )
IF_IMPL_SENDER  (  IRadioClient::sendActivateStation(int index),
                   activateStation(index)                         )
IF_IMPL_SENDER  (  IRadioClient::sendStations(const StationList &sl),
                   setStations(sl)                                )

IF_IMPL_QUERY   (  bool IRadioClient::queryIsPowerOn(),
                   isPowerOn(),
                   false                     )
IF_IMPL_QUERY   (  bool IRadioClient::queryIsPowerOff(),
                   isPowerOff(),
                   true                      )

IF_IMPL_QUERY   (  const RadioStation  &  IRadioClient::queryCurrentStation(),
                   getCurrentStation(),
                   undefinedRadioStation          )

IF_IMPL_QUERY   (  const StationList &  IRadioClient::queryStations(),
                   getStations(),
                   emptyStationList               )

