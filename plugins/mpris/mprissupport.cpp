/***************************************************************************
                          mprissupport.cpp  -  description
                             -------------------
    copyright            : (C) 2014 by Pino Toscano
    email                : toscano.pino@tiscali.it
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
#include <KAboutData>

#include <unistd.h>

#include "mprissupport.h"
#include "mprisroot.h"
#include "mprisplayer.h"

#include "errorlog_interfaces.h"
#include "radiodevice_interfaces.h"
#include "seekradio_interfaces.h"
#include "stationlist.h"
#include "pluginmanager.h"
#include "frequencyradiostation.h"
#include "internetradiostation.h"

///////////////////////////////////////////////////////////////////////
//// plugin library functions

static KAboutData aboutData()
{
    KAboutData about("MPRISSupport",
                     i18nc("@title", "MPRIS"),
                     KRADIO_VERSION,
                     i18nc("@title", "MPRIS Support"),
                     KAboutLicense::LicenseKey::GPL,
                     i18nc("@info:credit", "(c) 2014 Pino Toscano"),
                     NULL,
                     "http://sourceforge.net/projects/kradio",
                     "emw-kradio@nocabal.de");
    about.addAuthor(i18nc("@info:credit", "Pino Toscano"), NULL, "toscano.pino@tiscali.it");
    return about;
}

KRADIO_EXPORT_PLUGIN(MPRISSupport, aboutData())
#include "mprissupport.moc"


/////////////////////////////////////////////////////////////////////////////

MPRISSupport::MPRISSupport(const QString &instanceID, const QString &name)
    : PluginBase(instanceID, name, i18n("MPRIS Plugin"))
{
}


MPRISSupport::~MPRISSupport()
{
}


void MPRISSupport::startPlugin()
{
    if (m_manager) {
        QDBusConnection dbus = QDBusConnection::sessionBus();

        QString mpris2Name = QString::fromLatin1("org.mpris.MediaPlayer2.%1.instance%2")
                             .arg(KAboutData::applicationData().componentName())
                             .arg(getpid());

        bool success = dbus.registerService(mpris2Name);
        if (success) {
            QDBusAbstractAdaptor *rootAdaptor = new MPRISRoot(this);
            Q_UNUSED(rootAdaptor);
            MPRISPlayer *playerAdaptor = new MPRISPlayer(this);
            connect(this, &MPRISSupport::powerChanged,           playerAdaptor, &MPRISPlayer::slotPowerChanged);
            connect(this, &MPRISSupport::RDSStateChanged,        playerAdaptor, &MPRISPlayer::slotRDSStateChanged);
            connect(this, &MPRISSupport::RDSRadioTextChanged,    playerAdaptor, &MPRISPlayer::slotRDSRadioTextChanged);
            connect(this, &MPRISSupport::RDSStationNameChanged,  playerAdaptor, &MPRISPlayer::slotRDSStationNameChanged);
            connect(this, &MPRISSupport::volumeChanged,          playerAdaptor, &MPRISPlayer::slotVolumeChanged);
            connect(this, &MPRISSupport::currentStreamChanged,   playerAdaptor, &MPRISPlayer::slotCurrentStreamChanged);
            dbus.registerObject("/org/mpris/MediaPlayer2", this);
        } else {
            logError("MPRISSupport: cannot register: " + mpris2Name);
        }
    } else {
        logError("Internal Error: MPRISSupport::startPlugin: Has NULL m_manager pointer. MPRIS not registered.");
    }
}

bool MPRISSupport::connectI (Interface *i)
{
    bool a = IRadioClient::connectI (i);
    bool b = PluginBase::connectI(i);
    bool c = ISoundStreamClient::connectI(i);
    return a || b || c;
}


bool MPRISSupport::disconnectI (Interface *i)
{
    bool a = IRadioClient::disconnectI (i);
    bool b = PluginBase::disconnectI(i);
    bool c = ISoundStreamClient::disconnectI(i);
    return a || b || c;
}



void MPRISSupport::saveState (KConfigGroup &c) const
{
    PluginBase::saveState(c);
}

void MPRISSupport::restoreState (const KConfigGroup &c)
{
    PluginBase::restoreState(c);
}


bool MPRISSupport::noticePowerChanged(bool on)
{
    emit powerChanged(on);
    return false;
}

bool MPRISSupport::noticeStationChanged(const RadioStation &, int)
{
    return false;
}

bool MPRISSupport::noticeStationsChanged(const StationList &)
{
    return false;
}

bool MPRISSupport::noticePresetFileChanged(const QUrl &)
{
    return false;
}

bool MPRISSupport::noticeRDSStateChanged(bool enabled)
{
    emit RDSStateChanged(enabled);
    return false;
}

bool MPRISSupport::noticeRDSRadioTextChanged(const QString &s)
{
    emit RDSRadioTextChanged(s);
    return false;
}

bool MPRISSupport::noticeRDSStationNameChanged(const QString &s)
{
    emit RDSStationNameChanged(s);
    return false;
}

bool MPRISSupport::noticeCurrentSoundStreamSourceIDChanged(SoundStreamID)
{
    return false;
}

bool MPRISSupport::noticeCurrentSoundStreamSinkIDChanged(SoundStreamID)
{
    return false;
}

void MPRISSupport::noticeConnectedI(ISoundStreamServer *s, bool pointer_valid)
{
    ISoundStreamClient::noticeConnectedI(s, pointer_valid);
    if (s && pointer_valid) {
        s->register4_notifyPlaybackVolumeChanged(this);
        s->register4_notifySoundStreamChanged(this);
    }
}

bool MPRISSupport::noticePlaybackVolumeChanged(SoundStreamID id, float volume)
{
    if (queryCurrentSoundStreamSinkID() == id) {
        emit volumeChanged(volume);
    }
    return false;
}

bool MPRISSupport::noticeSoundStreamChanged(SoundStreamID)
{
    emit currentStreamChanged();
    return false;
}


void MPRISSupport::showAllWidgetPlugins()
{
    if (m_manager) {
        m_manager->slotRestoreAllWidgetPlugins();
    }
}

bool MPRISSupport::isPlaying() const
{
    return queryIsPowerOn();
}

bool MPRISSupport::isPaused() const
{
    SoundStreamID streamSinkID = queryCurrentSoundStreamSinkID();
    bool          p            = false;
    queryIsPlaybackPaused(streamSinkID, p);
    return p;
}

double MPRISSupport::volume() const
{
    SoundStreamID streamSinkID = queryCurrentSoundStreamSinkID();
    float         v            = 0;
    queryPlaybackVolume(streamSinkID, v);
    return v;
}

void MPRISSupport::setVolume(double vol)
{
    if (queryIsPowerOn()) {
        SoundStreamID streamSinkID = queryCurrentSoundStreamSinkID();
        sendPlaybackVolume(streamSinkID, vol);
    }
}

static QString makeTrackId(const RadioStation &rs)
{
  return QString::fromLatin1("/net/sourceforge/") + KAboutData::applicationData().componentName() + "/id-" + rs.stationID();
}

QVariantMap MPRISSupport::mprisMetadata() const
{
    QVariantMap metaData;

    if (!isPlaying()) {
        return metaData;
    }

    const int idx = queryCurrentStationIdx();
    const StationList &sl = queryStations();
    if (idx < 0 || idx >= sl.count()) {
        return metaData;
    }

    const RadioStation &rs = sl.at(idx);

    metaData["mpris:trackid"] = makeTrackId(rs);
    const QString stationName = rs.name();
    metaData["xesam:title"] = stationName;
    metaData["xesam:artist"] = QStringList() << stationName;

    const QString className = rs.getClassName();
    if (className == QLatin1String("InternetRadioStation")) {
        const InternetRadioStation *ir = static_cast<const InternetRadioStation *>(&rs);
        metaData["xesam:url"] = ir->url().toDisplayString();
    } else if (className == QLatin1String("FrequencyRadioStation")) {
        const FrequencyRadioStation *fr = static_cast<const FrequencyRadioStation *>(&rs);
        metaData["kradio:frequency"] = fr->frequency();
    }

    if (queryRDSState()) {
        metaData["xesam:title"] = metaData["kradio:rds:text"] = queryRDSRadioText();
        metaData["kradio:rds:stationName"] = queryRDSStationName();
    }

    return metaData;
}

void MPRISSupport::stop()
{
    if (queryIsPowerOn()) {
        sendPowerOff();
    }
}

void MPRISSupport::play()
{
    if (queryIsPowerOn()) {
        SoundStreamID streamSinkID = queryCurrentSoundStreamSinkID();
        bool          paused       = false;
        queryIsPlaybackPaused(streamSinkID, paused);
        if (paused) {
            sendResumePlayback(streamSinkID);
        }
    } else {
        sendPowerOn();
    }
}

void MPRISSupport::pause()
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

void MPRISSupport::playPause()
{
    if (queryIsPowerOn()) {
        SoundStreamID streamSinkID = queryCurrentSoundStreamSinkID();
        bool          paused       = false;
        queryIsPlaybackPaused(streamSinkID, paused);
        if (paused) {
            sendResumePlayback(streamSinkID);
        } else {
            sendPausePlayback(streamSinkID);
        }
    } else {
        sendPowerOn();
    }
}

