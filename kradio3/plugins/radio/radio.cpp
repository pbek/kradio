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

#include "../../src/radio-stations/radiostation.h"
#include "../../src/libkradio-gui/aboutwidget.h"
#include "../../src/interfaces/radiodevice_interfaces.h"
#include "radio.h"
#include "radio-configuration.h"

#include <kstandarddirs.h>
#include <kurl.h>
#include <kaboutdata.h>
#include <kconfig.h>


///////////////////////////////////////////////////////////////////////
//// plugin library functions

PLUGIN_LIBRARY_FUNCTIONS(Radio, "Central Radio Device Multiplexer");

/////////////////////////////////////////////////////////////////////////////

Radio::Radio(const QString &name)
  : PluginBase(name, i18n("Radio Multiplexer Plugin")),
    IRadioDeviceClient(-1),
    m_presetFile(locateLocal("data", "kradio/stations.krp")),
    m_activeDevice (NULL)
{
}


Radio::~Radio()
{
}


bool Radio::connectI    (Interface *i)
{
    bool a = IRadio::connectI(i);
    bool b = IRadioDeviceClient::connectI(i);
    bool c = IRadioDevicePool::connectI(i);
    bool d = PluginBase::connectI(i);
    bool e = ISoundStreamClient::connectI(i);

    // no "return IA::connectI() | return IB::connnectI to
    // prevent "early termination" optimization in boolean expressions
    return a || b || c || d || e;
}


bool Radio::disconnectI (Interface *i)
{
    bool a = IRadio::disconnectI(i);
    bool b = IRadioDeviceClient::disconnectI(i);
    bool c = IRadioDevicePool::disconnectI(i);
    bool d = PluginBase::disconnectI(i);
    bool e = ISoundStreamClient::disconnectI(i);

    // no "return IA::disconnectI() | return IB::disconnnectI to
    // prevent "early termination" optimization in boolean expressions
    return a || b || c || d || e;
}


void Radio::saveState (KConfig *config) const
{
    config->setGroup(QString("radio-") + name());

    config->writeEntry("presetfile", m_presetFile);

    m_stationList.writeXML(m_presetFile, *this);
}


void Radio::restoreState (KConfig *config)
{
    config->setGroup(QString("radio-") + name());

    m_presetFile = config->readEntry("presetfile",
                                     locateLocal("data", "kradio/stations.krp"));

    m_stationList.readXML(KURL(m_presetFile), *this);

    notifyStationsChanged(m_stationList);
    notifyPresetFileChanged(m_presetFile);
}



ConfigPageInfo Radio::createConfigurationPage()
{
    RadioConfiguration *conf = new RadioConfiguration (NULL, *this);
    connectI (conf);
    return ConfigPageInfo(
        conf,
        i18n("Radio Stations"),
        i18n("Setup Radio Stations"),
        "kradio"
    );
}


AboutPageInfo Radio::createAboutPage()
{
/*    KAboutData aboutData("kradio",
                         NULL,
                         NULL,
                         I18N_NOOP("Radio Device Multiplexer and Station Management for KRadio"),
                         KAboutData::License_GPL,
                         "(c) 2002-2005 Martin Witte, Klas Kalass",
                         0,
                         "http://sourceforge.net/projects/kradio",
                         0);
    aboutData.addAuthor("Martin Witte",  "", "witte@kawo1.rwth-aachen.de");
    aboutData.addAuthor("Klas Kalass",   "", "klas.kalass@gmx.de");

    return AboutPageInfo(
              new KRadioAboutWidget(aboutData, KRadioAboutWidget::AbtTabbed),
              i18n("Device and Station Management"),
              i18n("Radio Device Multiplexer and Station Management"),
              "kradio"
           );
*/
    return AboutPageInfo();
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

        for (IRadioDeviceClient::IFIterator it(IRadioDeviceClient::iConnections); it.current(); ++it) {

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
    notifyStationsChanged(m_stationList);
    return true;
}

bool Radio::setPresetFile(const QString &presetFile)
{
    m_presetFile = presetFile;
    notifyPresetFileChanged(m_presetFile);
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

SoundStreamID Radio::getCurrentSoundStreamID() const
{
    return queryCurrentSoundStreamID();
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

    if (!rd || IRadioDeviceClient::iConnections.containsRef(rd)) {     // new device is ok

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
        notifyCurrentSoundStreamIDChanged(queryCurrentSoundStreamID());
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
    return IRadioDeviceClient::iConnections;
}


const QString &Radio::getDeviceDescription() const
{
    return queryDescription();
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


static QString qstrUnknown(I18N_NOOP("unknown"));
static QString i18nqstrUnknown;
const QString &Radio::queryDescription() const
{
    return m_activeDevice ? m_activeDevice->getDescription() : (i18nqstrUnknown = i18n(qstrUnknown.ascii()));
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


bool Radio::noticeDescriptionChanged (const QString &s, const IRadioDevice *sender)
{
    if (sender == m_activeDevice)
        notifyDeviceDescriptionChanged(s);
    return true;
}


bool Radio::noticeCurrentSoundStreamIDChanged(SoundStreamID id, const IRadioDevice *sender)
{
    if (sender == m_activeDevice)
        notifyCurrentSoundStreamIDChanged(id);
    return true;
}


SoundStreamID Radio::queryCurrentSoundStreamID() const
{
    return m_activeDevice ? m_activeDevice->getCurrentSoundStreamID() : SoundStreamID::InvalidID;
}



void Radio::noticeConnectedI(IRadioDeviceClient::cmplInterface *dev, bool pointer_valid)
{
    IRadioDeviceClient::noticeConnectedI(dev, pointer_valid);

    if (! m_activeDevice && pointer_valid)
        setActiveDevice (dev, false);

    notifyDevicesChanged(IRadioDeviceClient::iConnections);
}


void Radio::noticeDisconnectI(IRadioDeviceClient::cmplInterface *rd, bool pointer_valid)
{
    IRadioDeviceClient::noticeDisconnectI(rd, pointer_valid);

    if (rd == m_activeDevice) {

        // search a new active device
        if (IRadioDeviceClient::iConnections.findRef(rd) >= 0) {

            IRadioDevice *new_rd = NULL;

            new_rd =  IRadioDeviceClient::iConnections.next();    // choose next device as active device if next exists
            if (!new_rd) {
                IRadioDeviceClient::iConnections.findRef(rd);
                new_rd = IRadioDeviceClient::iConnections.prev(); // otherwise try prev then, may be NULL (no connections)
            }
            setActiveDevice(new_rd);

        } else {
            // strange error occurred, m_activeDevice not in connections... set to first.

            setActiveDevice(IRadioDeviceClient::iConnections.first());
        }
    }
    notifyDevicesChanged(IRadioDeviceClient::iConnections);
}


// ITimeControlClient

bool Radio::noticeAlarm(const Alarm &a)
{
    if (a.alarmType() == Alarm::StartPlaying ||
        a.alarmType() == Alarm::StartRecording)
    {
        const RawStationList &sl = getStations().all();
        const RadioStation &rs = sl.stationWithID(a.stationID());
        activateStation(rs);
        powerOn();

        if (a.volumePreset() >= 0)
            sendPlaybackVolume(getCurrentSoundStreamID(), a.volumePreset());

        SoundStreamID id = getCurrentSoundStreamID();
        bool r = false;
        queryIsRecordingRunning(id, r);
        if (a.alarmType() == Alarm::StartRecording && !r)
            startRecording(id);

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

