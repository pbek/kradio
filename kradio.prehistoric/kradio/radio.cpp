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
#include "radio-configuration.h"

#include <kstandarddirs.h>
#include <kurl.h>

/////////////////////////////////////////////////////////////////////////////

Radio::Radio(const QString &name)
  : PluginBase(name),
    IRadioDeviceClient(-1),
    m_presetFile(locateLocal("data", "kradio/stations-new.krp")),
    m_activeDevice (NULL)
{
}


Radio::~Radio()
{
}


bool Radio::connect    (Interface *i)
{
	bool a = IRadio::connect(i);
	bool b = IRadioDeviceClient::connect(i);
	bool c = IRadioDevicePool::connect(i);
/*
    if (a) kdDebug() << "Radio: IRadio connected\n";
    if (b) kdDebug() << "Radio: IRadioDeviceClient connected\n";
    if (c) kdDebug() << "Radio: IRadioDevicePool connected\n";
*/
	// no "return IA::connect() | return IB::connnect to
	// prevent "early termination" optimization in boolean expressions
	return a || b || c;
}


bool Radio::disconnect (Interface *i)
{
	bool a = IRadio::disconnect(i);
	bool b = IRadioDeviceClient::disconnect(i);
	bool c = IRadioDevicePool::disconnect(i);
/*
    if (a) kdDebug() << "Radio: IRadio disconnected\n";
    if (b) kdDebug() << "Radio: IRadioDeviceClient disconnected\n";
    if (c) kdDebug() << "Radio: IRadioDevicePool disconnected\n";
*/
	// no "return IA::connect() | return IB::connnect to
	// prevent "early termination" optimization in boolean expressions
	return a || b || c;
}


void Radio::saveState (KConfig *config) const
{
    config->setGroup(QString("radio-") + name());

    config->writeEntry("presetfile", m_presetFile);

    m_stationList.writeXML(m_presetFile);
}


void Radio::restoreState (KConfig *config)
{
    config->setGroup(QString("radio-") + name());

    m_presetFile = config->readEntry("presetfile",
                                     locateLocal("data", "kradio/stations-new.krp"));

    m_stationList.readXML(KURL(m_presetFile));

    notifyStationsChanged(m_stationList);
}



ConfigPageInfo Radio::createConfigurationPage()
{
	RadioConfiguration *conf = new RadioConfiguration (NULL);
	connect (conf);
	return ConfigPageInfo(
		conf,
		"Radio Stations",
		"Setup Radio Stations",
		"kradio"
	);
}


QWidget *Radio::createAboutPage()
{
	// FIXME
	return NULL;
}






/* IRadio Interface Methods
*/

/* offer new station to current device.
   if that does not accept, try all other devices.
   Any device will be powered off if it does not accept the station
*/

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
	return true;
}


int Radio::getStationIdx(const RadioStation &rs) const
{
	RawStationList &sl = const_cast<RawStationList&>(m_stationList.all());
	return sl.find(&rs);
}

int Radio::getCurrentStationIdx() const
{
	return getStationIdx(getCurrentStation());
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

		// send notifications
	    notifyActiveDeviceChanged(m_activeDevice);
	    const RadioStation &rs = queryCurrentStation();
	    notifyStationChanged(rs, getStationIdx(rs));

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
	if (m_activeDevice) {
		RadioStation &rs = const_cast<RadioStation&>(m_activeDevice->getCurrentStation());
		int idx = getStationIdx(rs);

		if (idx >= 0) {
			rs.copyDescriptionFrom(m_stationList.at(idx));
		} else {
			rs.copyDescriptionFrom(undefinedRadioStation);
		}

		return rs;
	} else {
		return undefinedRadioStation;
	}
}


bool Radio::noticePowerChanged (bool on, const IRadioDevice *sender)
{
	if (on) {
		setActiveDevice(const_cast<IRadioDevice*>(sender), false);
	                    // false: do not set power state on new device
	                    // constcast valid because power-state of sender is not changed
		notifyPowerChanged(true);
	    return true;
	    
	} else {
		if (sender == m_activeDevice) {
			notifyPowerChanged(false);
			return true;
		}
		return false;
	}
}


bool Radio::noticeStationChanged (const RadioStation &_rs, const IRadioDevice *sender)
{
	RadioStation &rs = const_cast<RadioStation&>(_rs);
	int idx = getStationIdx(rs);

	RadioStation &known = (idx >= 0) ? (RadioStation&)m_stationList.at(idx) :
	                                   (RadioStation&)undefinedRadioStation;
	rs.copyDescriptionFrom(known);

	if (sender == m_activeDevice)
		notifyStationChanged(rs, idx);
	return true;
}


void Radio::noticeConnected(IRadioDeviceClient::cmplInterface *dev, bool pointer_valid)
{
	IRadioDeviceClient::noticeConnected(dev, pointer_valid);
	
	if (! m_activeDevice && pointer_valid)
		setActiveDevice (dev, false);

	notifyDevicesChanged(IRadioDeviceClient::connections);
}


void Radio::noticeDisconnect(IRadioDeviceClient::cmplInterface *rd, bool pointer_valid)
{
	IRadioDeviceClient::noticeDisconnected(rd, pointer_valid);
	
	if (rd == m_activeDevice) {

	    // search a new active device
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
	notifyDevicesChanged(IRadioDeviceClient::connections);
}


// ITimeControlClient

bool Radio::noticeAlarm(const Alarm &a)
{
	if (a.alarmType() == Alarm::StartPlaying ||
	    a.alarmType() == Alarm::StartRecording) {
		const RawStationList &sl = getStations().all();
		const RadioStation &rs = sl.stationWithID(a.stationID());
		activateStation(rs);
		powerOn();
	} else {
		powerOff();
	}
	return true;
}


bool Radio::noticeCountdownZero()
{
	powerOff();
	return true;
}
