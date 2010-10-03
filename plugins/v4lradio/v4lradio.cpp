/***************************************************************************
                          v4lradio.cpp  -  description
                             -------------------
    begin                : Don M�r  8 21:57:17 CET 2001
    copyright            : (C) 2002-2005 by Ernst Martin Witte
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

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <math.h>
#include <sys/utsname.h>



#ifdef HAVE_V4L2
#include "linux/videodev2.h"
#endif
#include "linux/videodev.h"
#include <linux/soundcard.h>

#include <string.h> // memcpy needed

#include <QtGui/QLayout>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QList>
#include <QtCore/QSocketNotifier>

#include <kconfiggroup.h>
#include <kiconloader.h>
#include <kaboutdata.h>
#include <klocale.h>

#include "stationlist.h"
// #warning "FIXME: port aboutwidget"
//#include "../../src/include/aboutwidget.h"
#include "utils.h"
#include "v4lradio.h"
#include "v4lradio-configuration.h"

#include "debug-profiler.h"

#include "rds_group_v4l.h"

struct _lrvol { unsigned char l, r; short dummy; };

///////////////////////////////////////////////////////////////////////

PLUGIN_LIBRARY_FUNCTIONS(V4LRadio, PROJECT_NAME, i18n("Support for V4L(2) Radio Devices"));

///////////////////////////////////////////////////////////////////////

#define IGNORED_INIT_DUMMY_V4LDEV_STRING    QString("this is some dummy to be ignored")

#define RDS_ERROR_RATE_SUBSAMPLING_RATE  (10*45) // 1187.5 bits/s / 26 bits/block ~= 45.67 blocks/s

V4LRadio::V4LRadio(const QString &instanceID, const QString &name)
  : PluginBase(instanceID, name, i18n("Video For Linux Plugin")),
    m_treble(0.5),
    m_bass(0.5),
    m_balance(0),
    m_deviceVolume(0.9),
    m_muted(false),
    m_signalQuality(0),
    m_stereo(false),
    m_stereoMode(STATION_STEREO_DONTCARE),
    m_minQuality(0.75),
    m_minFrequency(87.0),
    m_maxFrequency(108.0),
    m_lastMinDevFrequency(87.0),
    m_lastMaxDevFrequency(108.0),

    m_defaultPlaybackVolume(0.5),

    m_scanStep(0.05),

    m_radioDev(IGNORED_INIT_DUMMY_V4LDEV_STRING),
    m_radio_fd(-1),
    m_useOldV4L2Calls(true),
    m_pollTimer(this),

    m_blockReadTuner(false),
    m_blockReadAudio(false),

    m_PlaybackMixerID(QString()),
    m_CaptureMixerID(QString()),
    m_PlaybackMixerChannel(QString()),
    m_CaptureMixerChannel(QString()),
    m_ActivePlayback(false),
    m_ActivePlaybackMuteCaptureChannelPlayback(true),
    m_MuteOnPowerOff(false),
    m_VolumeZeroOnPowerOff(false),
    m_restorePowerOn(false),
    m_V4L_version_override(V4L_Version2),
    m_V4L_version_override_by_kernel_once(false),
    m_RDS_notify(NULL),
    m_RDS_visible(false),
    m_RDS_decoder(new RDSGroupV4L()),
    m_RDS_errorRate_subsample_counter(0),
    m_RDSForceEnabled(false)
{
    m_SoundStreamSourceID = createNewSoundStream(false);
    m_SoundStreamSinkID   = m_SoundStreamSourceID;


#ifdef RDS_DEBUG_EMULATE
    m_RDS_emulate_pos = 0;
    m_RDS_emulate_timer.setInterval(21); // 26 bits/block / 1187.5 bits/s = 21.89 ms / block
    m_RDS_emulate_timer.setSingleShot(false);
    QObject::connect (&m_RDS_emulate_timer, SIGNAL(timeout()), this, SLOT(slotEmulateRDS()));
#endif

    QObject::connect (&m_pollTimer, SIGNAL(timeout()), this, SLOT(poll()));
    m_pollTimer.setSingleShot(false);
    m_pollTimer.setInterval(333);

    m_audio = new video_audio;
    bzero(m_audio, sizeof(video_audio));
    m_tuner = new video_tuner;
    bzero(m_tuner, sizeof(video_tuner));
#ifdef HAVE_V4L2
    m_tuner2 = new v4l2_tuner;
    bzero(m_tuner2, sizeof(v4l2_tuner));
#endif

    m_seekHelper = new FrequencySeekHelper(*this);
    m_seekHelper->connectI(this);
}


V4LRadio::~V4LRadio()
{
    setPower(false);

    if (m_seekHelper)
        delete m_seekHelper;

    if (m_audio)  delete m_audio;
    if (m_tuner)  delete m_tuner;
#ifdef HAVE_V4L2
    if (m_tuner2) delete m_tuner2;
#endif
}


bool V4LRadio::connectI (Interface *i)
{
    bool a = IRadioDevice::connectI(i);
    bool b = IRadioClient::connectI(i);
    bool c = ISeekRadio::connectI(i);
    bool d = IFrequencyRadio::connectI(i);
    bool e = IV4LCfg::connectI(i);
    bool f = PluginBase::connectI(i);
    bool g = ISoundStreamClient::connectI(i);
    return a || b || c || d || e || f || g;
}


bool V4LRadio::disconnectI (Interface *i)
{
    bool a = IRadioDevice::disconnectI(i);
    bool b = IRadioClient::connectI(i);
    bool c = ISeekRadio::disconnectI(i);
    bool d = IFrequencyRadio::disconnectI(i);
    bool e = IV4LCfg::disconnectI(i);
    bool f = PluginBase::disconnectI(i);
    bool g = ISoundStreamClient::disconnectI(i);
    m_seekHelper->disconnectI(i);
    return a || b || c || d || e || f || g;
}


void V4LRadio::noticeConnectedI (ISoundStreamServer *s, bool pointer_valid)
{
    ISoundStreamClient::noticeConnectedI(s, pointer_valid);
    if (s && pointer_valid) {
        m_seekHelper->connectI(s);

        s->register4_notifyPlaybackChannelsChanged(this);
        s->register4_notifyCaptureChannelsChanged(this);

        s->register4_queryPlaybackVolume(this);
        s->register4_sendTreble(this);
        s->register4_sendBass(this);
        s->register4_sendBalance(this);
        s->register4_sendMuteSource(this);
        s->register4_sendUnmuteSource(this);
        s->register4_sendSignalMinQuality(this);
        s->register4_sendStereoMode(this);

        s->register4_queryTreble(this);
        s->register4_queryBass(this);
        s->register4_queryBalance(this);
        s->register4_querySignalQuality(this);
        s->register4_querySignalMinQuality(this);
        s->register4_queryHasGoodQuality(this);
        s->register4_queryIsStereo(this);
        s->register4_queryIsSourceMuted(this);


        s->register4_sendPlaybackVolume(this);
        s->register4_sendCaptureVolume(this);

        s->register4_sendStopCapture(this);

        s->register4_querySoundStreamDescription(this);
        s->register4_querySoundStreamRadioStation(this);
        s->register4_queryEnumerateSourceSoundStreams(this);

        s->register4_notifySoundStreamSourceRedirected(this);
        s->register4_notifySoundStreamSinkRedirected(this);
        notifySoundStreamCreated(m_SoundStreamSinkID);
        if (m_SoundStreamSinkID != m_SoundStreamSourceID) {
            notifySoundStreamCreated(m_SoundStreamSourceID);
        }
    }
}

void V4LRadio::noticeConnectedSoundClient(ISoundStreamClient::thisInterface *i, bool pointer_valid)
{
    if (i && pointer_valid && i->getSoundStreamClientID() == m_PlaybackMixerID) {
        setPlaybackMixer(m_PlaybackMixerID, m_PlaybackMixerChannel, /* force = */ true);
    }
    if (i && pointer_valid && i->getSoundStreamClientID() == m_CaptureMixerID) {
        setCaptureMixer(m_CaptureMixerID, m_CaptureMixerChannel, /* force = */ true);
    }
}


bool V4LRadio::noticePlaybackChannelsChanged(const QString & client_id, const QStringList &/*channels*/)
{
    if (client_id == m_PlaybackMixerID) {
        setPlaybackMixer(m_PlaybackMixerID, m_PlaybackMixerChannel, /* force = */ true);
    }
    return true;
}


bool V4LRadio::noticeCaptureChannelsChanged (const QString & client_id, const QStringList &/*channels*/)
{
    if (client_id == m_CaptureMixerID) {
        setCaptureMixer(m_CaptureMixerID, m_CaptureMixerChannel, /* force = */ true);
    }
    return true;
}


// IRadioDevice methods

bool V4LRadio::setPower (bool on)
{
    return on ? powerOn() : powerOff();
}

void V4LRadio::searchMixers(ISoundStreamClient **playback_mixer, ISoundStreamClient **capture_mixer)
{
    if (playback_mixer) {
        *playback_mixer = getSoundStreamClientWithID(m_PlaybackMixerID);
        if (!*playback_mixer) {
            QList<ISoundStreamClient*> playback_mixers = queryPlaybackMixers();
            if (!playback_mixers.isEmpty())
                *playback_mixer = playback_mixers.first();
        }
    }
    if (capture_mixer) {
        *capture_mixer  = getSoundStreamClientWithID(m_CaptureMixerID);
        if (!*capture_mixer) {
            QList<ISoundStreamClient*> capture_mixers  = queryCaptureMixers();
            if (!capture_mixers.isEmpty())
                *capture_mixer = capture_mixers.first();
        }
    }
}


