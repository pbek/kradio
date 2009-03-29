/***************************************************************************
                          internetradio.cpp  -  description
                             -------------------
    begin                : Mon Feb 23 CET 2009
    copyright            : (C) 2009 by Ernst Martin Witte
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QtCore/QList>

#include <kconfiggroup.h>
#include <kiconloader.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kio/jobclasses.h>
#include <kio/job.h>

#include "stationlist.h"
#include "utils.h"
#include "internetradio.h"
#include "soundstream_decoding_step_event.h"
#include "decoder_thread.h"

#include "internetradio-configuration.h"


#warning "FIXME: make buffer size configurable"
#define MAX_BUFFERS           20
#define MIN_BUFFERS4PLAYBACK  10

///////////////////////////////////////////////////////////////////////

PLUGIN_LIBRARY_FUNCTIONS(InternetRadio, PROJECT_NAME, i18n("Pseudo radio device for internet radio stream support"));

///////////////////////////////////////////////////////////////////////



InternetRadio::InternetRadio(const QString &instanceID, const QString &name)
  : PluginBase(instanceID, name, i18n("Internet Radio Plugin")),
    m_decoderThread(NULL),
    m_currentStation(InternetRadioStation()),
    m_stereoFlag(false),
    m_muted(false),
    m_defaultPlaybackVolume(0.5),
    m_PlaybackMixerID(QString()),
    m_PlaybackMixerChannel(QString()),
    m_restorePowerOn(false),
    m_RDS_visible(false),
    m_RDS_StationName(QString()),
    m_RDS_RadioText(QString()),
    m_waitForBufferMinFill(true)/*,
    m_MimetypeJob(NULL)*/
{
    m_SoundStreamSinkID   = createNewSoundStream(false);
    m_SoundStreamSourceID = m_SoundStreamSinkID;
}


InternetRadio::~InternetRadio()
{
    setPower(false);
}


bool InternetRadio::connectI (Interface *i)
{
    bool a = IRadioDevice      ::connectI(i);
    bool b = IRadioClient      ::connectI(i);
    bool c = IInternetRadio    ::connectI(i);
    bool d = PluginBase        ::connectI(i);
    bool e = ISoundStreamClient::connectI(i);
    return a || b || c || d || e;
}


bool InternetRadio::disconnectI (Interface *i)
{
    bool a = IRadioDevice      ::disconnectI(i);
    bool b = IRadioClient      ::disconnectI(i);
    bool c = IInternetRadio    ::disconnectI(i);
    bool d = PluginBase        ::disconnectI(i);
    bool e = ISoundStreamClient::disconnectI(i);
    return a || b || c || d || e;
}


void InternetRadio::noticeConnectedI (ISoundStreamServer *s, bool pointer_valid)
{
    ISoundStreamClient::noticeConnectedI(s, pointer_valid);
    if (s && pointer_valid) {

        s->register4_notifyPlaybackChannelsChanged(this);
        s->register4_sendStartCaptureWithFormat(this);

        s->register4_queryPlaybackVolume(this);

        s->register4_sendMuteSource(this);
        s->register4_sendUnmuteSource(this);

        s->register4_querySignalQuality(this);
        s->register4_queryHasGoodQuality(this);
        s->register4_queryIsStereo(this);
        s->register4_queryIsSourceMuted(this);

        s->register4_sendPlaybackVolume(this);

        s->register4_querySoundStreamDescription(this);
        s->register4_querySoundStreamRadioStation(this);
        s->register4_queryEnumerateSourceSoundStreams(this);

        s->register4_notifyReadyForPlaybackData(this);
        s->register4_notifySoundStreamClosed(this);
        s->register4_notifySoundStreamSinkRedirected(this);
        s->register4_notifySoundStreamSourceRedirected(this);

        notifySoundStreamCreated(m_SoundStreamSinkID);
        if (m_SoundStreamSinkID != m_SoundStreamSourceID) {
            notifySoundStreamCreated(m_SoundStreamSourceID);
        }
    }
}

