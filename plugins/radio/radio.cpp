/***************************************************************************
                          radio.cpp  -  description
                             -------------------
    begin                : Sat March 29 2003
    copyright            : (C) 2003 by Klas Kalass, Ernst Martin Witte
    email                : klas@kde.org, emw-kradio@nocabal.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "radiostation.h"
// #warning "FIXME: port aboutwidgets"
//#include "aboutwidget.h"
#include "radiodevice_interfaces.h"
#include "radio.h"
#include "radio-configuration.h"

#include <kstandarddirs.h>
#include <kurl.h>
#include <kaboutdata.h>
#include <kconfig.h>

#include "debug-profiler.h"

///////////////////////////////////////////////////////////////////////
//// plugin library functions

PLUGIN_LIBRARY_FUNCTIONS(Radio, PROJECT_NAME, i18n("Central Radio Device Multiplexer"));

/////////////////////////////////////////////////////////////////////////////

Radio::Radio(const QString &instanceID, const QString &name)
  : PluginBase(instanceID, name, i18n("Radio Multiplexer Plugin")),
    IRadioDeviceClient(-1),
    m_presetFile(KStandardDirs::locateLocal("data", "kradio4/stations.krp")),
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


void Radio::saveState (KConfigGroup &config) const
{
    PluginBase::saveState(config);

    config.writeEntry("presetfile", m_presetFile);
    m_stationList.writeXML(m_presetFile, *this);

    if (m_activeDevice) {
        config.writeEntry("active_device", m_activeDevice->getRadioDeviceID());
    }
}


void Radio::restoreState (const KConfigGroup &config)
{
    PluginBase::restoreState(config);

    m_presetFile = config.readEntry("presetfile", QString());

    bool first_restore = false;
    if (m_presetFile.isNull() || m_presetFile.length() == 0) {
        m_presetFile = KStandardDirs::locateLocal("data", "kradio4/stations.krp");
        first_restore = true;
    }

    m_stationList.readXML(KUrl(m_presetFile), *this, /*enable-messagebox*/ !first_restore);

    notifyStationsChanged(m_stationList);
    notifyPresetFileChanged(m_presetFile);

    m_startup_LastActiveDeviceID = config.readEntry("active_device", "");

}


void Radio::startPlugin()
{
    for (IRadioDeviceClient::IFIterator it = IRadioDeviceClient::iConnections.begin(); it != IRadioDeviceClient::iConnections.end(); ++it) {

        if ((*it)->getRadioDeviceID() == m_startup_LastActiveDeviceID) {
            setActiveDevice((*it));
        }
    }
}


ConfigPageInfo Radio::createConfigurationPage()
{
    RadioConfiguration *conf = new RadioConfiguration (NULL, *this);
    connectI (conf);
    return ConfigPageInfo(
        conf,
        i18n("Radio Stations"),
        i18n("Setup Radio Stations"),
        "kradio4"
    );
}


/*AboutPageInfo Radio::createAboutPage()
{*/
/*    KAboutData aboutData("kradio4",
                         NULL,
                         NULL,
                         I18N_NOOP("Radio Device Multiplexer and Station Management for KRadio"),
                         KAboutData::License_GPL,
                         "(c) 2002-2005 Martin Witte, Klas Kalass",
                         0,
                         "http://sourceforge.net/projects/kradio",
                         0);
    aboutData.addAuthor("Martin Witte",  "", "emw-kradio@nocabal.de");
    aboutData.addAuthor("Klas Kalass",   "", "klas.kalass@gmx.de");

    return AboutPageInfo(
              new KRadioAboutWidget(aboutData, KRadioAboutWidget::AbtTabbed),
              i18n("Device and Station Management"),
              i18n("Radio Device Multiplexer and Station Management"),
              "kradio4"
           );
*/
// /*    return AboutPageInfo();
// }*/






/* IRadio Interface Methods
*/

/* offer new station to current device.
   if that does not accept, try all other devices.
   Any device will be powered off if it does not accept the station
*/