bool V4LRadio::powerOn ()
{
    if (isPowerOn())
        return true;

    radio_init();

    if (isPowerOn()) {
#ifdef RDS_DEBUG_EMULATE
        m_RDS_emulate_timer.start();
#endif
        m_pollTimer.start();

        ISoundStreamClient *playback_mixer = NULL,
                           *capture_mixer  = NULL;

        searchMixers(&playback_mixer, &capture_mixer);

        if (playback_mixer)
            playback_mixer->preparePlayback(m_SoundStreamSinkID, m_PlaybackMixerChannel, m_ActivePlayback, false);
        if (capture_mixer)
            capture_mixer->prepareCapture(m_SoundStreamSourceID, m_CaptureMixerChannel);

        sendStartPlayback(m_SoundStreamSinkID);
        float tmp_vol = 0;
        queryPlaybackVolume(m_SoundStreamSinkID, tmp_vol);
        if (tmp_vol < 0.005) {
            sendPlaybackVolume(m_SoundStreamSinkID, m_defaultPlaybackVolume);
        }

        if (m_ActivePlayback) {
            SoundFormat sf;
            sendStartCaptureWithFormat(m_SoundStreamSourceID, sf, sf);
        }

        if (m_ActivePlayback && m_ActivePlaybackMuteCaptureChannelPlayback) {
            sendMuteSourcePlayback(m_SoundStreamSourceID);
        }

        // Fix for drivers that are not reporting the mute state correctly:
        // force internal copy of mute state to be synced correctly by first
        // muting and unmuting again. At least the second call will have an effect.
        sendMuteSource  (m_SoundStreamSourceID);
        sendUnmuteSource(m_SoundStreamSourceID);

        // ... same fix again ...
        sendMuteSink    (m_SoundStreamSinkID);
        sendUnmuteSink  (m_SoundStreamSinkID);

        notifyPowerChanged(true);
        notifySoundStreamChanged(m_SoundStreamSourceID);
    }

    return true;
}


bool V4LRadio::powerOff ()
{
    if (! isPowerOn())
        return true;

    #ifdef RDS_DEBUG_EMULATE
    m_RDS_emulate_timer.stop();
    #endif
    m_pollTimer.stop();

    queryPlaybackVolume(m_SoundStreamSinkID, m_defaultPlaybackVolume);
    if (m_MuteOnPowerOff) {
        sendMuteSink  (m_SoundStreamSourceID, true);
    }
    if (m_VolumeZeroOnPowerOff)
        sendPlaybackVolume(m_SoundStreamSinkID, 0.0);
    muteSource(m_SoundStreamSourceID, true);
    radio_done();

    sendStopRecording(m_SoundStreamSinkID);
    sendStopPlayback (m_SoundStreamSinkID);
    sendStopCapture  (m_SoundStreamSinkID);
    closeSoundStream (m_SoundStreamSourceID);
    closeSoundStream (m_SoundStreamSinkID);
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

    return true;
}


bool V4LRadio::activateStation(const RadioStation &rs)
{
    const FrequencyRadioStation *frs = dynamic_cast<const FrequencyRadioStation*>(&rs);
    if (frs == NULL)
        return false;

    if (setFrequency(frs->frequency(), frs)) {
        m_currentStation = *frs;

        if (frs->initialVolume() > 0)
            setPlaybackVolume(m_SoundStreamSinkID, frs->initialVolume());

        setStereoMode(m_SoundStreamSourceID, frs->stereoMode());

        return true;
    }

    return false;
}



bool V4LRadio::isPowerOn() const
{
    return m_radio_fd >= 0;
}


bool V4LRadio::isPowerOff() const
{
    return m_radio_fd < 0;
}


const RadioStation &V4LRadio::getCurrentStation() const
{
    return m_currentStation;
}


const QString &V4LRadio::getDescription() const
{
    return m_caps.description;
}


SoundStreamID V4LRadio::getCurrentSoundStreamSourceID() const
{
    return m_SoundStreamSourceID;
}

SoundStreamID V4LRadio::getCurrentSoundStreamSinkID() const
{
    return m_SoundStreamSinkID;
}

bool V4LRadio::getRDSState() const
{
    return m_RDS_visible;
}

const QString &V4LRadio::getRDSRadioText() const
{
    return m_RDS_RadioText;
}

const QString &V4LRadio::getRDSStationName() const
{
    return m_RDS_StationName;
}



bool V4LRadio::setTreble (SoundStreamID id, float t)
{
    if (id != m_SoundStreamSourceID)
        return false;

    if (t > 1.0) t = 1.0;
    if (t < 0)   t = 0.0;
    if ((int)rint(m_treble*65535) != (int)rint(t*65535)) {
        m_treble = t;
        writeAudioInfo();
        notifyTrebleChanged(id, t);
    }
    return true;
}


bool V4LRadio::setBass (SoundStreamID id, float b)
{
    if (id != m_SoundStreamSourceID)
        return false;

    if (b > 1.0) b = 1.0;
    if (b < 0)   b = 0.0;
    if ((int)rint(m_bass*65535) != (int)rint(b*65535)) {
        m_bass = b;
        writeAudioInfo();
        notifyBassChanged(id, b);
    }

    return true;
}


bool V4LRadio::setBalance (SoundStreamID id, float b)
{
    if (id != m_SoundStreamSourceID)
        return false;

    if (b > +1.0) b = +1.0;
    if (b < -1.0) b = -1.0;
    if ((int)rint(m_balance*32767) != (int)rint(b*32767)) {
        m_balance = b;
        writeAudioInfo();
        notifyBalanceChanged(id, b);
    }
    return true;
}


bool V4LRadio::setDeviceVolume (float v)
{
    if (v > 1.0) v = 1.0;
    if (v < 0)   v = 0;
    if ((int)rint(m_deviceVolume*65535) != (int)rint(v*65535)) {
        m_deviceVolume = v;
        writeAudioInfo();
        notifyDeviceVolumeChanged(v);
    }
    return true;
}


bool V4LRadio::muteSource(SoundStreamID id, bool mute)
{
    if (id != m_SoundStreamSourceID)
        return false;

    logDebug(QString("(un)muting v4l: old=%1, new=%2").arg(m_muted ? "muted" : "unmuted").arg(mute ? "muted" : "unmuted"));

    // We do not filter out the (possibly) unnecessary calls. Some drivers are reporting
    // the mute state wrongly. Therefore, unmuting would not work unless we disable this check.
    // if (m_muted != mute) {
        m_muted = mute;
        bool r = writeAudioInfo();
        if (r)
            notifySourceMuted(id, m_muted);
        return r;
    // }
    // return false;
}


bool V4LRadio::unmuteSource (SoundStreamID id, bool unmute)
{
    return muteSource(id, !unmute);
}


bool V4LRadio::setSignalMinQuality (SoundStreamID id, float mq)
{
    if (id != m_SoundStreamSourceID)
        return false;
    if (rint(mq*100) == rint(m_minQuality*100))
        return true;

    m_minQuality = mq;
    notifySignalMinQualityChanged(id, m_minQuality);
    return true;
}


bool    V4LRadio::setStereoMode(SoundStreamID id, StationStereoMode mode)
{
    if (id != m_SoundStreamSourceID)
        return false;

    m_stereoMode = mode;
    writeAudioInfo();
    readAudioInfo();
    return true;
}




bool V4LRadio::getTreble (SoundStreamID id, float &t) const
{
    if (id != m_SoundStreamSourceID)
        return false;

    readAudioInfo();
    t = m_treble;
    return true;
}


bool V4LRadio::getBass (SoundStreamID id, float &b) const
{
    if (id != m_SoundStreamSourceID)
        return false;

    readAudioInfo();
    b = m_bass;
    return true;
}


bool V4LRadio::getBalance (SoundStreamID id, float &b) const
{
    if (id != m_SoundStreamSourceID)
        return false;

    readAudioInfo();
    b = m_balance;
    return true;
}


float V4LRadio::getDeviceVolume () const
{
    readAudioInfo();
    return m_deviceVolume;
}



bool V4LRadio::getSignalMinQuality(SoundStreamID id, float &q) const
{
    if (id != m_SoundStreamSourceID)
        return false;

    q = m_minQuality;
    return true;
}


bool V4LRadio::getSignalQuality(SoundStreamID id, float &q) const
{
    if (id != m_SoundStreamSourceID)
        return false;

    readTunerInfo();
    q = m_signalQuality;
    return true;
}


bool   V4LRadio::hasGoodQuality(SoundStreamID id, bool &good) const
{
    if (id != m_SoundStreamSourceID)
        return false;

    float q = 0;
    if (getSignalQuality(id, q))
        good = q >= m_minQuality;
    return true;
}


bool    V4LRadio::isStereo(SoundStreamID id, bool &s) const
{
    if (id != m_SoundStreamSourceID)
        return false;

    readAudioInfo();
    s = m_stereo;
    return true;
}


bool    V4LRadio::isSourceMuted(SoundStreamID id, bool &m) const
{
    if (id != m_SoundStreamSourceID)
        return false;

    readAudioInfo();
    m = m_muted;
    return true;
}


// IRadioClient

bool V4LRadio::noticeStationsChanged(const StationList &sl)
{
    const FrequencyRadioStation *frs = findMatchingStation(sl);
    if (frs && frs->stationID() != m_currentStation.stationID()) {
        float oldf = m_currentStation.frequency();
        m_currentStation = *frs;
        m_currentStation.setFrequency(oldf);
        notifyStationChanged(m_currentStation);
    }
    return true;
}


// ISeekRadio

bool V4LRadio::toBeginning()
{
    setFrequency(getMinFrequency(), NULL);
    return true;
}

bool V4LRadio::toEnd()
{
    setFrequency(getMaxFrequency(), NULL);
    return true;
}

bool V4LRadio::startSeekUp()
{
    return startSeek(true);
}

bool V4LRadio::startSeekDown()
{
    return startSeek(false);
}

bool V4LRadio::startSeek(bool up)
{
    if (isPowerOn() && m_seekHelper) {
        m_seekHelper->start(m_SoundStreamSourceID, up ? SeekHelper::up : SeekHelper::down);
        return true;
    } else {
        return false;
    }
}

