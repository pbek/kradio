/***************************************************************************
                          dbussupport.cpp  -  description
                             -------------------
    begin                : Tue Mar 3 2009
    copyright            : (C) 2009 by Martin Witte
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
#include <QDBusConnection>
#include <QApplication>

#include "dbussupport.h"

#include "errorlog_interfaces.h"
#include "radiodevice_interfaces.h"
#include "seekradio_interfaces.h"
#include "stationlist.h"
#include "pluginmanager.h"

#include "kradioadaptor.h"

///////////////////////////////////////////////////////////////////////
//// plugin library functions

static KAboutData aboutData()
{
    KAboutData about(QStringLiteral("DBusSupport"),
                     i18nc("@title", "D-Bus"),
                     QStringLiteral(KRADIO_VERSION),
                     i18nc("@title", "D-Bus Support"),
                     KAboutLicense::LicenseKey::GPL,
                     NULL,
		     NULL,
                     "http://sourceforge.net/projects/kradio",
                     "emw-kradio@nocabal.de");
    return about;
}

KRADIO_EXPORT_PLUGIN(DBusSupport, aboutData())
#include "dbussupport.moc"


/////////////////////////////////////////////////////////////////////////////

DBusSupport::DBusSupport(const QString &instanceID, const QString &name)
    : PluginBase(instanceID, name, i18n("D-Bus Plugin"))
{
}


DBusSupport::~DBusSupport()
{
}


void DBusSupport::startPlugin()
{
    if (m_manager) {
        new KradioAdaptor(this);
        QDBusConnection dbus = QDBusConnection::sessionBus();
        QString object_path = QString("/KRadio4/%1").arg(m_manager->instanceName());
        object_path.replace(" ", "");
        logDebug("DBus Object Path: " + object_path);
        dbus.registerObject(object_path, this);
    } else {
        logError("Internal Error: DBusSupport::startPlugin: Has NULL m_manager pointer. DBus not registered.");
    }
}

bool DBusSupport::connectI (Interface *i)
{
    bool a = IRadioClient::connectI (i);
    bool b = ITimeControlClient::connectI (i);
    bool c = IRadioDevicePoolClient::connectI (i);
    bool d = PluginBase::connectI(i);
    bool e = ISoundStreamClient::connectI(i);
    return a || b || c || d || e;
}


bool DBusSupport::disconnectI (Interface *i)
{
    bool a = IRadioClient::disconnectI (i);
    bool b = ITimeControlClient::disconnectI (i);
    bool c = IRadioDevicePoolClient::disconnectI (i);
    bool d = PluginBase::disconnectI(i);
    bool e = ISoundStreamClient::disconnectI(i);
    return a || b || c || d || e;
}



void   DBusSupport::saveState (KConfigGroup &c) const
{
    PluginBase::saveState(c);
}

void   DBusSupport::restoreState (const KConfigGroup &c)
{
    PluginBase::restoreState(c);
}


// DBus support signals/slots


void DBusSupport::powerOn()
{
    if (!queryIsPowerOn()) {
        sendPowerOn();
    }
}

void DBusSupport::powerOff()
{
    if (queryIsPowerOn()) {
        sendPowerOff();
    }
}

void DBusSupport::recordingStart()
{
    SoundStreamID streamSinkID = queryCurrentSoundStreamSinkID();
    bool          rec          = false;
    SoundFormat   sf;
    queryIsRecordingRunning(streamSinkID, rec, sf);
    if (!rec) {
        sendStartRecording(streamSinkID);
    }
}

void DBusSupport::recordingStop()
{
    SoundStreamID streamSinkID = queryCurrentSoundStreamSinkID();
    bool          rec          = false;
    SoundFormat   sf;
    queryIsRecordingRunning(streamSinkID, rec, sf);
    if (rec) {
        sendStopRecording(streamSinkID);
    }
}

void DBusSupport::playbackPause()
{
    if (queryIsPowerOn()) {
        SoundStreamID streamSinkID = queryCurrentSoundStreamSinkID();
        bool          paused       = false;
        queryIsPlaybackPaused(streamSinkID, paused);
        if (!paused) {
            sendPausePlayback(streamSinkID);
        }
    }
}

void DBusSupport::playbackResume()
{
    if (queryIsPowerOn()) {
        SoundStreamID streamSinkID = queryCurrentSoundStreamSinkID();
        bool          paused       = false;
        queryIsPlaybackPaused(streamSinkID, paused);
        if (paused) {
            sendResumePlayback(streamSinkID);
        }
    }
}


void DBusSupport::setVolume(float v)
{
    if (queryIsPowerOn()) {
        SoundStreamID streamSinkID = queryCurrentSoundStreamSinkID();
        sendPlaybackVolume (streamSinkID, v);
    }
}

void DBusSupport::increaseVolume()
{
    if (queryIsPowerOn()) {
        SoundStreamID streamSinkID = queryCurrentSoundStreamSinkID();
        float         oldVolume    = 0;
        queryPlaybackVolume(streamSinkID, oldVolume);
        sendPlaybackVolume (streamSinkID, oldVolume + 1.0/32.0);
    }
}

void DBusSupport::decreaseVolume()
{
    if (queryIsPowerOn()) {
        SoundStreamID streamSinkID = queryCurrentSoundStreamSinkID();
        float         oldVolume    = 0;
        queryPlaybackVolume(streamSinkID, oldVolume);
        sendPlaybackVolume (streamSinkID, oldVolume - 1.0/32.0);
    }
}


void DBusSupport::nextStation()
{
//     if (queryIsPowerOn()) {
        int k = queryCurrentStationIdx() + 1;
        if (k >= queryStations().count())
            k = 0;
        sendActivateStation(k);
//     }
}

void DBusSupport::prevStation()
{
//     if (queryIsPowerOn()) {
        int k = queryCurrentStationIdx() - 1;
        if (k < 0)
            k = queryStations().count() - 1;
        sendActivateStation(k);
//     }
}

void DBusSupport::setStation(int idx)
{
//     if (queryIsPowerOn()) {
        sendActivateStation(idx);
//     }
}

void DBusSupport::setStation(const QString &stationid)
{
    const StationList  &l   = queryStations();
    int                 idx = l.idxWithID(stationid);
    if (idx >= 0) {
        sendActivateStation(idx);
    }
}

void DBusSupport::searchNextStation()
{
    if (queryIsPowerOn()) {
        ISeekRadio *seeker = dynamic_cast<ISeekRadio*> (queryActiveDevice());
        if (seeker) {
            seeker->startSeekUp();
        }
    }
}

void DBusSupport::searchPrevStation()
{
    if (queryIsPowerOn()) {
        ISeekRadio *seeker = dynamic_cast<ISeekRadio*> (queryActiveDevice());
        if (seeker) {
            seeker->startSeekDown();
        }
    }
}


void DBusSupport::startSleepCountdown(int seconds, bool suspendOnSleep)
{
    if (queryIsPowerOn()) {
        sendCountdownSeconds(seconds, suspendOnSleep);
        sendStartCountdown();
    }
}

void DBusSupport::stopSleepCountdown()
{
    QDateTime dt = queryCountdownEnd();
    if (queryIsPowerOn() && dt.isValid()) {
        sendStopCountdown();
    }
}


void DBusSupport::showAllWidgets()
{
    if (m_manager) {
        m_manager->slotShowAllWidgetPlugins();
    }
}

void DBusSupport::hideAllWidgets()
{
    if (m_manager) {
        m_manager->slotHideAllWidgetPlugins();
    }
}

void DBusSupport::restoreAllWidgets()
{
    if (m_manager) {
        m_manager->slotRestoreAllWidgetPlugins();
    }
}

void DBusSupport::quitKRadio()
{
    qApp->quit();
}


bool DBusSupport::isPowerOn()   const
{
    return queryIsPowerOn();
}

bool DBusSupport::isPaused()    const
{
    SoundStreamID streamSinkID = queryCurrentSoundStreamSinkID();
    bool          p            = false;
    queryIsPlaybackPaused(streamSinkID, p);
    return p;
}

bool DBusSupport::isRecording() const
{
    SoundStreamID streamSinkID = queryCurrentSoundStreamSinkID();
    bool          rec          = false;
    SoundFormat   sf;
    queryIsRecordingRunning(streamSinkID, rec, sf);
    return rec;
}

bool DBusSupport::isSleepCountdownRunning() const
{
    QDateTime dt = queryCountdownEnd();
    return dt.isValid();
}

time_t DBusSupport::getSleepCountdownEnd() const
{
    return queryCountdownEnd().toTime_t();
}


float DBusSupport::getVolume()   const
{
    SoundStreamID streamSinkID = queryCurrentSoundStreamSinkID();
    float         v            = 0;
    queryPlaybackVolume(streamSinkID, v);
    return v;
}


int DBusSupport::getCurrentStationIndex() const
{
    return queryCurrentStationIdx();
}

QString DBusSupport::getStationName(int idx) const
{
    const StationList &sl = queryStations();
    if (idx >= 0 && idx < sl.count()) {
        const RadioStation &rs = sl.at(idx);
        return rs.name();
    }
    return QString();
}

QString DBusSupport::getStationShortName(int idx) const
{
    const StationList &sl = queryStations();
    if (idx >= 0 && idx < sl.count()) {
        const RadioStation &rs = sl.at(idx);
        return rs.shortName();
    }
    return QString();
}

QString DBusSupport::getStationLongName(int idx) const
{
    const StationList &sl = queryStations();
    if (idx >= 0 && idx < sl.count()) {
        const RadioStation &rs = sl.at(idx);
        return rs.longName();
    }
    return QString();
}

QString DBusSupport::getStationDescription(int idx) const
{
    const StationList &sl = queryStations();
    if (idx >= 0 && idx < sl.count()) {
        const RadioStation &rs = sl.at(idx);
        return rs.description();
    }
    return QString();
}


int DBusSupport::getStationsCount() const
{
    return queryStations().count();
}



