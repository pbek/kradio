/***************************************************************************
                          radiodevicepool_interface.h  -  description
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

#ifndef KRADIO_RADIODEVICEPOOL_INTERFACES_H
#define KRADIO_RADIODEVICEPOOL_INTERFACES_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "interfaces.h"


class IRadioDevice;


INTERFACE(IRadioDevicePool, IRadioDevicePoolClient)
{
public:
	IRadioDevicePool() : IRadioDevicePool::BaseClass(-1) {}

RECEIVERS:
	IF_RECEIVER(  setActiveDevice(IRadioDevice *rd, bool keepPower = true))

SENDERS:
	IF_SENDER  (  notifyActiveDeviceChanged(IRadioDevice *rd)             )

ANSWERS:
	IF_ANSWER  (  IRadioDevice                  * getActiveDevice() const )
	IF_ANSWER  (  const QPtrList<IRadioDevice>  & getDevices() const      )
};


INTERFACE(IRadioDevicePoolClient, IRadioDevicePool)
{
public:
	IRadioDevicePoolClient() : IRadioDevicePoolClient::BaseClass(1) {}

SENDERS:
	IF_SENDER  (  sendActiveDevice(IRadioDevice *rd, bool keepPower = true))

RECEIVERS:
	IF_RECEIVER(  noticeActiveDeviceChanged(IRadioDevice *rd)             )

QUERIES:
	IF_QUERY   (  IRadioDevice                  *queryActiveDevice()      )
	IF_QUERY   (  const QPtrList<IRadioDevice>  &queryDevices()           )
};


#endif