bool V4LRadio::stopSeek()
{
    if (m_seekHelper) m_seekHelper->stop();
    return true;
}

bool V4LRadio::isSeekRunning() const
{
    if (m_seekHelper)
        return m_seekHelper->isRunning();
    else
        return false;
}


bool V4LRadio::isSeekUpRunning() const
{
    if (m_seekHelper)
        return m_seekHelper->isRunningUp();
    else
        return false;
}


bool V4LRadio::isSeekDownRunning() const
{
    if (m_seekHelper)
        return m_seekHelper->isRunningDown();
    else
        return false;
}

float V4LRadio::getProgress () const
{
    float min = getMinFrequency();
    float max = getMaxFrequency();

    return (getFrequency() - min) / (max - min);
}


// IFrequencyRadio

bool V4LRadio::setFrequency(float freq, const FrequencyRadioStation *rs)
{
    float old_freq = m_currentStation.frequency();

    // ensure that we store the most recent station ID! (and do decision
    // if we don't need to do anything later)
    if (rs) {
        m_currentStation = *rs;
    } else {
        m_currentStation                 = FrequencyRadioStation(freq);
        const FrequencyRadioStation *frs = findMatchingStation(queryStations());
        if (frs) {
            m_currentStation = *frs;
            m_currentStation.setFrequency(freq);
        }
    }

    if (old_freq == freq) {
        return true;
    }

    float minf = getMinFrequency();
    float maxf = getMaxFrequency();

    if (isPowerOn()) {

        bool oldSourceMute = false;
        bool oldSinkMute   = false;
        queryIsSourceMuted(m_SoundStreamSourceID, oldSourceMute);
        queryIsSinkMuted  (m_SoundStreamSourceID, oldSinkMute);
        // We do not filter out the (possibly) unnecessary calls. Some drivers are reporting
        // the mute state wrongly. Therefore, unmuting would not work unless we disable this check.
        // if (!oldSourceMute && !m_ActivePlayback)
            sendMuteSource(m_SoundStreamSourceID);
        if (!oldSinkMute)
            sendMuteSink(m_SoundStreamSourceID);


        if (!m_tunercache.valid) readTunerInfo();
        float         df = m_tunercache.deltaF;

        unsigned long lfreq = (unsigned long) rint(freq / df);

        if (freq > maxf || freq < minf) {
            logError("V4LRadio::setFrequency: " +
                     i18n("invalid frequency %1", QString().setNum(freq)));
            // We do not filter out the (possibly) unnecessary calls. Some drivers are reporting
            // the mute state wrongly. Therefore, unmuting would not work unless we disable this check.
            // if (!oldSourceMute && !m_ActivePlayback)
                sendUnmuteSource(m_SoundStreamSourceID);
            if (!oldSinkMute)
                sendUnmuteSink(m_SoundStreamSourceID);
            return false;
        }

        int r = -1;
        if (m_V4L_version_override == V4L_Version1 && m_caps.v4l_version_support[V4L_Version1]) {
            r = ioctl(m_radio_fd, VIDIOCSFREQ, &lfreq);
        }
#ifdef HAVE_V4L2
        else if (m_V4L_version_override == V4L_Version2 && m_caps.v4l_version_support[V4L_Version2]) {
            v4l2_frequency   tmp;
            tmp.tuner = 0;
            tmp.type = V4L2_TUNER_RADIO;
            tmp.frequency = lfreq;
            r = ioctl(m_radio_fd, VIDIOC_S_FREQUENCY, &tmp);
        }
#endif
        else {
            logError("V4LRadio::setFrequency: " +
                     i18n("don't known how to handle V4L-version %1", (int)m_V4L_version_override));
            for (int i = 0; i < V4L_Version_COUNT; ++i) {
                logDebug(i18n("%1 Support: %2", V4LVersionStrings[i], m_caps.v4l_version_support[i]));
            }
        }

        if (r) {
            logError("V4LRadio::setFrequency: " +
                     i18n("error setting frequency to %1 (%2)", QString().setNum(freq), QString().setNum(r)));
            // unmute the old radio with the old radio station
            // We do not filter out the (possibly) unnecessary calls. Some drivers are reporting
            // the mute state wrongly. Therefore, unmuting would not work unless we disable this check.
            // if (!oldSourceMute && !m_ActivePlayback)
                sendUnmuteSource(m_SoundStreamSourceID);
            if (!oldSinkMute)
                sendUnmuteSink(m_SoundStreamSourceID);
            return false;
        }

        // unmute this radio device, because we now have the current radio station
        // We do not filter out the (possibly) unnecessary calls. Some drivers are reporting
        // the mute state wrongly. Therefore, unmuting would not work unless we disable this check.
        // if (!oldSourceMute && !m_ActivePlayback)
            sendUnmuteSource(m_SoundStreamSourceID);
        if (!oldSinkMute)
            sendUnmuteSink  (m_SoundStreamSourceID);
    }

//     m_currentStation.setFrequency(freq);

    updateRDSStationName(QString()); // erase RDS stuff and wait for new data
    updateRDSRadioText  (QString());

    notifyFrequencyChanged(freq, &m_currentStation);
    notifyStationChanged(m_currentStation);
    notifyProgress((freq - minf) / (maxf - minf));
    notifySoundStreamChanged(m_SoundStreamSourceID);
    return true;
}


bool V4LRadio::setMinFrequency (float minF)
{
    float oldm = getMinFrequency();
    m_minFrequency = minF;

    float newm = getMinFrequency();
    if (oldm != newm)
        notifyMinMaxFrequencyChanged(newm, getMaxFrequency());

    return true;
}


bool V4LRadio::setMaxFrequency (float maxF)
{
    float oldm = getMaxFrequency();
    m_maxFrequency = maxF;

    float newm = getMaxFrequency();
    if (oldm != newm)
        notifyMinMaxFrequencyChanged(getMinFrequency(), newm);

    return true;
}


bool V4LRadio::setScanStep(float s)
{
    float old = m_scanStep;
    m_scanStep = s;

    if (old != s) notifyScanStepChanged(m_scanStep);
    return true;
}


float V4LRadio::getFrequency()     const
{
    return m_currentStation.frequency();
}


float V4LRadio::getMinFrequency()  const
{
    return m_minFrequency ? m_minFrequency : getMinDeviceFrequency();
}


float V4LRadio::getMaxFrequency()  const
{
    return m_maxFrequency ? m_maxFrequency : getMaxDeviceFrequency();
}


float V4LRadio::getMinDeviceFrequency() const
{
    if (!m_tunercache.valid)
        readTunerInfo();

    return m_tunercache.minF;
}


float V4LRadio::getMaxDeviceFrequency() const
{
    if (!m_tunercache.valid)
        readTunerInfo();

    return m_tunercache.maxF;
}


float V4LRadio::getScanStep()      const
{
    return m_scanStep;
}



// IV4LCfg methods

bool  V4LRadio::setRadioDevice(const QString &s)
{
    if (m_radioDev != s) {
        bool p = isPowerOn();
        powerOff();
        m_radioDev = s;

        m_caps = readV4LCaps(m_radioDev);
        notifyRadioDeviceChanged(m_radioDev);
        notifyDescriptionChanged(m_caps.description);
        notifyCapabilitiesChanged(m_caps);
        setPower(p);
    }
    return true;
}

static inline void assignChannelIfValid(QString &dest_channel, const QString &test_channel, const QStringList &valid_channels)
{
    if (valid_channels.contains(test_channel) || !valid_channels.size()) {
        dest_channel = test_channel;
    }
}

bool  V4LRadio::setPlaybackMixer(const QString &soundStreamClientID, const QString &ch, bool force)
{
    QString old_channel          = m_PlaybackMixerChannel;
    m_PlaybackMixerID            = soundStreamClientID;
    ISoundStreamClient *mixer    = getSoundStreamClientWithID(m_PlaybackMixerID);
    QStringList         channels = mixer ? mixer->getPlaybackChannels() : QStringList();

    if (channels.size()) {
        assignChannelIfValid(m_PlaybackMixerChannel, channels[0], channels);  // lowest priority
    }
    assignChannelIfValid(m_PlaybackMixerChannel, "PCM",       channels);
    assignChannelIfValid(m_PlaybackMixerChannel, "Wave",      channels);
    assignChannelIfValid(m_PlaybackMixerChannel, "Line",      channels);
    assignChannelIfValid(m_PlaybackMixerChannel, "Master",    channels);
    assignChannelIfValid(m_PlaybackMixerChannel, ch,          channels);  // highest priority

    bool change = (m_PlaybackMixerID != soundStreamClientID) || (m_PlaybackMixerChannel != old_channel);

    if (change || force) {
        if (isPowerOn()) {
            // only send start/stop playback if we still have a direct link to the mixer
            if (m_SoundStreamSinkID == m_SoundStreamSourceID) {
                queryPlaybackVolume(m_SoundStreamSourceID, m_defaultPlaybackVolume);
                sendStopPlayback(m_SoundStreamSourceID);
                sendReleasePlayback(m_SoundStreamSourceID);
            }
        }

        ISoundStreamClient *playback_mixer = NULL;
        searchMixers(&playback_mixer, NULL);
        if (playback_mixer)
            playback_mixer->preparePlayback(m_SoundStreamSourceID, m_PlaybackMixerChannel, m_ActivePlayback, false);

        if (isPowerOn()) {
            // only send start/stop playback if we still have a direct link to the mixer
            if (m_SoundStreamSinkID == m_SoundStreamSourceID) {
                sendStartPlayback(m_SoundStreamSourceID);
                sendPlaybackVolume(m_SoundStreamSourceID, m_defaultPlaybackVolume);

                // hmmmmm... should we have this really here?
    /*            if (m_ActivePlayback) {
                    SoundFormat sf;
                    sendStartCaptureWithFormat(m_SoundStreamSourceID, sf, sf);
                }*/
            }
        }

        if (change) // update config IV4LCfg
            notifyPlaybackMixerChanged(soundStreamClientID, ch);
    }
    return true;
}