void InternetRadio::noticeConnectedSoundClient(ISoundStreamClient::thisInterface *i, bool pointer_valid)
{
    if (i && pointer_valid && i->getSoundStreamClientID() == m_PlaybackMixerID) {
        setPlaybackMixer(m_PlaybackMixerID, m_PlaybackMixerChannel, /* force = */ true);
    }
}


bool InternetRadio::noticePlaybackChannelsChanged(const QString & client_id, const QStringList &/*channels*/)
{
    if (client_id == m_PlaybackMixerID) {
        setPlaybackMixer(m_PlaybackMixerID, m_PlaybackMixerChannel, /* force = */ true);
    }
    return true;
}



bool InternetRadio::noticeSoundStreamClosed(SoundStreamID id)
{
    if (id == m_SoundStreamSourceID) {
        return true;
    } else if (id == m_SoundStreamSinkID) {
        return true;
    } else {
        return false;
    }
}


bool InternetRadio::noticeSoundStreamSourceRedirected(SoundStreamID oldID, SoundStreamID newID)
{
    if (m_SoundStreamSourceID == oldID) {
        m_SoundStreamSourceID = newID;
        notifyCurrentSoundStreamSourceIDChanged(m_SoundStreamSourceID);
        return true;
    } else {
        return false;
    }
}



bool InternetRadio::noticeSoundStreamSinkRedirected(SoundStreamID oldID, SoundStreamID newID)
{
    if (m_SoundStreamSinkID == oldID) {
        m_SoundStreamSinkID = newID;
        notifyCurrentSoundStreamSinkIDChanged(m_SoundStreamSinkID);
        return true;
    } else {
        return false;
    }
}



// IRadioDevice methods

bool InternetRadio::setPower (bool on)
{
    return on ? powerOn() : powerOff();
}

void InternetRadio::searchMixer(ISoundStreamClient **playback_mixer)
{
    if (playback_mixer) {
        *playback_mixer = getSoundStreamClientWithID(m_PlaybackMixerID);
        if (!*playback_mixer) {
            QList<ISoundStreamClient*> playback_mixers = queryPlaybackMixers();
            if (!playback_mixers.isEmpty())
                *playback_mixer = playback_mixers.first();
        }
    }
}


bool InternetRadio::powerOn ()
{
    if (isPowerOn())
        return true;

    radio_init();

    if (isPowerOn()) {
        ISoundStreamClient *playback_mixer = NULL;

        searchMixer(&playback_mixer);

        if (playback_mixer)
            playback_mixer->preparePlayback(m_SoundStreamSinkID, m_PlaybackMixerChannel, true, false);

        sendStartPlayback(m_SoundStreamSinkID);

        float tmp_vol = 0;
        queryPlaybackVolume(m_SoundStreamSinkID, tmp_vol);
        if (tmp_vol < 0.005)
            sendPlaybackVolume(m_SoundStreamSinkID, m_defaultPlaybackVolume);

        unmuteSource  (m_SoundStreamSourceID);
        sendUnmuteSink(m_SoundStreamSourceID);
        notifyPowerChanged(true);
        notifyStationChanged(m_currentStation);
        notifyURLChanged(m_currentStation.url(), &m_currentStation);

        bool s = false;
        isStereo(m_SoundStreamSourceID, s);
        notifyStereoChanged(m_SoundStreamSourceID, s);
        float sq = 1.0;
        getSignalQuality(m_SoundStreamSourceID, sq);
        notifySignalQualityChanged(m_SoundStreamSourceID, sq);

    }

    return true;
}