bool Radio::activateStation (const RadioStation &rs)
{


    if (sendActivateStation(rs)) {    // first try activeDevice

        return true;

    } else {                          // hmm... active device did not want it. Try others...

        int n = 0;

        for (IRadioDeviceClient::IFIterator it = IRadioDeviceClient::iConnections.begin(); it != IRadioDeviceClient::iConnections.end(); ++it) {

            if ((*it)->activateStation(rs)) {

                setActiveDevice((*it));  // select new device
                ++n;

//             } else {
//
//                 (*it)->powerOff();

            }
        }

        if (n <= 0) {
            logError(i18n("Could not find a plugin for radio station %1", rs.longName()));
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
    if (true/*m_stationList != sl*/) {
        BlockProfiler("Radio::setStations");
        m_stationList = sl;
        notifyStationsChanged(m_stationList);
    }
    return true;
}

bool Radio::setPresetFile(const QString &presetFile)
{
    if (m_presetFile != presetFile) {
        m_presetFile = presetFile;
        notifyPresetFileChanged(m_presetFile);
    }
    return true;
}

int Radio::getStationIdx(const RadioStation &rs) const
{
    return m_stationList.idxWithID(rs.stationID());
}

int Radio::getCurrentStationIdx() const
{
    return getStationIdx(getCurrentStation());
}


bool Radio::getRDSState () const
{
    return queryRDSState();
}

const QString & Radio::getRDSRadioText() const
{
    return queryRDSRadioText();
}

const QString & Radio::getRDSStationName() const
{
    return queryRDSStationName();
}


SoundStreamID Radio::getCurrentSoundStreamSourceID() const
{
    return queryCurrentSoundStreamSourceID();
}

SoundStreamID Radio::getCurrentSoundStreamSinkID() const
{
    return queryCurrentSoundStreamSinkID();
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

    if (!rd || IRadioDeviceClient::iConnections.contains(rd)) {     // new device is ok

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
        notifyCurrentSoundStreamSourceIDChanged(queryCurrentSoundStreamSourceID());
        notifyCurrentSoundStreamSinkIDChanged  (queryCurrentSoundStreamSinkID  ());
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


const QList<IRadioDevice*> &Radio::getDevices() const
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


static QString i18nqstrUnknown;
static QString emptyString;
const QString &Radio::queryDescription() const
{
    return m_activeDevice ? m_activeDevice->getDescription() : (i18nqstrUnknown = i18n("unknown"));
}

bool Radio::queryRDSState () const
{
    return m_activeDevice ? m_activeDevice->getRDSState() : false;
}

const QString &Radio::queryRDSRadioText() const
{
    return m_activeDevice ? m_activeDevice->getRDSRadioText()   : emptyString;
}

const QString &Radio::queryRDSStationName() const
{
    return m_activeDevice ? m_activeDevice->getRDSStationName() : emptyString;
}

SoundStreamID Radio::queryCurrentSoundStreamSourceID() const
{
    return m_activeDevice ? m_activeDevice->getCurrentSoundStreamSourceID() : SoundStreamID::InvalidID;
}

SoundStreamID Radio::queryCurrentSoundStreamSinkID() const
{
    return m_activeDevice ? m_activeDevice->getCurrentSoundStreamSinkID() : SoundStreamID::InvalidID;
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
            sendStopCountdown();
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
    if (sender == m_activeDevice) {
        notifyDeviceDescriptionChanged(s);
    }
    if (IRadioDeviceClient::iConnections.contains(const_cast<IRadioDevice*>(sender))) {
        notifyDevicesChanged(IRadioDeviceClient::iConnections);
    }
    return true;
}


bool Radio::noticeCurrentSoundStreamSourceIDChanged(SoundStreamID id, const IRadioDevice *sender)
{
    if (sender == m_activeDevice)
        notifyCurrentSoundStreamSourceIDChanged(id);
    return true;
}


bool Radio::noticeCurrentSoundStreamSinkIDChanged(SoundStreamID id, const IRadioDevice *sender)
{
    if (sender == m_activeDevice)
        notifyCurrentSoundStreamSinkIDChanged(id);
    return true;
}


bool Radio::noticeRDSStateChanged (bool enabled, const IRadioDevice *sender)
{
    if (sender == m_activeDevice)
        notifyRDSStateChanged(enabled);
    return true;
}

bool Radio::noticeRDSRadioTextChanged (const QString &s, const IRadioDevice *sender)
{
    if (sender == m_activeDevice)
        notifyRDSRadioTextChanged(s);
    return true;
}

bool Radio::noticeRDSStationNameChanged(const QString &s, const IRadioDevice *sender)
{
    if (sender == m_activeDevice)
        notifyRDSStationNameChanged(s);
    return true;
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
        int idx = IRadioDeviceClient::iConnections.indexOf(rd);
        if (idx >= 0) {

            IRadioDevice *new_rd = NULL;

            if (idx + 1 < IRadioDeviceClient::iConnections.size()) { // choose next device as active device if next exists
                new_rd =  IRadioDeviceClient::iConnections[idx+1];
            } else if (idx > 0) {
                new_rd = IRadioDeviceClient::iConnections[idx-1];    // otherwise try prev then, may be NULL (no connections)
            }
            setActiveDevice(new_rd);

        } else {
            // strange error occurred, m_activeDevice not in connections... set to first.

            setActiveDevice(IRadioDeviceClient::iConnections.first());
        }
    }
}


void Radio::noticeDisconnectedI(IRadioDeviceClient::cmplInterface *rd, bool pointer_valid)
{
    IRadioDeviceClient::noticeDisconnectedI(rd, pointer_valid);
    notifyDevicesChanged(IRadioDeviceClient::iConnections);
}

// ITimeControlClient

bool Radio::noticeAlarm(const Alarm &a)
{
    if (a.alarmType() == Alarm::StartPlaying ||
        a.alarmType() == Alarm::StartRecording)
    {
        QString rsID = a.stationID();
        if (rsID.length()) {
            const RadioStation &rs = getStations().stationWithID(rsID);
            activateStation(rs);
        }
        powerOn();

        SoundStreamID dst_id = getCurrentSoundStreamSinkID();
        if (a.volumePreset() >= 0)
            sendPlaybackVolume(dst_id, a.volumePreset());

        bool r = false;
        SoundFormat sf;
        // we are using sink for recording, in order to get also effect plugins outputs
        queryIsRecordingRunning(dst_id, r, sf);
        if (a.alarmType() == Alarm::StartRecording && !r)
            sendStartRecording(dst_id, a.recordingTemplate());

    }
    else if (a.alarmType() == Alarm::StopRecording) {
        SoundStreamID dst_id = getCurrentSoundStreamSinkID();
        sendStopRecording(dst_id);
    }
    else if (a.alarmType() == Alarm::StopPlaying) {
        powerOff();
    }
    return true;
}


bool Radio::noticeCountdownZero()
{
    powerOff();
    return true;
}


void Radio::aboutToQuit()
{
    sendPowerOff();
}