bool  V4LRadio::setCaptureMixer(const QString &soundStreamClientID, const QString &ch, bool force)
{
    QString old_channel          = m_CaptureMixerChannel;
    m_CaptureMixerID             = soundStreamClientID;
    ISoundStreamClient *mixer    = getSoundStreamClientWithID(m_CaptureMixerID);
    QStringList         channels = mixer ? mixer->getPlaybackChannels() : QStringList();

    if (channels.size()) {
        assignChannelIfValid(m_CaptureMixerChannel, channels[0], channels);  // lowest priority
    }
    assignChannelIfValid(m_CaptureMixerChannel, "PCM",       channels);
    assignChannelIfValid(m_CaptureMixerChannel, "Wave",      channels);
    assignChannelIfValid(m_CaptureMixerChannel, "Line",      channels);
    assignChannelIfValid(m_CaptureMixerChannel, "Master",    channels);
    assignChannelIfValid(m_CaptureMixerChannel, "Capture",   channels);
    assignChannelIfValid(m_CaptureMixerChannel, ch,          channels);  // highest priority

    bool change = (m_CaptureMixerID != soundStreamClientID) || (m_CaptureMixerChannel != old_channel);

    if (change || force) {

        bool r = false;
        SoundFormat sf;
        queryIsCaptureRunning(m_SoundStreamSourceID, r, sf);

        float v = 0;
        if (isPowerOn() && r) {
            queryCaptureVolume(m_SoundStreamSourceID, v);
            sendStopCapture   (m_SoundStreamSourceID);
            sendReleaseCapture(m_SoundStreamSourceID);
        }

        ISoundStreamClient *capture_mixer  = NULL;
        searchMixers(NULL, &capture_mixer);
        if (capture_mixer)
            capture_mixer->prepareCapture(m_SoundStreamSourceID, m_CaptureMixerChannel);

        if (isPowerOn() && r) {
            sendStartCaptureWithFormat(m_SoundStreamSourceID, sf, sf);
            sendCaptureVolume(m_SoundStreamSourceID, v);
            if (m_ActivePlayback && m_ActivePlaybackMuteCaptureChannelPlayback) {
                sendMuteSourcePlayback(m_SoundStreamSourceID);
            }
        }

        if (change) // update config IV4LCfg
            notifyCaptureMixerChanged(soundStreamClientID, ch);
    }
    return true;
}


V4LCaps V4LRadio::getCapabilities(const QString &dev) const
{
    if (dev.isNull()) {
        return m_caps;
    } else {
        return readV4LCaps(dev);
    }
}


bool V4LRadio::setActivePlayback(bool a, bool mute_capture_channel_playback)
{
    if ((a == m_ActivePlayback) && (mute_capture_channel_playback == m_ActivePlaybackMuteCaptureChannelPlayback))
        return true;


    if (isPowerOn()) {
        // only send start/stop playback if we still have a direct link to the mixer
        if (m_SoundStreamSinkID == m_SoundStreamSourceID) {
            queryPlaybackVolume(m_SoundStreamSinkID, m_defaultPlaybackVolume);
            sendStopPlayback(m_SoundStreamSinkID);
            sendReleasePlayback(m_SoundStreamSinkID);
        }
        if (m_ActivePlayback) {
            sendStopCapture(m_SoundStreamSourceID);
        }
    }

    m_ActivePlayback                           = a;
    m_ActivePlaybackMuteCaptureChannelPlayback = mute_capture_channel_playback;

    ISoundStreamClient *playback_mixer = NULL;
    searchMixers(&playback_mixer, NULL);
    if (playback_mixer)
        playback_mixer->preparePlayback(m_SoundStreamSourceID, m_PlaybackMixerChannel, m_ActivePlayback, false);

    if (isPowerOn()) {
        // only send start/stop playback if we still have a direct link to the mixer
        if (m_SoundStreamSinkID == m_SoundStreamSourceID) {
            sendStartPlayback(m_SoundStreamSourceID);
            sendPlaybackVolume(m_SoundStreamSourceID, m_defaultPlaybackVolume);
        }
        if (m_ActivePlayback) {
            SoundFormat sf;
            sendStartCaptureWithFormat(m_SoundStreamSourceID, sf, sf);
            if (m_ActivePlaybackMuteCaptureChannelPlayback) {
                sendMuteSourcePlayback(m_SoundStreamSourceID);
            }
        }
    }

    notifyActivePlaybackChanged(m_ActivePlayback, m_ActivePlaybackMuteCaptureChannelPlayback);

    return true;
}

bool V4LRadio::setForceRDSEnabled(bool a)
{
    if (a != m_RDSForceEnabled) {
        m_RDSForceEnabled = a;
        notifyForceRDSEnabledChanged(m_RDSForceEnabled);
    }
    return true;
}

bool V4LRadio::setMuteOnPowerOff(bool a)
{
    if (a != m_MuteOnPowerOff) {
        m_MuteOnPowerOff = a;
        notifyMuteOnPowerOffChanged(m_MuteOnPowerOff);
    }
    return true;
}

bool V4LRadio::setVolumeZeroOnPowerOff(bool a)
{
    if (a != m_VolumeZeroOnPowerOff) {
        m_VolumeZeroOnPowerOff = a;
        notifyVolumeZeroOnPowerOffChanged(m_VolumeZeroOnPowerOff);
    }
    return true;
}

bool V4LRadio::setV4LVersionOverride(V4LVersion vo)
{
    if (vo != m_V4L_version_override) {
        m_V4L_version_override = vo;
        notifyV4LVersionOverrideChanged(m_V4L_version_override);
        m_caps = readV4LCaps(m_radioDev);
        notifyCapabilitiesChanged(m_caps);
        notifyDescriptionChanged(m_caps.description);
    }
    return true;
}

// PluginBase methods

void   V4LRadio::saveState (KConfigGroup &config) const
{
    PluginBase::saveState(config);

    config.writeEntry("RadioDev",         m_radioDev);

    config.writeEntry("PlaybackMixerID",      m_PlaybackMixerID);
    config.writeEntry("PlaybackMixerChannel", m_PlaybackMixerChannel);
    config.writeEntry("CaptureMixerID",       m_CaptureMixerID);
    config.writeEntry("CaptureMixerChannel",  m_CaptureMixerChannel);

    config.writeEntry("fMinOverride",     m_minFrequency);
    config.writeEntry("fMaxOverride",     m_maxFrequency);
    config.writeEntry("fLastDevMin",      m_lastMinDevFrequency);
    config.writeEntry("fLastDevMax",      m_lastMaxDevFrequency);

    config.writeEntry("defaultPlaybackVolume",  m_defaultPlaybackVolume);

    config.writeEntry("signalMinQuality", m_minQuality);

    config.writeEntry("scanStep",         m_scanStep);

    config.writeEntry("Frequency",        m_currentStation.frequency());
    config.writeEntry("Treble",           m_treble);
    config.writeEntry("Bass",             m_bass);
    config.writeEntry("Balance",          m_balance);
    config.writeEntry("DeviceVolume",     m_deviceVolume);

    config.writeEntry("PowerOn",          isPowerOn());
    config.writeEntry("UseOldV4L2Calls",  m_useOldV4L2Calls);

    config.writeEntry("ActivePlayback",                           m_ActivePlayback);
    config.writeEntry("ActivePlaybackMuteCaptureChannelPlayback", m_ActivePlaybackMuteCaptureChannelPlayback);
    config.writeEntry("MuteOnPowerOff",                           m_MuteOnPowerOff);
    config.writeEntry("RDSForceEnabled",                          m_RDSForceEnabled);
    config.writeEntry("VolumeZeroOnPowerOff",                     m_VolumeZeroOnPowerOff);

    config.writeEntry("V4LVersionOverride",                       (int)m_V4L_version_override);
    config.writeEntry("V4LVersionOverrideByKernelOnce",           (int)m_V4L_version_override_by_kernel_once);
    saveRadioDeviceID(config);

}