bool InternetRadio::powerOff ()
{
    if (! isPowerOn())
        return true;

    queryPlaybackVolume(m_SoundStreamSinkID, m_defaultPlaybackVolume);
    sendMuteSink(m_SoundStreamSourceID);
    muteSource  (m_SoundStreamSourceID);
    radio_done();

    sendStopPlayback(m_SoundStreamSinkID);
    sendStopCapture (m_SoundStreamSinkID);
    closeSoundStream(m_SoundStreamSinkID);
    closeSoundStream(m_SoundStreamSourceID);
    m_SoundStreamSourceID = createNewSoundStream(m_SoundStreamSourceID, false);
    m_SoundStreamSinkID   = m_SoundStreamSourceID;
    notifySoundStreamCreated(m_SoundStreamSourceID);
    notifyCurrentSoundStreamSinkIDChanged  (m_SoundStreamSinkID);
    notifyCurrentSoundStreamSourceIDChanged(m_SoundStreamSourceID);

    if (isPowerOff()) {
        notifyPowerChanged(false);
    }

    updateRDSState      (false);
    updateRDSStationName(QString());
    updateRDSRadioText  (QString());

    bool s = false;
    isStereo(m_SoundStreamSourceID, s);
    notifyStereoChanged(m_SoundStreamSourceID, s);
    float sq = 1.0;
    getSignalQuality(m_SoundStreamSourceID, sq);
    notifySignalQualityChanged(m_SoundStreamSourceID, sq);

    return true;
}


bool InternetRadio::activateStation(const RadioStation &rs)
{
    const InternetRadioStation *irs = dynamic_cast<const InternetRadioStation*>(&rs);
    if (irs == NULL)
        return false;

    if (setURL(irs->url(), irs)) {
        m_currentStation = *irs;

        if (irs->initialVolume() > 0)
            setPlaybackVolume(m_SoundStreamSinkID, irs->initialVolume());

        return true;
    }

    return false;
}



bool InternetRadio::isPowerOn() const
{
    return m_decoderThread;
}


bool InternetRadio::isPowerOff() const
{
    return !isPowerOn();
}


/*SoundStreamID InternetRadio::getSoundStreamSinkID() const
{
    return m_SoundStreamSinkID;
}


SoundStreamID InternetRadio::getSoundStreamSourceID() const
{
    return m_SoundStreamSourceID;
}

*/
const RadioStation &InternetRadio::getCurrentStation() const
{
    return m_currentStation;
}


static QString staticInternetRadioDescription;

const QString &InternetRadio::getDescription() const
{
    if (!staticInternetRadioDescription.length()) {
       staticInternetRadioDescription = i18n("Internet radio station decoder / pseudo device");
    }
    return staticInternetRadioDescription;
}


SoundStreamID InternetRadio::getCurrentSoundStreamSinkID() const
{
    return m_SoundStreamSinkID;
}

SoundStreamID InternetRadio::getCurrentSoundStreamSourceID() const
{
    return m_SoundStreamSourceID;
}

bool InternetRadio::getRDSState() const
{
    return m_RDS_visible;
}

const QString &InternetRadio::getRDSRadioText() const
{
    return m_RDS_RadioText;
}

const QString &InternetRadio::getRDSStationName() const
{
    return m_RDS_StationName;
}




// soundstreamclient stuff
// we are only reacting on sourceID requests. effect plugins etc. have to forward and/or
// buffer(timeshifter) such "properties"

bool InternetRadio::muteSource (SoundStreamID id, bool mute)
{
    if (id != m_SoundStreamSourceID)
        return false;

    if (m_muted != mute) {
        m_muted = mute;
        notifySourceMuted(id, m_muted);
        return true;
    }
    return false;
}


bool InternetRadio::unmuteSource (SoundStreamID id, bool unmute)
{
    return muteSource(id, !unmute);
}


bool InternetRadio::getSignalQuality(SoundStreamID id, float &q) const
{
    if (id != m_SoundStreamSourceID)
        return false;

    q = 1.0; // we always have good reception :)... perhaps we can later add some code that looks for skips in the sound output
    return true;
}


