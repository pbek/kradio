/***************************************************************************
                          radio_interfaces.h  -  description
                             -------------------
    begin                : Mon M�r 10 2003
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
 *                                                                         *
 ***************************************************************************/

#ifndef KRADIO_RADIO_INTERFACES_H
#define KRADIO_RADIO_INTERFACES_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "interfaces.h"

class RadioStation;
class StationList;


///////////////////////////////////////////////////////////////////////


INTERFACE(IRadio, IRadioClient)
{
public :
    IRadio() : IRadio::BaseClass (-1) {}

RECEIVERS:
	IF_RECEIVER(  powerOn()                                      )
	IF_RECEIVER(  powerOff()                                     )
    IF_RECEIVER(  activateStation(const RadioStation &rs)        )
    IF_RECEIVER(  activateStation(int index)                     )
	IF_RECEIVER(  setStations(const StationList &sl)             )

SENDERS:
	IF_SENDER  (  notifyPowerOn()                                )
	IF_SENDER  (  notifyPowerOff()                               )
	IF_SENDER  (  notifyStationChanged (const RadioStation &)    )
	IF_SENDER  (  notifyStationsChanged(const StationList &sl)   )

ANSWERS:
	IF_ANSWER  (  bool                   isPowerOn() const         )
	IF_ANSWER  (  bool                   isPowerOff() const        )
	IF_ANSWER  (  const RadioStation  &  getCurrentStation() const )
	IF_ANSWER  (  const StationList   &  getStations() const       )

};


INTERFACE(IRadioClient, IRadio)
{
public :
    IRadioClient() : IRadioClient::BaseClass (-1) {}

SENDERS:
	IF_SENDER  (  sendPowerOn()                                      )
	IF_SENDER  (  sendPowerOff()                                     )
    IF_SENDER  (  sendActivateStation(const RadioStation &rs)        )
    IF_SENDER  (  sendActivateStation(int index)                     )
	IF_SENDER  (  sendStations(const StationList &sl)                )

RECEIVERS:
	IF_RECEIVER(  noticePowerOn()                                  )
	IF_RECEIVER(  noticePowerOff()                                 )
	IF_RECEIVER(  noticeStationChanged (const RadioStation &)      )
	IF_RECEIVER(  noticeStationsChanged(const StationList &sl)     )

QUERIES:
	IF_QUERY   (  bool                   queryIsPowerOn()          )
	IF_QUERY   (  bool                   queryIsPowerOff()         )
	IF_QUERY   (  const RadioStation  &  queryCurrentStation()     )
	IF_QUERY   (  const StationList &    queryStations()           )

};



#endif