/***************************************************************************
                          radiodevicepool_interface.cpp  -  description
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


#include "radiodevicepool_interfaces.h"
#include "radiodevice_interfaces.h"

// IRadioDevicePool

IF_IMPL_SENDER  (  IRadioDevicePool::notifyActiveDeviceChanged(IRadioDevice *rd),
                   noticeActiveDeviceChanged(rd)      )
IF_IMPL_SENDER  (  IRadioDevicePool::notifyDevicesChanged(const QPtrList<IRadioDevice> &l),
                   noticeDevicesChanged(l)            )
IF_IMPL_SENDER  (  IRadioDevicePool::notifyDeviceDescriptionChanged(const QString &s),
                   noticeDeviceDescriptionChanged(s)  )

// IRadioDevicePoolClient

IF_IMPL_SENDER  (  IRadioDevicePoolClient::sendActiveDevice(IRadioDevice *rd, bool keepPower ),
                   setActiveDevice(rd, keepPower)     )

IF_IMPL_QUERY   (  IRadioDevice *IRadioDevicePoolClient::queryActiveDevice(),
                   getActiveDevice(),
                   NULL                               )

static const QPtrList<IRadioDevice> IRadioDevicePoolClient_emptyList;
IF_IMPL_QUERY   (  const QPtrList<IRadioDevice> &IRadioDevicePoolClient::queryDevices(),
                   getDevices(),
                   IRadioDevicePoolClient_emptyList                          )

static QString IRadioDevicePoolClient_unknown("unknown");
IF_IMPL_QUERY   (  const QString &IRadioDevicePoolClient::queryDeviceDescription(),
                   getDeviceDescription(),
                   IRadioDevicePoolClient_unknown                            )


void IRadioDevicePoolClient::noticeConnectedI    (cmplInterface *, bool /*pointer_valid*/)
{
    noticeActiveDeviceChanged(queryActiveDevice());
    noticeDevicesChanged(queryDevices());
    noticeDeviceDescriptionChanged(queryDeviceDescription());
}

void IRadioDevicePoolClient::noticeDisconnectedI   (cmplInterface *, bool /*pointer_valid*/)
{
    noticeActiveDeviceChanged(queryActiveDevice());
    noticeDevicesChanged(queryDevices());
    noticeDeviceDescriptionChanged(queryDeviceDescription());
}