bool   InternetRadio::hasGoodQuality(SoundStreamID id, bool &good) const
{
    if (id != m_SoundStreamSourceID)
        return false;

    good = true;
    return true;
}


bool    InternetRadio::isStereo(SoundStreamID id, bool &s) const
{
    if (id != m_SoundStreamSourceID)
        return false;

    s = m_stereoFlag;
    return true;
}


bool    InternetRadio::isSourceMuted(SoundStreamID id, bool &m) const
{
    if (id != m_SoundStreamSourceID)
        return false;

    m = m_muted;
    return true;
}


// IInternetRadioCfg methods


bool  InternetRadio::setPlaybackMixer(const QString &soundStreamClientID, const QString &ch, bool force)
{
    bool change = m_PlaybackMixerID != soundStreamClientID || m_PlaybackMixerChannel != ch;
    m_PlaybackMixerID      = soundStreamClientID;
    m_PlaybackMixerChannel = ch;

    if (change || force) {
        if (isPowerOn()) {
            // only send start/stop playback if we still have a direct link to the mixer
            if (m_SoundStreamSinkID == m_SoundStreamSourceID) {
                queryPlaybackVolume(m_SoundStreamSourceID, m_defaultPlaybackVolume);
                sendStopPlayback   (m_SoundStreamSourceID);
                sendReleasePlayback(m_SoundStreamSourceID);
            }
        }

        ISoundStreamClient *playback_mixer = NULL;
        searchMixer(&playback_mixer);
        if (playback_mixer)
            playback_mixer->preparePlayback(m_SoundStreamSourceID, m_PlaybackMixerChannel, true, false);

        if (isPowerOn()) {
            // only send start/stop playback if we still have a direct link to the mixer
            if (m_SoundStreamSinkID == m_SoundStreamSourceID) {
                sendStartPlayback (m_SoundStreamSourceID);
                sendPlaybackVolume(m_SoundStreamSourceID, m_defaultPlaybackVolume);
            }
        }

        if (change)
            emit sigNotifyPlaybackMixerChanged(soundStreamClientID, ch);
    }
    return true;
}


void InternetRadio::slotNoticePlaybackMixerChanged(const QString &mixerID, const QString &channelID, bool force)
{
    setPlaybackMixer(mixerID, channelID, force);
}


// PluginBase methods

void   InternetRadio::saveState (KConfigGroup &config) const
{
    PluginBase::saveState(config);

    config.writeEntry("PlaybackMixerID",        m_PlaybackMixerID);
    config.writeEntry("PlaybackMixerChannel",   m_PlaybackMixerChannel);
    config.writeEntry("defaultPlaybackVolume",  m_defaultPlaybackVolume);
    config.writeEntry("URL",                    m_currentStation.url());
    config.writeEntry("PowerOn",                isPowerOn());

    saveRadioDeviceID(config);
}


void   InternetRadio::restoreState (const KConfigGroup &config)
{
    PluginBase::restoreState(config);

    restoreRadioDeviceID(config);
    QString PlaybackMixerID      = config.readEntry ("PlaybackMixerID", QString());
    QString PlaybackMixerChannel = config.readEntry ("PlaybackMixerChannel", "Line");
    m_defaultPlaybackVolume      = config.readEntry ("defaultPlaybackVolume", 0.5);

    setPlaybackMixer(PlaybackMixerID, PlaybackMixerChannel, /* force = */ true);

    setURL(config.readEntry("URL", KUrl()), NULL);
    m_restorePowerOn = config.readEntry ("PowerOn", false);

    if (isPowerOff())
        notifyPlaybackVolumeChanged(m_SoundStreamSinkID, m_defaultPlaybackVolume);
}

void InternetRadio::startPlugin()
{
    PluginBase::startPlugin();
    setPower(m_restorePowerOn);
}

