/***************************************************************************
                          radiodevicepool_interface.h  -  description
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

#ifndef KRADIO_RADIODEVICEPOOL_INTERFACES_H
#define KRADIO_RADIODEVICEPOOL_INTERFACES_H

#include "interfaces.h"


class IRadioDevice;


INTERFACE(IRadioDevicePool, IRadioDevicePoolClient)
{
public:
	IF_CON_DESTRUCTOR(IRadioDevicePool, -1)

RECEIVERS:
	IF_RECEIVER(  setActiveDevice(IRadioDevice *rd, bool keepPower = true))

SENDERS:
	IF_SENDER  (  notifyActiveDeviceChanged(IRadioDevice *rd)             )
	IF_SENDER  (  notifyDevicesChanged(const QList<IRadioDevice*> &)      )
	IF_SENDER  (  notifyDeviceDescriptionChanged(const QString &)         )

ANSWERS:
	IF_ANSWER  (  IRadioDevice                  * getActiveDevice() const )
	IF_ANSWER  (  const QList<IRadioDevice*>    & getDevices() const      )
	IF_ANSWER  (  const QString                 & getDeviceDescription() const )
};


INTERFACE(IRadioDevicePoolClient, IRadioDevicePool)
{
public:
	IF_CON_DESTRUCTOR(IRadioDevicePoolClient, -1)

SENDERS:
	IF_SENDER  (  sendActiveDevice(IRadioDevice *rd, bool keepPower = true))

RECEIVERS:
	IF_RECEIVER(  noticeActiveDeviceChanged(IRadioDevice *rd)             )
	IF_RECEIVER(  noticeDevicesChanged(const QList<IRadioDevice*> &)    )
	IF_RECEIVER(  noticeDeviceDescriptionChanged(const QString &)         )

QUERIES:
	IF_QUERY   (  IRadioDevice                  *queryActiveDevice()      )
	IF_QUERY   (  const QList<IRadioDevice*>    &queryDevices()           )
	IF_QUERY   (  const QString                 &queryDeviceDescription() )

RECEIVERS:
	virtual void noticeConnectedI    (cmplInterface *, bool /*pointer_valid*/);
	virtual void noticeDisconnectedI (cmplInterface *, bool /*pointer_valid*/);
};


#endif