void   V4LRadio::restoreState (const KConfigGroup &config)
{
    BlockProfiler p("V4LRadio::restoreState");

    PluginBase::restoreState(config);

    restoreRadioDeviceID(config);

    m_V4L_version_override                = (V4LVersion)config.readEntry("V4LVersionOverride", (int)V4L_Version1);
    m_V4L_version_override_by_kernel_once = config.readEntry("V4LVersionOverrideByKernelOnce", false);
    struct utsname uname_data;
    if (uname (&uname_data) == 0) {
        int major = 0;
        int minor = 0;
        int step  = 0;
        if (sscanf(uname_data.release, "%d.%d.%d", &major, &minor, &step) == 3) {
            if (major > 2 || (major == 2 && (minor > 6 || (minor == 6 && step >= 31)))) {
                if (m_V4L_version_override == 1 && !m_V4L_version_override_by_kernel_once) {
                    m_V4L_version_override_by_kernel_once = true;
                    m_V4L_version_override                = V4L_Version2;
                    logWarning(i18n("You have selected V4L API Version 1. Starting with kernel 2.6.31, the support for V4L1 seems to be not working properly any more. I'm now once switching to V4L Version 2. You may override it later."));
                }
            }
        }
    }


    QString base_devname = "/dev/radio";

    QStringList testlist (base_devname );
    for (int i = 0; i < 9; ++i)
        testlist.append(base_devname + QString::number(i));

    QString found_devname;
    for (QList<QString>::const_iterator it = testlist.begin(); it != testlist.end(); ++it) {
        QFile f(*it);
        if (f.exists()) {
            QFileInfo info(f);
            if (info.isReadable() && info.isWritable()) {
                found_devname = *it;
                break;
            }
            else {
                if (found_devname.isNull())
                    found_devname = *it;
                logWarning(i18n("Device %1 does exist but is not readable/writable. Please check device permissions.", *it));
            }
        }
    }

    QString default_devname = found_devname.isNull() ? base_devname : found_devname;

    QString devname = config.readEntry ("RadioDev", default_devname);

    if (found_devname.isNull() && devname == default_devname) {
        logError(i18n("Could not find an accessible v4l(2) radio device."));
    }

    setRadioDevice(devname);

    QString PlaybackMixerID      = config.readEntry ("PlaybackMixerID", QString());
    QString PlaybackMixerChannel = config.readEntry ("PlaybackMixerChannel", "Line");

    QString CaptureMixerID       = config.readEntry ("CaptureMixerID",  QString());
    QString CaptureMixerChannel  = config.readEntry ("CaptureMixerChannel", "Line");

    m_ActivePlayback                           = config.readEntry("ActivePlayback", false);
    m_ActivePlaybackMuteCaptureChannelPlayback = config.readEntry("ActivePlaybackMuteCaptureChannelPlayback", true);
    m_MuteOnPowerOff                           = config.readEntry("MuteOnPowerOff", false);
    m_RDSForceEnabled                          = config.readEntry("RDSForceEnabled", false);
    m_VolumeZeroOnPowerOff                     = config.readEntry("VolumeZeroOnPowerOff", false);

    float def_min =  65.0;
    float def_max = 108.0;
    m_lastMinDevFrequency   = config.readEntry ("fLastDevMin", def_min);
    m_lastMaxDevFrequency   = config.readEntry ("fLastDevMax", def_max);
    m_minFrequency          = config.readEntry ("fMinOverride", m_lastMinDevFrequency);
    m_maxFrequency          = config.readEntry ("fMaxOverride", m_lastMaxDevFrequency);

    // precaution: if radio card behaves strange, next start we will see some broken frequency
    // ranges
    if (m_minFrequency >= m_maxFrequency) {
        m_lastMinDevFrequency = m_minFrequency = def_min;
        m_lastMaxDevFrequency = m_maxFrequency = def_max;
    }

    m_minQuality            = config.readEntry ("signalMinQuality", 0.75);
    m_scanStep              = config.readEntry ("scanStep", 0.05);
    m_defaultPlaybackVolume = config.readEntry ("defaultPlaybackVolume", 0.5);

    setPlaybackMixer(PlaybackMixerID, PlaybackMixerChannel, /* force = */ true);
    setCaptureMixer (CaptureMixerID,  CaptureMixerChannel,  /* force = */ true);
    notifyDeviceMinMaxFrequencyChanged(m_lastMinDevFrequency, m_lastMaxDevFrequency);
    notifyMinMaxFrequencyChanged(m_minFrequency, m_maxFrequency);
    notifySignalMinQualityChanged(m_SoundStreamSourceID, m_minQuality);
    notifyScanStepChanged(m_scanStep);
    notifyActivePlaybackChanged(m_ActivePlayback, m_ActivePlaybackMuteCaptureChannelPlayback);
    notifyMuteOnPowerOffChanged(m_MuteOnPowerOff);
    notifyForceRDSEnabledChanged(m_RDSForceEnabled);
    notifyVolumeZeroOnPowerOffChanged(m_VolumeZeroOnPowerOff);
    notifyV4LVersionOverrideChanged(m_V4L_version_override);

    BlockProfiler p2("V4LRadio::restoreState2");

    setFrequency(config.readEntry("Frequency", 88.0), NULL);
    m_restorePowerOn = config.readEntry ("PowerOn",   false);

    BlockProfiler p3("V4LRadio::restoreState3");

    setTreble      (m_SoundStreamSourceID, config.readEntry("Treble",       0.5));
    setBass        (m_SoundStreamSourceID, config.readEntry("Bass",         0.5));
    setBalance     (m_SoundStreamSourceID, config.readEntry("Balance",      0.0));
    setDeviceVolume(                       config.readEntry("DeviceVolume", 0.9));

    m_useOldV4L2Calls = config.readEntry("UseOldV4L2Calls",  true);

    if (isPowerOff())
        notifyPlaybackVolumeChanged(m_SoundStreamSinkID, m_defaultPlaybackVolume);
}

void V4LRadio::startPlugin()
{
    PluginBase::startPlugin();
    setPower(m_restorePowerOn);
}

ConfigPageInfo V4LRadio::createConfigurationPage()
{
    V4LRadioConfiguration *v4lconf = new V4LRadioConfiguration(NULL, m_SoundStreamSourceID);
    connectI(v4lconf);
    return ConfigPageInfo (v4lconf,
                           i18n("V4L Radio"),
                           i18n("V4L Radio Options"),
                           "kradio_v4l"
                          );
}


/*AboutPageInfo V4LRadio::createAboutPage()
{
#warning "FIXME: port about stuff"*/
/*    KAboutData aboutData("kradio",
                         NULL,
                         NULL,
                         I18N_NOOP("V4L/V4L2 Plugin for KRadio."
                                   "<P>"
                                   "Provides Support for V4L/V4L2 based Radio Cards"
                                   "<P>"),
                         0,
                         //KAboutData::License_GPL,
                         "(c) 2002-2005 Martin Witte, Klas Kalass",
                         0,
                         "http://sourceforge.net/projects/kradio",
                         0);
    aboutData.addAuthor("Martin Witte",  "", "emw-kradio@nocabal.de");
    aboutData.addAuthor("Klas Kalass",   "", "klas.kalass@gmx.de");

    return AboutPageInfo(
              new KRadioAboutWidget(aboutData, KRadioAboutWidget::AbtTabbed),
              i18n("V4L/V4L2"),
              i18n("V4L/V4L2 Plugin"),
              "kradio_v4l"
           );*/
// }

////////////////////////////////////////
// anything else

void V4LRadio::radio_init()
{
    if (isSeekRunning())
        stopSeek();

    m_caps = readV4LCaps(m_radioDev);
    notifyCapabilitiesChanged(m_caps);
    notifyDescriptionChanged(m_caps.description);

/*    m_mixer_fd = open(m_mixerDev, O_RDONLY);
    if (m_mixer_fd < 0) {
        radio_done();

        logError("V4LRadio::radio_init: " +
                 i18n("Cannot open mixer device %1", m_mixerDev));
        return;
    }
*/
    m_radio_fd = open(m_radioDev.toLocal8Bit(), O_RDONLY);
    if (m_radio_fd < 0) {
        radio_done();

        logError("V4LRadio::radio_init: " +
                 i18n("Cannot open radio device %1", m_radioDev));
        return;
    }

    readTunerInfo();
    writeAudioInfo(); // set tuner-audio config as used last time
    readAudioInfo();  // reread tuner-audio and read-only flags (e.g. stereo)

    if (m_RDS_notify)
        delete m_RDS_notify;

    m_RDS_notify = new QSocketNotifier(m_radio_fd, QSocketNotifier::Read, this);
    if (m_RDS_notify)
        QObject::connect(m_RDS_notify, SIGNAL(activated(int)), this, SLOT(slotRDSData(int)));

    // restore frequency
    FrequencyRadioStation cur = m_currentStation;
    m_currentStation.setFrequency(0); // ensure that setting will be forced
    setFrequency(cur.frequency(), &cur);

}


void V4LRadio::radio_done()
{
    if (isSeekRunning())
        stopSeek();

    if (m_radio_fd >= 0) close (m_radio_fd);
//     if (m_mixer_fd >= 0) close (m_mixer_fd);

    if (m_RDS_notify)
        delete m_RDS_notify;
    m_RDS_notify = NULL;

    m_radio_fd = -1;
//  m_mixer_fd = -1;
}