ConfigPageInfo InternetRadio::createConfigurationPage()
{
    InternetRadioConfiguration *conf = new InternetRadioConfiguration(NULL, m_SoundStreamSourceID);

    QObject::connect(this, SIGNAL(sigNotifyPlaybackMixerChanged (const QString &, const QString &, bool)),
                     conf, SLOT  (slotNoticePlaybackMixerChanged(const QString &, const QString &, bool)));

    QObject::connect(conf, SIGNAL(sigPlaybackMixerChanged (const QString &, const QString &, bool)),
                     this, SLOT  (slotNoticePlaybackMixerChanged(const QString &, const QString &, bool)));

    return ConfigPageInfo (conf,
                           i18n("Internet Radio"),
                           i18n("Internet Radio Options"),
                           "network-wired"
                           );
}

// IRadioClient

bool InternetRadio::noticeStationsChanged(const StationList &sl)
{
    const InternetRadioStation *irs = findMatchingStation(sl);
    if (irs) {
        if (irs->stationID() == m_currentStation.stationID()) {
            m_currentStation = *irs;
            notifyStationChanged(m_currentStation);
        } else {
            KUrl oldurl = m_currentStation.url();
            m_currentStation = *irs;
            m_currentStation.setUrl(oldurl);
            notifyStationChanged(m_currentStation);
        }
    }
    return true;
}


////////////////////////////////////////
// anything else

void InternetRadio::radio_init()
{
    m_stereoFlag = false;
    if (m_decoderThread) {
        return; // should not happen
    }
    freeAllBuffers(); // just to be sure;-)

//     m_MimetypeJob = KIO::mimetype(m_currentStation.url());
//     QObject::connect(m_MimetypeJob, SIGNAL(mimetype(KIO::Job *, const QString &)), this, SLOT(slotMimetypeResult(KIO::Job *, const QString &)));
//     m_MimetypeJob->start();

    m_decoderThread = new DecoderThread(this, m_currentStation, MAX_BUFFERS);
    m_decoderThread->start();

    m_waitForBufferMinFill = true;
    logDebug(QString("InternetRadio::radio_init"));
}


// void InternetRadio::slotMimetypeResult(KIO::Job *job, const QString &type)
// {
//     if (job && !job->error()) {
//         logDebug(QString("InternetRadio::radio_init: mimetype for %1: %2").arg(m_currentStation.url().pathOrUrl()).arg(type));
//     }
// }
//

void InternetRadio::radio_done()
{
// /*    if (m_MimetypeJob) {
//         delete m_MimetypeJob;
//     }
//     m_MimetypeJob = NULL;*/

    m_stereoFlag = false;
    if (m_decoderThread) {
        m_decoderThread->setDone();
    }
    freeAllBuffers();
    if (m_decoderThread) {
        if (!m_decoderThread->wait(2000)) { // wait at max 2000 ms, otherwise we'll kill you ;-)
            m_decoderThread->terminate();
            m_decoderThread->wait();
            logWarning(i18n("I'm sorry, but I'd to kill the ffmpeg decoder thread. It might be a good idea to restart KRadio"));
        }
        delete m_decoderThread;
        m_decoderThread = NULL;
    }
}



bool InternetRadio::setPlaybackVolume(SoundStreamID id, float volume)
{
    if (isPowerOff() && id == m_SoundStreamSinkID) {
        m_defaultPlaybackVolume = min(max(volume, 0.0), 1.0);
        return true;
    } else {
        return false;
    }
}

bool InternetRadio::getPlaybackVolume(SoundStreamID id, float &volume) const
{
    if (isPowerOff() && id == m_SoundStreamSinkID) {
        volume = m_defaultPlaybackVolume;
        return true;
    } else {
        return false;
    }
}



bool InternetRadio::getSoundStreamDescription(SoundStreamID id, QString &descr) const
{
    if (id == m_SoundStreamSourceID) {
        descr = name() + " - " + m_currentStation.name();
        return true;
    }
    else {
        return false;
    }
}


