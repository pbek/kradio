/***************************************************************************
                          radio.h  -  description
                             -------------------
    begin                : Sat March 29 2003
    copyright            : (C) 2003 by Klas Kalass, Ernst Martin Witte
    email                : klas@kde.org, witte@kawo1.rwth-aachen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRADIO_RADIO_H
#define KRADIO_RADIO_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include "radio_interfaces.h"
#include "radiodevicepool_interface.h"
#include "radiodevice_interfaces.h"
#include "stationlist.h"

/**
 * The main Radio class, which is used as the interface of the radio functionality
 * to the GUI parts of the application
 * @author Klas Kalass, Ernst Martin Witte
 */

/////////////////////////////////////////////////////////////////////////////

/*  A class, that is able to manage more than one radio device, one of those
    is active at a time. This class behaves represents the active device,
    the active devices can be changed either by selecting a station or by
    explicitly changing the devices.

    At any time a valid active device exists as long as any device is connected.

*/

class Radio : public IRadio,
              public IRadioDevicePool,
              public IRadioDeviceClient
{
public:
    Radio();
    ~Radio();



    // IRadio methods

RECEIVERS:    
	bool powerOn()        { return sendPowerOn()  > 0; }
	bool powerOff()       { return sendPowerOff() > 0; }
    bool activateStation(const RadioStation &rs);
    bool activateStation(int index);
	bool setStations(const StationList &sl);

ANSWERS:
	bool                   isPowerOn() const;
	bool                   isPowerOff() const;
	const RadioStation  &  getCurrentStation() const;
	const StationList   &  getStations() const;

    

	// IRadioDevicePool methods

RECEIVERS:
	bool                           setActiveDevice(IRadioDevice *rd, bool keepPower = true);

ANSWERS:
	IRadioDevice                 * getActiveDevice() const;
	const QPtrList<IRadioDevice> & getDevices() const;



	// IRadioDeviceClient methods, even sending methods overwritten
	// to provide "1-of-N" functionality

SENDERS:
	IF_SENDER  (  sendPowerOn()                                      )
	IF_SENDER  (  sendPowerOff()                                     )
    IF_SENDER  (  sendActivateStation (const RadioStation &rs)       )

QUERIES:
	IF_QUERY   (  bool                   queryIsPowerOn()            )
	IF_QUERY   (  bool                   queryIsPowerOff()           )
	IF_QUERY   (  const RadioStation  &  queryCurrentStation()       )

RECEIVERS:
	virtual bool noticePowerOn        (IRadioDevice *sender = NULL);
	virtual bool noticePowerOff       (IRadioDevice *sender = NULL);
	virtual bool noticeStationChanged (const RadioStation &rs, IRadioDevice *sender = NULL);

	virtual void noticeConnect(IRadioDevice *rd);
	virtual void noticeDisconnect(IRadioDevice *rd);
	
protected:

    StationList    m_stationList;
    IRadioDevice  *m_activeDevice;
};


#endif