#define CAPS_NAME_LEN 127
V4LCaps V4LRadio::readV4LCaps(const QString &device) const
{
    if (device == IGNORED_INIT_DUMMY_V4LDEV_STRING) {
        return V4LCaps();
    }

    char buffer[CAPS_NAME_LEN+1];
    int r;
    int fd;

    V4LCaps v4l_caps[V4L_Version_COUNT];
    for (int i = 0; i < V4L_Version_COUNT; ++i) {
        v4l_caps[i].description = i18n("V4L Plugin (V4L%1 mode, invalid device): %2", i, device);
    }

    fd = open(device.toLocal8Bit(), O_RDONLY);

    if (fd < 0) {
        logWarning("V4LRadio::readV4LCaps: " +
                   i18n("cannot open %1", device));
        return v4l_caps[m_V4L_version_override];
    }

    video_capability caps;
    r = ioctl(fd, VIDIOCGCAP, &caps);
    if (r == 0) {
        v4l_caps[V4L_Version1].v4l_version_support[V4L_Version1] = true;
        logInfo(i18n("detected %1", V4LVersionStrings[V4L_Version1]));

        v4l_caps[V4L_Version1].hasRDS  = true;

        size_t l = sizeof(caps.name);
        l = l < CAPS_NAME_LEN ? l : CAPS_NAME_LEN;
        memcpy(buffer, caps.name, l);
        buffer[l] = 0;
        v4l_caps[V4L_Version1].description = i18n("V4L Plugin (V4L%1 mode): %2", V4L_Version1, buffer);

        v4l_caps[V4L_Version1].hasMute = false;
        v4l_caps[V4L_Version1].unsetVolume();
        v4l_caps[V4L_Version1].unsetTreble();
        v4l_caps[V4L_Version1].unsetBass();
        v4l_caps[V4L_Version1].unsetBalance();

        video_audio audiocaps;
        if (0 == ioctl(fd, VIDIOCGAUDIO, &audiocaps)) {
            if ((audiocaps.flags & VIDEO_AUDIO_MUTABLE) != 0)
                v4l_caps[V4L_Version1].hasMute = true;
            if ((audiocaps.flags & VIDEO_AUDIO_VOLUME)  != 0)
                v4l_caps[V4L_Version1].setVolume (0, 65535);
            if ((audiocaps.flags & VIDEO_AUDIO_TREBLE)  != 0)
                v4l_caps[V4L_Version1].setTreble (0, 65535);
            if ((audiocaps.flags & VIDEO_AUDIO_BASS)    != 0)
                v4l_caps[V4L_Version1].setBass   (0, 65535);
            if ((audiocaps.flags & VIDEO_AUDIO_BALANCE) != 0)
                v4l_caps[V4L_Version1].setBalance(0, 65535);

            logDebug("V4LRadio::readV4LCaps: " +
                     i18n("audio caps = %1", QString().sprintf("0x%08X", audiocaps.flags)));
            logDebug("V4L1 full caps: " + v4l_caps[V4L_Version1].getDebugDescription());
        }
    } else {
//         logError("V4LRadio::readV4LCaps: " +
//                  i18n("error reading V4L1 caps"));
    }

#ifdef HAVE_V4L2
    v4l2_capability caps2;
    r = ioctl(fd, VIDIOC_QUERYCAP, &caps2);
    if (r == 0) {
        v4l_caps[V4L_Version2].v4l_version_support[V4L_Version2] = true;
        logInfo(i18n("detected %1", V4LVersionStrings[V4L_Version2]));

        v4l_caps[V4L_Version2].hasRDS = m_RDSForceEnabled || (((~caps2.capabilities) & (V4L2_CAP_RDS_CAPTURE | V4L2_CAP_READWRITE)) == 0);

        size_t l = sizeof(caps.name);
        l = l < CAPS_NAME_LEN ? l : CAPS_NAME_LEN;
        memcpy(buffer, caps.name, l);
        buffer[l] = 0;
        v4l_caps[V4L_Version2].description = i18n("V4L Plugin (V4L%1 mode): %2", V4L_Version2, buffer);

        v4l2_queryctrl  ctrl;

        v4l_caps[V4L_Version2].hasMute = false;
        v4l_caps[V4L_Version2].unsetVolume();
        v4l_caps[V4L_Version2].unsetTreble();
        v4l_caps[V4L_Version2].unsetBass();
        v4l_caps[V4L_Version2].unsetBalance();

        ctrl.id = V4L2_CID_AUDIO_MUTE;
        if (0 == ioctl(fd, VIDIOC_QUERYCTRL, &ctrl))
            v4l_caps[V4L_Version2].hasMute = !(ctrl.flags & V4L2_CTRL_FLAG_DISABLED);
        else
            logWarning(i18n("V4L2: Querying mute control failed"));

        ctrl.id = V4L2_CID_AUDIO_VOLUME;
        if (0 == ioctl(fd, VIDIOC_QUERYCTRL, &ctrl)) {
            if (!(ctrl.flags & V4L2_CTRL_FLAG_DISABLED))
                v4l_caps[V4L_Version2].setVolume(ctrl.minimum, ctrl.maximum);
        } else {
            logWarning(i18n("V4L2: Querying volume control failed"));
        }

        ctrl.id = V4L2_CID_AUDIO_TREBLE;
        if (0 == ioctl(fd, VIDIOC_QUERYCTRL, &ctrl)) {
            if (!(ctrl.flags & V4L2_CTRL_FLAG_DISABLED))
                v4l_caps[V4L_Version2].setTreble(ctrl.minimum, ctrl.maximum);
        } else {
            logWarning(i18n("V4L2: Querying treble control failed"));
        }

        ctrl.id = V4L2_CID_AUDIO_BASS;
        if (0 == ioctl(fd, VIDIOC_QUERYCTRL, &ctrl)) {
            if (!(ctrl.flags & V4L2_CTRL_FLAG_DISABLED))
                v4l_caps[V4L_Version2].setBass(ctrl.minimum, v4l_caps[V4L_Version2].maxBass = ctrl.maximum);
        } else {
            logWarning(i18n("V4L2: Querying bass control failed"));
        }

        ctrl.id = V4L2_CID_AUDIO_BALANCE;
        if (0 == ioctl(fd, VIDIOC_QUERYCTRL, &ctrl)) {
            if (!(ctrl.flags & V4L2_CTRL_FLAG_DISABLED))
                v4l_caps[V4L_Version2].setBalance(ctrl.minimum, ctrl.maximum);
        } else {
            logWarning(i18n("V4L2: Querying balance control failed"));
        }

        logDebug(i18n("V4L2 - Version: %1, caps=%2",
                      QString().sprintf("0x%08X", caps2.version),
                      QString().sprintf("0x%08X", caps2.capabilities))
                );

        logDebug("V4L2 full caps: " + v4l_caps[V4L_Version2].getDebugDescription());

    } else {
//         logWarning(i18n("V4LRadio::readV4LCaps: Reading V4L2 caps failed"));
    }
#endif
    bool any_found = false;
    for (int i = 0; i < (int)V4L_Version_COUNT; ++i) {
        if (v4l_caps[i].v4l_version_support[i]) {
            any_found = true;
            break;
        }
    }
    if (!any_found) {
        logError(i18n("V4L not detected"));
    }

    close(fd);

    V4LCaps c;

    switch (m_V4L_version_override) {
        case V4L_Version1:
            c = v4l_caps[V4L_Version1];
            if (c.v4l_version_support[V4L_Version1]) {
                break;
            }
        case V4L_Version2:
            c = v4l_caps[V4L_Version2];
            if (c.v4l_version_support[V4L_Version2]) {
                break;
            }
        default:
            c = V4LCaps();
            break;
    }

    // transfer support information
    for (int i = 0; i < V4L_Version_COUNT; ++i) {
        c.v4l_version_support[i] = v4l_caps[i].v4l_version_support[i];
    }

    logDebug("V4L final caps: " + c.getDebugDescription());

//     logDebug(c.hasMute   ? i18n("Radio is mutable")         : i18n("Radio is not mutable"));
//     logDebug(c.hasVolume ? i18n("Radio has Volume Control") : i18n("Radio has no Volume Control"));
//     logDebug(c.hasBass   ? i18n("Radio has Bass Control")   : i18n("Radio has no Bass Control"));
//     logDebug(c.hasTreble ? i18n("Radio has Treble Control") : i18n("Radio has no Treble Control"));

    return c;
}


bool V4LRadio::readTunerInfo() const
{
    if (m_blockReadTuner) return true;

    float oldq    = m_signalQuality;
    float oldminf = m_tunercache.minF;
    float oldmaxf = m_tunercache.maxF;

    bool  newRDS  = false;


    if (!m_tunercache.valid) {
        m_tunercache.minF   = m_lastMinDevFrequency;
        m_tunercache.maxF   = m_lastMaxDevFrequency;
        m_tunercache.deltaF = 1.0/16.0;
        m_tunercache.valid  = true;
    }

    int r = 0;
    if (isPowerOn()) {

        // v4l1
        if (m_V4L_version_override == V4L_Version1 && m_caps.v4l_version_support[V4L_Version1]) {

            r = ioctl(m_radio_fd, VIDIOCGTUNER, m_tuner);

            if (r == 0) {
                if ((m_tuner->flags & VIDEO_TUNER_LOW) != 0)
                    m_tunercache.deltaF = 1.0 / 16000.0;
                m_tunercache.minF = float(m_tuner->rangelow)  * m_tunercache.deltaF;
                m_tunercache.maxF = float(m_tuner->rangehigh) * m_tunercache.deltaF;
                m_tunercache.valid = true;
                m_signalQuality = float(m_tuner->signal) / 32767.0;
                {
                    static int debug_print_count = 0;
                    if ((debug_print_count++ & 0x3F) == 0) {
                        logDebug(QString("V4L1 tuner->flags = %1").arg(QString().sprintf("%08X", m_tuner->flags)));
                    }
                }
                newRDS = m_RDSForceEnabled || (m_tuner->flags & VIDEO_TUNER_RDS_ON);

            }
        }
#ifdef HAVE_V4L2
        // v4l2
        else if (m_V4L_version_override == V4L_Version2 && m_caps.v4l_version_support[V4L_Version2]) {

            r = ioctl(m_radio_fd, VIDIOC_G_TUNER, m_tuner2);

            if (r == 0) {
                if ((m_tuner2->capability & V4L2_TUNER_CAP_LOW) != 0)
                    m_tunercache.deltaF = 1.0 / 16000.0;
                m_tunercache.minF  = float(m_tuner2->rangelow)  * m_tunercache.deltaF;
                m_tunercache.maxF  = float(m_tuner2->rangehigh) * m_tunercache.deltaF;
                m_tunercache.valid = true;
                m_signalQuality    = float(m_tuner2->signal) / 32767.0;
                newRDS             = m_RDSForceEnabled || m_caps.hasRDS;
            }
        }
#endif
        else {
            logError("V4LRadio::readTunerInfo: " +
                     i18n("don't known how to handle V4L-version %1", (int)m_V4L_version_override));
            for (int i = 0; i < V4L_Version_COUNT; ++i) {
                logDebug(i18n("%1 Support: %2", V4LVersionStrings[i], m_caps.v4l_version_support[i]));
            }
        }

        if (r != 0) {
            m_signalQuality = 0;
            newRDS          = false;
            logError("V4LRadio::readTunerInfo: " +
                     i18n("cannot get tuner info (error %1)", QString().setNum(r)));
        }
    } else {
        m_signalQuality = 0;
        newRDS          = false;
    }

    if (m_RDS_notify) {
        m_RDS_notify->setEnabled(newRDS);
    }

    // prevent loops, if noticeXYZ-method is reading my state
    m_blockReadTuner = true;

    const_cast<V4LRadio*>(this)->updateRDSState(newRDS);

    if (oldminf != m_tunercache.minF || oldmaxf != m_tunercache.maxF)
        notifyDeviceMinMaxFrequencyChanged(m_tunercache.minF, m_tunercache.maxF);
    m_lastMinDevFrequency = m_tunercache.minF;
    m_lastMaxDevFrequency = m_tunercache.maxF;

    if (  (! m_minFrequency && (oldminf != m_tunercache.minF))
       || (! m_maxFrequency && (oldmaxf != m_tunercache.maxF)))
        notifyMinMaxFrequencyChanged(getMinFrequency(), getMaxFrequency());


    if (m_signalQuality != oldq)
        notifySignalQualityChanged(m_SoundStreamSourceID, m_signalQuality);
    if ( (m_signalQuality >= m_minQuality) != (oldq >= m_minQuality))
        notifySignalQualityBoolChanged(m_SoundStreamSourceID, m_signalQuality > m_minQuality);

    m_blockReadTuner = false;

    return true;
}