bool InternetRadio::getSoundStreamRadioStation(SoundStreamID id, const RadioStation *&rs) const
{
    if (id == m_SoundStreamSourceID) {
        rs = &m_currentStation;
        return true;
    }
    else {
        return false;
    }
}


bool InternetRadio::enumerateSourceSoundStreams(QMap<QString, SoundStreamID> &list) const
{
    if (m_SoundStreamSourceID.isValid()) {
        QString tmp = QString();
        getSoundStreamDescription(m_SoundStreamSourceID, tmp);
        list[tmp] = m_SoundStreamSourceID;
        return true;
    }
    return false;
}





void InternetRadio::updateRDSState      (bool enabled)
{
    if (enabled != m_RDS_visible) {
        m_RDS_visible = enabled;
        notifyRDSStateChanged(m_RDS_visible);
    }
}

void InternetRadio::updateRDSStationName(const QString &s)
{
    if (m_RDS_StationName != s) {
        m_RDS_StationName = s;
        notifyRDSStationNameChanged(m_RDS_StationName);
    }
}

void InternetRadio::updateRDSRadioText  (const QString &s)
{
    if (m_RDS_RadioText != s) {
        m_RDS_RadioText = s;
        notifyRDSRadioTextChanged(m_RDS_RadioText);
    }
}




bool InternetRadio::noticeReadyForPlaybackData(SoundStreamID id, size_t free_size)
{
    if (!id.isValid() || id != m_SoundStreamSourceID)
        return false;

    if (m_decoderThread && m_decoderThread->warning()) {
        logWarning(m_decoderThread->warningString());
    }
    if (m_decoderThread && m_decoderThread->debug()) {
        logDebug(m_decoderThread->debugString());
    }
    if (m_decoderThread && m_decoderThread->error()) {
        logError(m_decoderThread->errorString());
        powerOff();
        return false;
    }

    int min_size = m_waitForBufferMinFill ? MIN_BUFFERS4PLAYBACK : 1;

    if (!m_decoderThread)
        return false;

    int n_bufs = m_decoderThread->availableBuffers();

    if (n_bufs < min_size) {
        m_waitForBufferMinFill = true;
//         logDebug(QString("InternetRadio::noticeReadyForPlaybackData: SKIP: min_count = %1, buf_count = %2").arg(min_size).arg(n_bufs));
        return false;
    }

    m_waitForBufferMinFill = false;

    size_t consumed_size = SIZE_T_DONT_CARE;
    while ((n_bufs = m_decoderThread->availableBuffers()) && (free_size > 0) && (consumed_size != 0)) {

        DataBuffer         &buf           = m_decoderThread->getFirstBuffer();
        const char         *data          = buf.currentPointer();
        size_t              size          = buf.remainingSize();
                            consumed_size = SIZE_T_DONT_CARE;
        const SoundMetaData &md           = buf.metaData();
        const SoundFormat   &sf           = buf.soundFormat();

        if (m_stereoFlag != (sf.m_Channels >= 2)) {
            m_stereoFlag = (sf.m_Channels >= 2);
            notifyStereoChanged(m_SoundStreamSourceID, m_stereoFlag);
        }

//         logDebug(QString("InternetRadio::noticeReadyForPlaybackData: PLAY: buf_size = %1, min_count = %2, buf_count = %3").arg(size).arg(min_size).arg(n_bufs));

        notifySoundStreamData(id, sf, data, size, consumed_size, md);

        if (consumed_size == SIZE_T_DONT_CARE) {
            consumed_size = size;
        }
        free_size -= consumed_size;
        buf.addProcessedSize(consumed_size);
        if (buf.remainingSize() <= 0) {
            m_decoderThread->popFirstBuffer();
        }
    }
    return true;
}





