/***************************************************************************
                          radio.cpp  -  description
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

#include "radio.h"
#include "radiostation.h"

/////////////////////////////////////////////////////////////////////////////

Radio::Radio()
  : IRadioDeviceClient(-1),
    m_activeDevice (NULL)
{
}


// offer new station to current device.
// if that does not accept, try all other devices.
// Any device will be powered off if it does not accept the station
bool Radio::activateStation (const RadioStation &rs) {

	if (sendActivateStation(rs)) {    // first try activeDevice

		return true;

	} else {                          // hmm... active device did not want it. Try others...

		int n = 0;

		for (IRadioDeviceClient::IFIterator it(IRadioDeviceClient::connections); it.current(); ++it) {

			if (it.current()->activateStation(rs)) {

				setActiveDevice(it.current());  // select new device
				++n;

			} else {

				it.current()->powerOff();

			}
		}

		return n > 0;
	}
}


bool Radio::activateStation(int index)
{
	if (index < 0 || index >= m_stationList.count())
		return false;

	return activateStation(m_stationList.at(index));
}


bool Radio::setStations(const StationList &sl)
{
	m_stationList = sl;
}



/* IRadioDevicePool Interface Methods

*/


bool Radio::setActiveDevice(IRadioDevice *rd, bool keepPower)
{
	// do nothing if old == new
	if (m_activeDevice == rd)
		return true;

	// check if new station is in "connections"
	// special case: rd == NULL: power off active device, new active device = NULL

	if (!rd || IRadioDeviceClient::connections.containsRef(rd)) {     // new device is ok

		// save old power state and power off old device
		bool oldPowerOn = false;
		if (m_activeDevice) {
			oldPowerOn = m_activeDevice->isPowerOn();
			m_activeDevice->powerOff();
		}

		// setup new active device && send notifications
		m_activeDevice = rd;
	    notifyActiveDeviceChanged(m_activeDevice);
	    notifyStationChanged(queryCurrentStation());

        if (keepPower)
			oldPowerOn ? sendPowerOn() : sendPowerOff();

		return true;

	} else {
		return false;
	}
}


IRadioDevice *Radio::getActiveDevice() const
{
	return m_activeDevice;
}


const QPtrList<IRadioDevice> &Radio::getDevices() const
{
	return IRadioDeviceClient::connections;
}



/* IRadioDeviceClient Interface Methods

   Many things are overwritten, particularly all sending methods
   
*/

int Radio::sendPowerOn() const
{
	return m_activeDevice ? m_activeDevice->powerOn() : 0;
}


int Radio::sendPowerOff() const
{
	return m_activeDevice ? m_activeDevice->powerOff() : 0;
}

int Radio::sendActivateStation (const RadioStation &rs) const
{
	return m_activeDevice ? m_activeDevice->activateStation(rs) : 0;
}


	
bool Radio::queryIsPowerOn() const
{
	return m_activeDevice ? m_activeDevice->isPowerOn() : false;
}


bool Radio::queryIsPowerOff() const
{
	return m_activeDevice ? m_activeDevice->isPowerOff() : true;
}


const RadioStation & Radio::queryCurrentStation() const
{
	return m_activeDevice ? m_activeDevice->getCurrentStation() : undefinedRadioStation;
}


bool Radio::noticePowerOn (IRadioDevice *sender)
{
	setActiveDevice(sender, false);  // false: do not set power state on new device
	notifyPowerOn();
	return true;
}


bool Radio::noticePowerOff(IRadioDevice *sender)
{
	if (sender == m_activeDevice) {
		notifyPowerOff();
		return true;
	}
	return false;
}


bool Radio::noticeStationChanged (const RadioStation &rs, IRadioDevice *sender)
{
	if (sender == m_activeDevice) {
		notifyStationChanged(rs);
		return true;
	}
	return false;
}


void Radio::noticeConnect(IRadioDevice *)
{
	// hopefully nothing to do
}


void Radio::noticeDisconnect(IRadioDevice *rd)
{
	if (rd == m_activeDevice) {

		if (IRadioDeviceClient::connections.findRef(rd) >= 0) {

			IRadioDevice *new_rd = NULL;

			new_rd =  IRadioDeviceClient::connections.next();    // choose next device as active device if next exists
			if (!new_rd) {
				IRadioDeviceClient::connections.findRef(rd);				
				new_rd = IRadioDeviceClient::connections.prev(); // otherwise try prev then, may be NULL (no connections)
			}
			setActiveDevice(new_rd);
			
        } else {
			// strange error occurred, m_activeDevice not in connections... set to first.
			
			setActiveDevice(IRadioDeviceClient::connections.first());
        }
	}
}