#define V4L2_S_CTRL(what,val) \
 {  ctl.value = (val); \
    ctl.id    = (what); \
    /* Problem: Current V4L2 development has changed the IOCTL-IDs for VIDIOC_S_CTRL */ \
    /* => we must do "try and error" to figure out what version we should use */ \
    r = ioctl (m_radio_fd,      m_useOldV4L2Calls ? VIDIOC_S_CTRL_OLD : VIDIOC_S_CTRL, &ctl); \
    /* in case this did not work, try the other version of the call */ \
    if (r) { \
        r = ioctl (m_radio_fd, !m_useOldV4L2Calls ? VIDIOC_S_CTRL_OLD : VIDIOC_S_CTRL, &ctl); \
        if (!r) m_useOldV4L2Calls = !m_useOldV4L2Calls; \
    } \
    x = x ? x : r; \
    if (r) \
        logError(i18np("error setting %1: %2", #what, QString().setNum(r))); \
 }

#define V4L2_G_CTRL(what) \
 {    ctl.id    = (what); \
    r = ioctl (m_radio_fd, VIDIOC_G_CTRL, &ctl); \
    x = x ? x : r; \
    if (r) \
        logError(i18np("error reading %1: %2", #what, QString().setNum(r))); \
 }


bool V4LRadio::updateAudioInfo(bool write) const
{
    if (m_blockReadAudio && !write)
        return true;

    bool  oldStereo        = m_stereo;
    bool  oldMute          = m_muted;
    int   iOldDeviceVolume = m_caps.intGetVolume (m_deviceVolume);
    int   iOldTreble       = m_caps.intGetTreble (m_treble);
    int   iOldBass         = m_caps.intGetBass   (m_bass);
    int   iOldBalance      = m_caps.intGetBalance(m_balance);

    if (isPowerOn()) {
        int r = 0;
        if (m_V4L_version_override == V4L_Version1 && m_caps.v4l_version_support[V4L_Version1]) {
            m_audio->audio = 0;
            if (m_muted) m_audio->flags |=  VIDEO_AUDIO_MUTE;
            else         m_audio->flags &= ~VIDEO_AUDIO_MUTE;

            m_audio->volume  = m_caps.intGetVolume (m_deviceVolume);
            m_audio->treble  = m_caps.intGetTreble (m_treble);
            m_audio->bass    = m_caps.intGetBass   (m_bass);
            m_audio->balance = m_caps.intGetBalance(m_balance);

            m_audio->mode    = 0;
            switch (m_stereoMode) {
                case STATION_STEREO_ON:
                    m_audio->mode = VIDEO_SOUND_STEREO;
                    break;
                case STATION_STEREO_OFF:
                    m_audio->mode = VIDEO_SOUND_MONO;
                    break;
                case STATION_STEREO_DONTCARE:
                    m_audio->mode = 0;
                    break;
                default:
                    break;
            }

            r = ioctl(m_radio_fd, write ? VIDIOCSAUDIO : VIDIOCGAUDIO, m_audio);

            m_stereo = (r == 0) && ((m_audio->mode  & VIDEO_SOUND_STEREO) != 0);

            m_muted  = m_caps.hasMute &&
                       ((r != 0) || ((m_audio->flags & VIDEO_AUDIO_MUTE) != 0));

// recent v4l drivers seem to return always 0 ... grrr
//            /* Some drivers seem to set volumes to zero if they are muted.
//               Thus we do not reload them if radio is muted */
//            if (!m_muted && !write) {
//                m_deviceVolume = m_caps.hasVolume  && !r ? m_caps.floatGetVolume (m_audio->volume)  : 1;
//                m_treble       = m_caps.hasTreble  && !r ? m_caps.floatGetTreble (m_audio->treble)  : 1;
//                m_bass         = m_caps.hasBass    && !r ? m_caps.floatGetBass   (m_audio->bass)    : 1;
//                m_balance      = m_caps.hasBalance && !r ? m_caps.floatGetBalance(m_audio->balance) : 0;
//            }
        }
#ifdef HAVE_V4L2
        else if (m_V4L_version_override == V4L_Version2 && m_caps.v4l_version_support[V4L_Version2]) {
            v4l2_control   ctl;
            int x = 0;    // x stores first ioctl error
            if (write) {
                if (m_caps.hasMute)
                    V4L2_S_CTRL(V4L2_CID_AUDIO_MUTE,    m_muted);
                if (m_caps.hasTreble)
                    V4L2_S_CTRL(V4L2_CID_AUDIO_TREBLE,  m_caps.intGetTreble(m_treble));
                if (m_caps.hasBass)
                    V4L2_S_CTRL(V4L2_CID_AUDIO_BASS,    m_caps.intGetBass(m_bass));
                if (m_caps.hasBalance)
                    V4L2_S_CTRL(V4L2_CID_AUDIO_BALANCE, m_caps.intGetBalance(m_balance));
                if (m_caps.hasVolume)
                    V4L2_S_CTRL(V4L2_CID_AUDIO_VOLUME,  m_caps.intGetVolume(m_deviceVolume));

                m_tuner2->audmode = 0;
                switch (m_stereoMode) {
                    case STATION_STEREO_ON:
                        m_tuner2->audmode = V4L2_TUNER_MODE_MONO;
                        break;
                    case STATION_STEREO_OFF:
                        m_tuner2->audmode = V4L2_TUNER_MODE_STEREO;
                        break;
                    case STATION_STEREO_DONTCARE:
                        m_tuner2->audmode = 0;
                        break;
                    default:
                        break;
                }

                r = ioctl( m_radio_fd, VIDIOC_S_TUNER, m_tuner2);
                x = x ? x : r;

            } else {
                if (m_caps.hasMute)
                    V4L2_G_CTRL(V4L2_CID_AUDIO_MUTE);
                m_muted   = m_caps.hasMute && ((r != 0) || ctl.value);

                /* Some drivers seem to set volumes to zero if they are muted.
                   Thus we do not reload them if radio is muted */
                if (!m_muted) {
                    if (m_caps.hasVolume)
                        V4L2_G_CTRL(V4L2_CID_AUDIO_VOLUME);
                    m_deviceVolume = m_caps.hasVolume && !r ? m_caps.floatGetVolume (ctl.value) : 1;
                    if (m_caps.hasTreble)
                        V4L2_G_CTRL(V4L2_CID_AUDIO_TREBLE);
                    m_treble       = m_caps.hasTreble && !r ? m_caps.floatGetTreble (ctl.value) : 1;
                    if (m_caps.hasBass)
                        V4L2_G_CTRL(V4L2_CID_AUDIO_BASS);
                    m_bass         = m_caps.hasBass   && !r ? m_caps.floatGetBass   (ctl.value) : 1;
                    if (m_caps.hasBalance)
                        V4L2_G_CTRL(V4L2_CID_AUDIO_BALANCE);
                    m_balance      = m_caps.hasBalance&& !r ? m_caps.floatGetBalance(ctl.value) : 0;
                }

                r = ioctl (m_radio_fd, VIDIOC_G_TUNER, m_tuner2);
                m_stereo = (r == 0) && ((m_tuner2->rxsubchans & V4L2_TUNER_SUB_STEREO) != 0);
                x = x ? x : r;
            }
            r = x;  // store first error back to r, used below for error message
        }
#endif
        else  {
            logError("V4LRadio::updateAudioInfo: " +
                     i18n("don't known how to handle V4L-version %1", m_V4L_version_override));
            for (int i = 0; i < V4L_Version_COUNT; ++i) {
                logDebug(i18n("%1 Support: %2", V4LVersionStrings[i], m_caps.v4l_version_support[i]));
            }
        }

        if (r) {
            logError("V4LRadio::updateAudioInfo: " +
                     i18n("error updating radio audio info (%1): %2",
                          write ? i18n("write") : i18n("read"),
                          QString().setNum(r)));
            return false;
        }
    }

    // prevent loops, if noticeXYZ-method is reading my state
    bool oldBlock = m_blockReadAudio;
    m_blockReadAudio = true;

    // send notifications

    if (oldStereo != m_stereo)
        notifyStereoChanged(m_SoundStreamSourceID, m_stereo);
    if (oldMute != m_muted)
        notifySourceMuted(m_SoundStreamSourceID, m_muted);
    if (iOldDeviceVolume != m_caps.intGetVolume(m_deviceVolume))
        notifyDeviceVolumeChanged(m_deviceVolume);
    if (iOldTreble       != m_caps.intGetTreble(m_treble))
        notifyTrebleChanged(m_SoundStreamSourceID, m_treble);
    if (iOldBass         != m_caps.intGetBass(m_bass))
        notifyBassChanged(m_SoundStreamSourceID, m_bass);
    if (iOldBalance      != m_caps.intGetBalance(m_balance))
        notifyBalanceChanged(m_SoundStreamSourceID, m_balance);

    m_blockReadAudio = oldBlock;

    return isPowerOn();
}




void V4LRadio::poll()
{
    // logDebug("V4LRadio::poll");
    BlockProfiler p("V4LRadio::poll");
    readTunerInfo();
    readAudioInfo();
}


bool V4LRadio::setPlaybackVolume(SoundStreamID id, float volume)
{
    if (isPowerOff() && id == m_SoundStreamSinkID) {
        m_defaultPlaybackVolume = min(max(volume, 0.0), 1.0);
        return true;
    } else {
        return false;
    }
}

bool V4LRadio::getPlaybackVolume(SoundStreamID id, float &volume) const
{
    if (isPowerOff() && id == m_SoundStreamSinkID) {
        volume = m_defaultPlaybackVolume;
        return true;
    } else {
        return false;
    }
}



bool V4LRadio::getSoundStreamDescription(SoundStreamID id, QString &descr) const
{
    if (id == m_SoundStreamSourceID) {
        descr = name() + " - " + m_currentStation.name();
        return true;
    }
    else {
        return false;
    }
}


bool V4LRadio::getSoundStreamRadioStation(SoundStreamID id, const RadioStation *&rs) const
{
    if (id == m_SoundStreamSourceID) {
        rs = &m_currentStation;
        return true;
    }
    else {
        return false;
    }
}


bool V4LRadio::enumerateSourceSoundStreams(QMap<QString, SoundStreamID> &list) const
{
    if (m_SoundStreamSourceID.isValid()) {
        QString tmp = QString();
        getSoundStreamDescription(m_SoundStreamSourceID, tmp);
        list[tmp] = m_SoundStreamSourceID;
        return true;
    }
    return false;
}



bool V4LRadio::noticeSoundStreamSinkRedirected(SoundStreamID oldID, SoundStreamID newID)
{
    if (oldID == m_SoundStreamSinkID) {
        m_SoundStreamSinkID = newID;
        notifyCurrentSoundStreamSinkIDChanged(m_SoundStreamSinkID);
        return true;
    } else {
        return false;
    }
}

bool V4LRadio::noticeSoundStreamSourceRedirected(SoundStreamID oldID, SoundStreamID newID)
{
    if (oldID == m_SoundStreamSourceID) {
        m_SoundStreamSourceID = newID;
        notifyCurrentSoundStreamSourceIDChanged(m_SoundStreamSourceID);
        return true;
    } else {
        return false;
    }
}


// bool V4LRadio::stopCapture(SoundStreamID id)
// {
//     if (id.isValid() && id == m_SoundStreamID && m_ActivePlayback) {
//         sendStopPlayback(id);
//         return true;
//     }
//     return false;
// }



const FrequencyRadioStation *V4LRadio::findMatchingStation(const StationList &sl) const
{
    for (StationList::const_iterator it = sl.begin(); it != sl.end(); ++it) {
        const FrequencyRadioStation *frs = dynamic_cast<const FrequencyRadioStation *>(*it);
        if (frs && frs->frequencyMatches(m_currentStation)) {
            return frs;
        }
    }
    return NULL;
}







void V4LRadio::updateRDSState      (bool enabled)
{
    if (enabled != m_RDS_visible) {
        m_RDS_visible = enabled;
        notifyRDSStateChanged(m_RDS_visible);
    }
}

void V4LRadio::updateRDSStationName(const QString &s)
{
    if (m_RDS_StationName != s) {
        m_RDS_StationName = s;
        notifyRDSStationNameChanged(m_RDS_StationName);
    }
}

void V4LRadio::updateRDSRadioText  (const QString &s)
{
    if (m_RDS_RadioText != s) {
        m_RDS_RadioText = s;
        notifyRDSRadioTextChanged(m_RDS_RadioText);
    }
}

void  V4LRadio::slotRDSData(int socket)
{
    unsigned char buf[3];
    size_t r = read(socket, buf, 3);
    if (r != 3) {
        m_RDS_notify->setEnabled(false);
        logWarning(i18n("error reading RDS data\n"));
    }
    else {
        processRDSData(buf);
    }
}

void  V4LRadio::processRDSData(unsigned char buf[3])
{
    m_RDS_decoder.addRawData(buf, 3);

    if (++m_RDS_errorRate_subsample_counter >= RDS_ERROR_RATE_SUBSAMPLING_RATE) {
        m_RDS_errorRate_subsample_counter = 0;
        double block_error_rate = m_RDS_decoder.statsBlockErrorRate();
        double group_error_rate = m_RDS_decoder.statsGroupErrorRate();
//         if (block_error_rate >= 0.1) {
            logDebug(i18n("V4LRadio::processRDSData: Block Error Rate: %1 %", block_error_rate*100));/**/
//         }
//         if (group_error_rate >= 0.1) {
            logDebug(i18n("V4LRadio::processRDSData: Group (~4 Blocks) Error Rate: %1 %", group_error_rate*100));
//         }
    }

    if (m_RDS_decoder.getStationNameDecoder()->isComplete()) {
        updateRDSStationName(m_RDS_decoder.getStationNameDecoder()->getStationName());
    }
    if (m_RDS_decoder.getRadioTextADecoder()->isComplete()) {
        updateRDSRadioText  (m_RDS_decoder.getRadioTextADecoder()->getRadioText());
    }
    if (m_RDS_decoder.getRadioTextBDecoder()->isComplete()) {
        updateRDSRadioText  (m_RDS_decoder.getRadioTextBDecoder()->getRadioText());
    }
}





























//#include "rds-testdump.h"
//#include "rds_RADIO_C.bin.h"
//#include "rds_PYCCKOE.bin.h"
//#include "rds_PILOT.bin.h"
//#include "rds_YUMOR_FM.bin.h"

static unsigned char RDS_test_data[] = {

    // station name
    0x00, 0x00, 0x00,            // BLOCK A
    0x00, 0x00, 0x01,            // BLOCK B, Group Type 0A, Pos 0,1
    0x00, 0x00, 0x02,            // BLOCK C,
    'E',  'T',  0x03,            // BLOCK D, "TE"

    0x00, 0x00, 0x00,            // BLOCK A
    0x01, 0x00, 0x01,            // BLOCK B, Group Type 0A, Pos 2,3
    0x00, 0x00, 0x02,            // BLOCK C,
    'T',  'S',  0x03,            // BLOCK D, "ST"

    0x00, 0x00, 0x00,            // BLOCK A
    0x02, 0x00, 0x01,            // BLOCK B, Group Type 0A, Pos 4,5
    0x00, 0x00, 0x02,            // BLOCK C,
    '2',  '1',  0x03,            // BLOCK D, "12"

    0x00, 0x00, 0x00,            // BLOCK A
    0x03, 0x00, 0x01,            // BLOCK B, Group Type 0A, Pos 6,7
    0x00, 0x00, 0x02,            // BLOCK C,
    '4',  '3',  0x03,            // BLOCK D, "34"

    // radio text
    0x00, 0x00, 0x00,            // BLOCK A
    0x00, 0x20, 0x01,            // BLOCK B, Group Type 2A, Pos 0,1,2,3
    'E',  'T',  0x02,            // BLOCK C, "TE"
    'T',  'S',  0x03,            // BLOCK D, "ST"

    0x00, 0x00, 0x00,            // BLOCK A
    0x01, 0x20, 0x01,            // BLOCK B, Group Type 2A, Pos 4,5,6,7
    'R',  ' ',  0x02,            // BLOCK C, " R"
    'D',  'A',  0x03,            // BLOCK D, "AD"

    0x00, 0x00, 0x00,            // BLOCK A
    0x02, 0x20, 0x01,            // BLOCK B, Group Type 2A, Pos 8,9,10,11
    'O',  'I',  0x02,            // BLOCK C, "IO"
    'T',  ' ',  0x03,            // BLOCK D, " T"

    0x00, 0x00, 0x00,            // BLOCK A
    0x03, 0x20, 0x01,            // BLOCK B, Group Type 2A, Pos 12,13,14,15
    'X',  'E',  0x02,            // BLOCK C, "EX"
    ' ',  'T',  0x03,            // BLOCK D, "T "

    0x00, 0x00, 0x00,            // BLOCK A
    0x04, 0x20, 0x01,            // BLOCK B, Group Type 2A, Pos 16,17,18,19
    '2',  '1',  0x02,            // BLOCK C, "12"
    '4',  '3',  0x03,            // BLOCK D, "34"

    0x00, 0x00, 0x00,            // BLOCK A
    0x05, 0x20, 0x01,            // BLOCK B, Group Type 2A, Pos 20,21,22,23
    '6',  '5',  0x02,            // BLOCK C, "56"
    '8',  '7',  0x03,            // BLOCK D, "78"

    0x00, 0x00, 0x00,            // BLOCK A
    0x06, 0x20, 0x01,            // BLOCK B, Group Type 2A, Pos 24,25,26,27
    'x',  '\r', 0x02,            // BLOCK C, "\rx"
    'x',  'x',  0x03,            // BLOCK D, "xx"

    // test invalid data
    0x00, 0x00, 0x80,            // BLOCK A with error, other blocks should be ignored
    0x06, 0x20, 0x01,            // BLOCK B, Group Type 2A, Pos 24,25,26,27
    'x',  '\r', 0x02,            // BLOCK C, "\rx"
    'x',  'x',  0x03,            // BLOCK D, "xx"

    0x00, 0x00, 0x00,            // BLOCK A
    0x00, 0x30, 0x01,            // BLOCK B, Group Type 3A should be ignored
    0x00, 0x00, 0x02,            // BLOCK C
    0x00, 0x00, 0x03,            // BLOCK D

    // terminator
    0xFF, 0xFF, 0xFF
};



void V4LRadio::slotEmulateRDS()
{
    // logDebug("V4LRadio::slotEmulateRDS");
    BlockProfiler p("V4LRadio::slotEmulateRDS");

    if (isPowerOn()) {
        unsigned char buf[3];
        buf[0] = RDS_test_data[m_RDS_emulate_pos++];
        buf[1] = RDS_test_data[m_RDS_emulate_pos++];
        buf[2] = RDS_test_data[m_RDS_emulate_pos++];
        if (buf[0] == 0xFF && buf[1] == 0xFF && buf[2] == 0xFF) {
            m_RDS_emulate_pos = 0;
        }
        else {
            processRDSData(buf);
        }
    }
}








#include "v4lradio.moc"