bool InternetRadio::event(QEvent *_e)
{
    if (SoundStreamDecodingEvent::isSoundStreamDecodingEvent(_e)) {
        SoundStreamDecodingEvent *e  = static_cast<SoundStreamDecodingEvent*>(_e);

        if (m_decoderThread) {
            if (m_decoderThread->warning()) {
                logWarning(m_decoderThread->warningString());
            }
            if (m_decoderThread && m_decoderThread->debug()) {
                logDebug(m_decoderThread->debugString());
            }
            if (m_decoderThread->error()) {
                logError(m_decoderThread->errorString());
                powerOff();
            } else {
                if (e->type() == SoundStreamDecodingTerminated) {
/*                } else if (!m_muted && e->type() == SoundStreamDecodingStep) {
                    SoundStreamDecodingStepEvent *step = static_cast<SoundStreamDecodingStepEvent*>(e);
                    logDebug(QString("InternetRadio::event: STORE: buf_size = %1, buf_count = %2").arg(step->size()).arg(m_DataQueue.size())+1);

                    char *data              = step->takeData();
                    size_t s                = step->size();
                    const SoundMetaData &md = step->metaData();
                    const SoundFormat   &sf = step->getSoundFormat();
                    m_DataQueue.push_back(DataBuffer(data, s, md, sf));
*/                }
            }
        }
        return true;
    } else {
        return QObject::event(_e);
    }
}


void InternetRadio::freeAllBuffers()
{
    if (m_decoderThread) {
        m_decoderThread->flushBuffers();
    }
}




bool InternetRadio::setURL(const KUrl &url, const InternetRadioStation *rs)
{
    KUrl oldurl = m_currentStation.url();

    if (rs) {
        m_currentStation = *rs;
    } else {
        m_currentStation                = InternetRadioStation(url, "", "");
        const InternetRadioStation *irs = findMatchingStation(queryStations());
        if (irs) {
            m_currentStation = *irs;
            m_currentStation.setUrl(url);
        }
    }

    if (oldurl != url) {
        if (isPowerOn()) {
            radio_done();
            radio_init();
        }
        notifyURLChanged(url, &m_currentStation);
    }

    bool s = false;
    isStereo(m_SoundStreamSourceID, s);
    notifyStereoChanged(m_SoundStreamSourceID, s);
    float sq = 1.0;
    getSignalQuality(m_SoundStreamSourceID, sq);
    notifySignalQualityChanged(m_SoundStreamSourceID, sq);

    notifyStationChanged(m_currentStation);
    notifySoundStreamChanged(m_SoundStreamSourceID);

    return true;
}

const KUrl &InternetRadio::getURL() const
{
    return m_currentStation.url();
}



// bool InternetRadio::prepareCapture(SoundStreamID id, const QString &channel)
// {
//     if (id.isValid() && m_SoundStreamID == id) {
//         return true;
//     }
//     return false;
// }

bool InternetRadio::startCaptureWithFormat(SoundStreamID      id,
                                           const SoundFormat &/*proposed_format*/,
                                           SoundFormat       &real_format,
                                           bool /*force_format*/)
{
    if (id != m_SoundStreamSourceID)
        return false;

    if (!isPowerOn()) {
        powerOn();
        for (int i = 0; i < 20 && m_decoderThread && !m_decoderThread->initDone(); ++i) {
            sleep (200);
        } // wait max 4 secs
    }
    if (m_decoderThread && m_decoderThread->initDone()) {
        real_format = m_decoderThread->soundFormat();
        return true;
    } else {
        return false;
    }
}



const InternetRadioStation *InternetRadio::findMatchingStation(const StationList &sl) const
{
    for (StationList::const_iterator it = sl.begin(); it != sl.end(); ++it) {
        const InternetRadioStation *irs = dynamic_cast<const InternetRadioStation *>(*it);
        if (irs && irs->url() == m_currentStation.url()) {
            return irs;
        }
    }
    return NULL;
}







#include "internetradio.moc"