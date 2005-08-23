/***************************************************************************
                          v4lradio.cpp  -  description
                             -------------------
    begin                : Don Mär  8 21:57:17 CET 2001
    copyright            : (C) 2002-2005 by Ernst Martin Witte
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#ifdef HAVE_V4L2
#include "linux/videodev2.h"
#endif
#include "linux/videodev.h"
#include <linux/soundcard.h>

#include <string.h> // memcpy needed

#include <qlayout.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qvaluelist.h>

#include <kconfig.h>
#include <kiconloader.h>
#include <kdialogbase.h>
#include <kaboutdata.h>

#include "../../src/libkradio-gui/aboutwidget.h"
#include "../../src/libkradio/utils.h"
#include "v4lradio.h"
#include "v4lradio-configuration.h"

#include "../../src/libkradio/debug-profiler.h"

struct _lrvol { unsigned char l, r; short dummy; };

///////////////////////////////////////////////////////////////////////

PLUGIN_LIBRARY_FUNCTIONS(V4LRadio, "Support for V4L(2) Radio Devices");

///////////////////////////////////////////////////////////////////////

V4LRadio::V4LRadio(const QString &name)
  : PluginBase(name, i18n("Video For Linux Plugin")),
    m_treble(0.5),
    m_bass(0.5),
    m_balance(0),
    m_deviceVolume(0.9),
    m_muted(false),
    m_signalQuality(0),
    m_stereo(false),
    m_minQuality(0.75),
    m_minFrequency(0),
    m_maxFrequency(0),

    m_defaultPlaybackVolume(0.5),

    m_scanStep(0.05),

    m_radioDev("/dev/radio0"),
    m_radio_fd(-1),
    m_useOldV4L2Calls(true),
    m_pollTimer(this),

    m_blockReadTuner(false),
    m_blockReadAudio(false),

    m_SoundStreamID(createNewSoundStream(false)),
    m_PlaybackMixerID(QString::null),
    m_CaptureMixerID(QString::null),
    m_PlaybackMixerChannel(QString::null),
    m_CaptureMixerChannel(QString::null),
    m_ActivePlayback(false),
    m_restorePowerOn(false)
{
    QObject::connect (&m_pollTimer, SIGNAL(timeout()), this, SLOT(poll()));
    m_pollTimer.start(333);

    m_audio = new video_audio;
    bzero(m_audio, sizeof(video_audio));
    m_tuner = new video_tuner;
    bzero(m_tuner, sizeof(video_tuner));
#ifdef HAVE_V4L2
    m_tuner2 = new v4l2_tuner;
    bzero(m_tuner2, sizeof(v4l2_tuner));
#endif
    m_caps.version = 0;

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
    bool b = ISeekRadio::connectI(i);
    bool c = IFrequencyRadio::connectI(i);
    bool d = IV4LCfg::connectI(i);
    bool e = PluginBase::connectI(i);
    bool f = ISoundStreamClient::connectI(i);
    return a || b || c || d || e || f;
}


bool V4LRadio::disconnectI (Interface *i)
{
    bool a = IRadioDevice::disconnectI(i);
    bool b = ISeekRadio::disconnectI(i);
    bool c = IFrequencyRadio::disconnectI(i);
    bool d = IV4LCfg::disconnectI(i);
    bool e = PluginBase::disconnectI(i);
    bool f = ISoundStreamClient::disconnectI(i);
    m_seekHelper->disconnectI(i);
    return a || b || c || d || e || f;
}


void V4LRadio::noticeConnectedI (ISoundStreamServer *s, bool pointer_valid)
{
    ISoundStreamClient::noticeConnectedI(s, pointer_valid);
    if (s && pointer_valid) {
        m_seekHelper->connectI(s);

        s->register4_queryPlaybackVolume(this);
        s->register4_sendTreble(this);
        s->register4_sendBass(this);
        s->register4_sendBalance(this);
        s->register4_sendMute(this);
        s->register4_sendUnmute(this);
        s->register4_sendSignalMinQuality(this);
        s->register4_sendStereo(this);

        s->register4_queryTreble(this);
        s->register4_queryBass(this);
        s->register4_queryBalance(this);
        s->register4_querySignalQuality(this);
        s->register4_querySignalMinQuality(this);
        s->register4_queryHasGoodQuality(this);
        s->register4_queryIsStereo(this);
        s->register4_queryIsMuted(this);


        s->register4_sendPlaybackVolume(this);
        s->register4_sendCaptureVolume(this);

        s->register4_querySoundStreamDescription(this);
        s->register4_querySoundStreamRadioStation(this);
        s->register4_queryEnumerateSoundStreams(this);

        notifySoundStreamCreated(m_SoundStreamID);
    }
}

void V4LRadio::noticeConnectedSoundClient(ISoundStreamClient::thisInterface *i, bool pointer_valid)
{
    if (i && pointer_valid && i->getSoundStreamClientID() == m_PlaybackMixerID) {
        setPlaybackMixer(m_PlaybackMixerID, m_PlaybackMixerChannel);
    }
    if (i && pointer_valid && i->getSoundStreamClientID() == m_CaptureMixerID) {
        setCaptureMixer(m_CaptureMixerID, m_CaptureMixerChannel);
    }
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
            QPtrList<ISoundStreamClient> playback_mixers = queryPlaybackMixers();
            if (!playback_mixers.isEmpty())
                *playback_mixer = playback_mixers.first();
        }
    }
    if (capture_mixer) {
        *capture_mixer  = getSoundStreamClientWithID(m_CaptureMixerID);
        if (!*capture_mixer) {
            QPtrList<ISoundStreamClient> capture_mixers  = queryCaptureMixers();
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
        ISoundStreamClient *playback_mixer = NULL,
                           *capture_mixer  = NULL;

        searchMixers(&playback_mixer, &capture_mixer);

        if (playback_mixer)
            playback_mixer->preparePlayback(m_SoundStreamID, m_PlaybackMixerChannel, m_ActivePlayback, false);
        if (capture_mixer)
            capture_mixer->prepareCapture(m_SoundStreamID, m_CaptureMixerChannel);

        sendStartPlayback(m_SoundStreamID);
        float tmp_vol = 0;
        queryPlaybackVolume(m_SoundStreamID, tmp_vol);
        if (tmp_vol < 0.005)
            sendPlaybackVolume(m_SoundStreamID, m_defaultPlaybackVolume);

        if (m_ActivePlayback) {
            SoundFormat sf;
            sendStartCaptureWithFormat(m_SoundStreamID, sf, sf);
        }

        unmute(m_SoundStreamID);
        notifyPowerChanged(true);
    }

    return true;
}


bool V4LRadio::powerOff ()
{
    if (! isPowerOn())
        return true;

    queryPlaybackVolume(m_SoundStreamID, m_defaultPlaybackVolume);
    mute(m_SoundStreamID);
    radio_done();

    sendStopPlayback(m_SoundStreamID);
    sendStopCapture(m_SoundStreamID);
    closeSoundStream(m_SoundStreamID);
    m_SoundStreamID = createNewSoundStream(m_SoundStreamID, false);
    notifySoundStreamCreated(m_SoundStreamID);

    if (isPowerOff()) {
        notifyPowerChanged(false);
    }

    return true;
}


bool V4LRadio::activateStation(const RadioStation &rs)
{
    const FrequencyRadioStation *frs = dynamic_cast<const FrequencyRadioStation*>(&rs);
    if (frs == NULL)
        return false;

    if (setFrequency(frs->frequency())) {
        m_currentStation = *frs;

        if (frs->initialVolume() > 0)
            setPlaybackVolume(m_SoundStreamID, frs->initialVolume());

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


SoundStreamID V4LRadio::getSoundStreamID() const
{
    return m_SoundStreamID;
}


const RadioStation &V4LRadio::getCurrentStation() const
{
    return m_currentStation;
}


const QString &V4LRadio::getDescription() const
{
    return m_caps.description;
}


SoundStreamID V4LRadio::getCurrentSoundStreamID() const
{
    return m_SoundStreamID;
}




bool V4LRadio::setTreble (SoundStreamID id, float t)
{
    if (id != m_SoundStreamID)
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
    if (id != m_SoundStreamID)
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
    if (id != m_SoundStreamID)
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


bool V4LRadio::mute (SoundStreamID id, bool mute)
{
    if (id != m_SoundStreamID)
        return false;

    if (m_muted != mute) {
        m_muted = mute;
        bool r = writeAudioInfo();
        notifyMuted(id, m_muted);
        return r;
    }
    return false;
}


bool V4LRadio::unmute (SoundStreamID id, bool unmute)
{
    return mute(id, !unmute);
}


bool V4LRadio::setSignalMinQuality (SoundStreamID id, float mq)
{
    if (id != m_SoundStreamID)
        return false;
    if (rint(mq*100) == rint(m_minQuality*100))
        return true;

    m_minQuality = mq;
    notifySignalMinQualityChanged(id, m_minQuality);
    return true;
}


bool    V4LRadio::setStereo(SoundStreamID /*id*/, bool /*b*/)
{
    // FIXME if possible
    return false;  // we can't do that currently, not even switch stereo to mono
}




bool V4LRadio::getTreble (SoundStreamID id, float &t) const
{
    if (id != m_SoundStreamID)
        return false;

    readAudioInfo();
    t = m_treble;
    return true;
}


bool V4LRadio::getBass (SoundStreamID id, float &b) const
{
    if (id != m_SoundStreamID)
        return false;

    readAudioInfo();
    b = m_bass;
    return true;
}


bool V4LRadio::getBalance (SoundStreamID id, float &b) const
{
    if (id != m_SoundStreamID)
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
    if (id != m_SoundStreamID)
        return false;

    q = m_minQuality;
    return true;
}


bool V4LRadio::getSignalQuality(SoundStreamID id, float &q) const
{
    if (id != m_SoundStreamID)
        return false;

    readTunerInfo();
    q = m_signalQuality;
    return true;
}


bool   V4LRadio::hasGoodQuality(SoundStreamID id, bool &good) const
{
    if (id != m_SoundStreamID)
        return false;

    float q = 0;
    if (getSignalQuality(id, q))
        good = q >= m_minQuality;
    return true;
}


bool    V4LRadio::isStereo(SoundStreamID id, bool &s) const
{
    if (id != m_SoundStreamID)
        return false;

    readAudioInfo();
    s = m_stereo;
    return true;
}


bool    V4LRadio::isMuted(SoundStreamID id, bool &m) const
{
    if (id != m_SoundStreamID)
        return false;

    readAudioInfo();
    m = m_muted;
    return true;
}


// ISeekRadio

bool V4LRadio::toBeginning()
{
    setFrequency(getMinFrequency());
    return true;
}

bool V4LRadio::toEnd()
{
    setFrequency(getMaxFrequency());
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
        m_seekHelper->start(m_SoundStreamID, up ? SeekHelper::up : SeekHelper::down);
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

bool V4LRadio::setFrequency(float freq)
{
//    if (isSeekRunning())
//        stopSeek();

    if (m_currentStation.frequency() == freq) {
        return true;
    }

    float minf = getMinFrequency();
    float maxf = getMaxFrequency();

    if (isPowerOn()) {

          bool oldMute = false;
          isMuted(m_SoundStreamID, oldMute);
          if (!oldMute && !m_ActivePlayback)
            mute(m_SoundStreamID);


        if (!m_tunercache.valid) readTunerInfo();
        float         df = m_tunercache.deltaF;

          unsigned long lfreq = (unsigned long) rint(freq / df);

          if (freq > maxf || freq < minf) {
              logError("V4LRadio::setFrequency: " +
                     i18n("invalid frequency %1").arg(QString().setNum(freq)));
            if (!oldMute && !m_ActivePlayback)
                unmute(m_SoundStreamID);
            return false;
        }

        int r = -1;
        if (m_caps.version == 1) {
            r = ioctl(m_radio_fd, VIDIOCSFREQ, &lfreq);
        }
#ifdef HAVE_V4L2
        else if (m_caps.version == 2) {
            v4l2_frequency   tmp;
            tmp.tuner = 0;
            tmp.type = V4L2_TUNER_RADIO;
            tmp.frequency = lfreq;
            r = ioctl(m_radio_fd, VIDIOC_S_FREQUENCY, &tmp);
        }
#endif
        else {
            logError("V4LRadio::setFrequency: " +
                     i18n("don't known how to handle V4L-version %1")
                     .arg(m_caps.version));
        }

          if (r) {
              logError("V4LRadio::setFrequency: " +
                     i18n("error setting frequency to %1 (%2)")
                     .arg(QString().setNum(freq))
                     .arg(QString().setNum(r)));
            // unmute the old radio with the old radio station
            if (!oldMute && !m_ActivePlayback)
                unmute(m_SoundStreamID);
            return false;
        }

        // unmute this radio device, because we now have the current
        // radio station
        if (!oldMute && !m_ActivePlayback)
            unmute(m_SoundStreamID);
    }

    m_currentStation.setFrequency(freq);
    notifyFrequencyChanged(freq, &m_currentStation);
    notifyStationChanged(m_currentStation);
    notifyProgress((freq - minf) / (maxf - minf));
    notifySoundStreamChanged(m_SoundStreamID);
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


bool  V4LRadio::setPlaybackMixer(const QString &soundStreamClientID, const QString &ch)
{
    bool change = m_PlaybackMixerID != soundStreamClientID || m_PlaybackMixerChannel != ch;
    m_PlaybackMixerID = soundStreamClientID;
    m_PlaybackMixerChannel = ch;


    if (isPowerOn()) {
        queryPlaybackVolume(m_SoundStreamID, m_defaultPlaybackVolume);
        sendStopPlayback(m_SoundStreamID);
        sendReleasePlayback(m_SoundStreamID);
    }

    ISoundStreamClient *playback_mixer = NULL;
    searchMixers(&playback_mixer, NULL);
    if (playback_mixer)
        playback_mixer->preparePlayback(m_SoundStreamID, m_PlaybackMixerChannel, m_ActivePlayback, false);

    if (isPowerOn()) {
        sendStartPlayback(m_SoundStreamID);
        sendPlaybackVolume(m_SoundStreamID, m_defaultPlaybackVolume);
        if (m_ActivePlayback) {
            SoundFormat sf;
            sendStartCaptureWithFormat(m_SoundStreamID, sf, sf);
        }
    }

    if (change)
        notifyPlaybackMixerChanged(soundStreamClientID, ch);

    return true;
}


bool  V4LRadio::setCaptureMixer(const QString &soundStreamClientID, const QString &ch)
{
    bool change = m_PlaybackMixerID != soundStreamClientID || m_PlaybackMixerChannel != ch;
    m_CaptureMixerID = soundStreamClientID;
    m_CaptureMixerChannel = ch;

    bool r = false;
    SoundFormat sf;
    queryIsCaptureRunning(m_SoundStreamID, r, sf);

    float v = 0;
    if (isPowerOn() && r) {
        queryCaptureVolume(m_SoundStreamID, v);
        sendStopCapture(m_SoundStreamID);
        sendReleaseCapture(m_SoundStreamID);
    }

    ISoundStreamClient *capture_mixer  = NULL;
    searchMixers(NULL, &capture_mixer);
    if (capture_mixer)
        capture_mixer->prepareCapture(m_SoundStreamID, m_CaptureMixerChannel);

    if (isPowerOn() && r) {
        sendStartCaptureWithFormat(m_SoundStreamID, sf, sf);
        sendCaptureVolume(m_SoundStreamID, v);
    }

    if (change)
        notifyCaptureMixerChanged(soundStreamClientID, ch);

    return true;
}


V4LCaps V4LRadio::getCapabilities(QString dev) const
{
    if (dev.isNull()) {
        return m_caps;
    } else {
        return readV4LCaps(dev);
    }
}


bool V4LRadio::setActivePlayback(bool a)
{
    if (a == m_ActivePlayback)
        return true;


    if (isPowerOn()) {
        queryPlaybackVolume(m_SoundStreamID, m_defaultPlaybackVolume);
        sendStopPlayback(m_SoundStreamID);
        sendReleasePlayback(m_SoundStreamID);
        if (m_ActivePlayback) {
            sendStopCapture(m_SoundStreamID);
        }
    }

    m_ActivePlayback = a;

    ISoundStreamClient *playback_mixer = NULL;
    searchMixers(&playback_mixer, NULL);
    if (playback_mixer)
        playback_mixer->preparePlayback(m_SoundStreamID, m_PlaybackMixerChannel, m_ActivePlayback, false);

    if (isPowerOn()) {
        sendStartPlayback(m_SoundStreamID);
        sendPlaybackVolume(m_SoundStreamID, m_defaultPlaybackVolume);
        if (m_ActivePlayback) {
            SoundFormat sf;
            sendStartCaptureWithFormat(m_SoundStreamID, sf, sf);
        }
    }

    // FIXME: restart playback/capture
    notifyActivePlaybackChanged(m_ActivePlayback);

    return true;
}

bool V4LRadio::getActivePlayback() const
{
    return m_ActivePlayback;
}

// PluginBase methods

void   V4LRadio::saveState (KConfig *config) const
{
    config->setGroup(QString("v4lradio-") + name());

    config->writeEntry("RadioDev",         m_radioDev);

    config->writeEntry("PlaybackMixerID",      m_PlaybackMixerID);
    config->writeEntry("PlaybackMixerChannel", m_PlaybackMixerChannel);
    config->writeEntry("CaptureMixerID",       m_CaptureMixerID);
    config->writeEntry("CaptureMixerChannel",  m_CaptureMixerChannel);

    config->writeEntry("fMinOverride",     m_minFrequency);
    config->writeEntry("fMaxOverride",     m_maxFrequency);

    config->writeEntry("defaultPlaybackVolume",  m_defaultPlaybackVolume);

    config->writeEntry("signalMinQuality", m_minQuality);

    config->writeEntry("scanStep",         m_scanStep);

    config->writeEntry("Frequency",        m_currentStation.frequency());
    config->writeEntry("Treble",           m_treble);
    config->writeEntry("Bass",             m_bass);
    config->writeEntry("Balance",          m_balance);
    config->writeEntry("DeviceVolume",     m_deviceVolume);

    config->writeEntry("PowerOn",          isPowerOn());
    config->writeEntry("UseOldV4L2Calls",  m_useOldV4L2Calls);

    config->writeEntry("ActivePlayback",   m_ActivePlayback);
}


void   V4LRadio::restoreState (KConfig *config)
{
    BlockProfiler p("V4LRadio::restoreState");

    config->setGroup(QString("v4lradio-") + name());

    QString base_devname = "/dev/radio";

    QStringList testlist (base_devname );
    for (int i = 0; i < 9; ++i)
        testlist.append(base_devname + QString::number(i));

    QString found_devname(QString::null);
    for (QValueListConstIterator<QString> it = testlist.begin(); it != testlist.end(); ++it) {
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
                logWarning(i18n("Device %1 does exist but is not readable/writable. Please check device permissions.").arg(*it));
            }
        }
    }

    QString default_devname = found_devname.isNull() ? base_devname : found_devname;

    QString devname = config->readEntry ("RadioDev", default_devname);

    if (found_devname.isNull() && devname == default_devname) {
        logError("Could not find an accessible v4l(2) radio device.");
    }

    setRadioDevice(devname);

    QString PlaybackMixerID      = config->readEntry ("PlaybackMixerID", QString::null);
    QString PlaybackMixerChannel = config->readEntry ("PlaybackMixerChannel", "Line");

    QString CaptureMixerID       = config->readEntry ("CaptureMixerID",  QString::null);
    QString CaptureMixerChannel  = config->readEntry ("CaptureMixerChannel", "Line");

    m_ActivePlayback       = config->readBoolEntry("ActivePlayback", false);

    m_minFrequency          = config->readDoubleNumEntry ("fMinOverride", 87.0);
    m_maxFrequency          = config->readDoubleNumEntry ("fMaxOverride", 108.0);

    m_minQuality            = config->readDoubleNumEntry ("signalMinQuality", 0.75);
    m_scanStep              = config->readDoubleNumEntry ("scanStep", 0.05);
    m_defaultPlaybackVolume = config->readDoubleNumEntry ("defaultPlaybackVolume", 0.5);

    setPlaybackMixer(PlaybackMixerID, PlaybackMixerChannel);
    setCaptureMixer (CaptureMixerID, CaptureMixerChannel);
    notifyMinMaxFrequencyChanged(m_minFrequency, m_maxFrequency);
    notifySignalMinQualityChanged(m_SoundStreamID, m_minQuality);
    notifyScanStepChanged(m_scanStep);
    notifyActivePlaybackChanged(m_ActivePlayback);

    BlockProfiler p2("V4LRadio::restoreState2");

    setFrequency(config->readDoubleNumEntry("Frequency", 88));
    m_restorePowerOn = config->readBoolEntry ("PowerOn",   false);

    BlockProfiler p3("V4LRadio::restoreState3");

    setTreble      (m_SoundStreamID, config->readDoubleNumEntry("Treble",       0.5));
    setBass        (m_SoundStreamID, config->readDoubleNumEntry("Bass",         0.5));
    setBalance     (m_SoundStreamID, config->readDoubleNumEntry("Balance",      0.0));
    setDeviceVolume(                 config->readDoubleNumEntry("DeviceVolume", 0.9));

    m_useOldV4L2Calls = config->readBoolEntry("UseOldV4L2Calls",  true);

    if (isPowerOff())
        notifyPlaybackVolumeChanged(m_SoundStreamID, m_defaultPlaybackVolume);
}

void V4LRadio::startPlugin()
{
    PluginBase::startPlugin();
    setPower(m_restorePowerOn);
}

ConfigPageInfo V4LRadio::createConfigurationPage()
{
    V4LRadioConfiguration *v4lconf = new V4LRadioConfiguration(NULL, m_SoundStreamID);
    connectI(v4lconf);
    return ConfigPageInfo (v4lconf,
                           i18n("V4L Radio"),
                           i18n("V4L Radio Options"),
                           "package_utilities");
}


AboutPageInfo V4LRadio::createAboutPage()
{
    KAboutData aboutData("kradio",
                         NULL,
                         NULL,
                         I18N_NOOP("V4L/V4L2 Plugin for KRadio."
                                   "<P>"
                                   "Provides Support for V4L/V4L2 based Radio Cards"
                                   "<P>"),
                         KAboutData::License_GPL,
                         "(c) 2002-2005 Martin Witte, Klas Kalass",
                         0,
                         "http://sourceforge.net/projects/kradio",
                         0);
    aboutData.addAuthor("Martin Witte",  "", "witte@kawo1.rwth-aachen.de");
    aboutData.addAuthor("Klas Kalass",   "", "klas.kalass@gmx.de");

    return AboutPageInfo(
              new KRadioAboutWidget(aboutData, KRadioAboutWidget::AbtTabbed),
              i18n("V4L/V4L2"),
              i18n("V4L/V4L2 Plugin"),
              "package_utilities"
           );
}

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
                 i18n("Cannot open mixer device %1").arg(m_mixerDev));
        return;
    }
*/
    m_radio_fd = open(m_radioDev.ascii(), O_RDONLY);
      if (m_radio_fd < 0) {
        radio_done();

        logError("V4LRadio::radio_init: " +
                 i18n("Cannot open radio device %1").arg(m_radioDev));
        return;
    }

    readTunerInfo();
    writeAudioInfo(); // set tuner-audio config as used last time
    readAudioInfo();  // reread tuner-audio and read-only flags (e.g. stereo)

    // restore frequency
    float old = getFrequency();
    m_currentStation.setFrequency(0);
    setFrequency(old);

    // read volume level from mixer
    // FIXME: do we still need this
/*  float v = 0;
    getVolume(m_SoundStreamID, v)
    setVolume (m_SoundStreamID, v);*/
}


void V4LRadio::radio_done()
{
    if (isSeekRunning())
        stopSeek();

    if (m_radio_fd >= 0) close (m_radio_fd);
//     if (m_mixer_fd >= 0) close (m_mixer_fd);

    m_radio_fd = -1;
//  m_mixer_fd = -1;
}





#define CAPS_NAME_LEN 127
V4LCaps V4LRadio::readV4LCaps(const QString &device) const
{
    char buffer[CAPS_NAME_LEN+1];
    int r;
    int fd;

    V4LCaps c;
    c.description = device;

    fd = open(device.ascii(), O_RDONLY);

    if (fd < 0) {
        logError("V4LRadio::readV4LCaps: " +
                 i18n("cannot open %1").arg(device));
        return c;
    }

    video_capability caps;
    r = ioctl(fd, VIDIOCGCAP, &caps);
    if (r == 0) {
        c.version = 1;

        size_t l = sizeof(caps.name);
        l = l < CAPS_NAME_LEN ? l : CAPS_NAME_LEN;
        memcpy(buffer, caps.name, l);
        buffer[l] = 0;
        c.description = buffer;

        c.hasMute = false;
        c.unsetVolume();
        c.unsetTreble();
        c.unsetBass();
        c.unsetBalance();

        video_audio audiocaps;
        if (0 == ioctl(fd, VIDIOCGAUDIO, &audiocaps)) {
            logDebug("V4LRadio::readV4LCaps: " +
                     i18n("audio caps = %1").arg(QString().setNum(audiocaps.flags)));
            if ((audiocaps.flags & VIDEO_AUDIO_MUTABLE)  != 0)
                c.hasMute = true;
            if ((audiocaps.flags & VIDEO_AUDIO_VOLUME)  != 0)
                c.setVolume (0, 65535);
            if ((audiocaps.flags & VIDEO_AUDIO_TREBLE)  != 0)
                c.setTreble (0, 65535);
            if ((audiocaps.flags & VIDEO_AUDIO_BASS)    != 0)
                c.setBass   (0, 65535);
            // at least my driver has support for balance, but the bit is not set ...
            c.setBalance(0, 65535);
        }
    } else {
        logError("V4LRadio::readV4LCaps: " +
                 i18n("error reading V4L1 caps"));
    }

#ifdef HAVE_V4L2
    v4l2_capability caps2;
    r = ioctl(fd, VIDIOC_QUERYCAP, &caps2);
    if (r == 0) {
        c.version  = 2;

        logDebug(QString().sprintf("V4L2 - Version: %08X", caps2.version));

        size_t l = sizeof(caps.name);
        l = l < CAPS_NAME_LEN ? l : CAPS_NAME_LEN;
        memcpy(buffer, caps.name, l);
        buffer[l] = 0;
        // c.description = buffer;

        v4l2_queryctrl  ctrl;

        c.hasMute = false;
        c.unsetVolume();
        c.unsetTreble();
        c.unsetBass();
        c.unsetBalance();

        ctrl.id = V4L2_CID_AUDIO_MUTE;
        if (0 == ioctl(fd, VIDIOC_QUERYCTRL, &ctrl))
            c.hasMute = !(ctrl.flags & V4L2_CTRL_FLAG_DISABLED);
        else
            logError(i18n("V4L2: Querying mute control failed"));

        ctrl.id = V4L2_CID_AUDIO_VOLUME;
        if (0 == ioctl(fd, VIDIOC_QUERYCTRL, &ctrl)) {
            if (!(ctrl.flags & V4L2_CTRL_FLAG_DISABLED))
                c.setVolume(ctrl.minimum, ctrl.maximum);
        } else {
            logError(i18n("V4L2: Querying volume control failed"));
        }

        ctrl.id = V4L2_CID_AUDIO_TREBLE;
        if (0 == ioctl(fd, VIDIOC_QUERYCTRL, &ctrl)) {
            if (!(ctrl.flags & V4L2_CTRL_FLAG_DISABLED))
                c.setTreble(ctrl.minimum, ctrl.maximum);
        } else {
            logError(i18n("V4L2: Querying treble control failed"));
        }

        ctrl.id = V4L2_CID_AUDIO_BASS;
        if (0 == ioctl(fd, VIDIOC_QUERYCTRL, &ctrl)) {
            if (!(ctrl.flags & V4L2_CTRL_FLAG_DISABLED))
                c.setBass(ctrl.minimum, c.maxBass = ctrl.maximum);
        } else {
            logError(i18n("V4L2: Querying bass control failed"));
        }

        ctrl.id = V4L2_CID_AUDIO_BALANCE;
        if (0 == ioctl(fd, VIDIOC_QUERYCTRL, &ctrl)) {
            if (!(ctrl.flags & V4L2_CTRL_FLAG_DISABLED))
                c.setBalance(ctrl.minimum, ctrl.maximum);
        } else {
            logError(i18n("V4L2: Querying balance control failed"));
        }

    } else {
        logWarning(i18n("V4LRadio::readV4LCaps: Reading V4L2 caps failed"));
    }
#endif
    if (c.version > 0) {
        logInfo(i18n("V4L %1 detected").arg(c.version));
    } else {
        logError(i18n("V4L not detected"));
    }

    logInfo(c.hasMute   ? i18n("Radio is mutable")         : i18n("Radio is not mutable"));
    logInfo(c.hasVolume ? i18n("Radio has Volume Control") : i18n("Radio has no Volume Control"));
    logInfo(c.hasBass   ? i18n("Radio has Bass Control")   : i18n("Radio has no Bass Control"));
    logInfo(c.hasTreble ? i18n("Radio has Treble Control") : i18n("Radio has no Treble Control"));

    close(fd);
    return c;
}


bool V4LRadio::readTunerInfo() const
{
    if (m_blockReadTuner) return true;

    float oldq    = m_signalQuality;
    float oldminf = m_tunercache.minF;
    float oldmaxf = m_tunercache.maxF;

    if (!m_tunercache.valid) {
        m_tunercache.minF   = 87;
        m_tunercache.maxF   = 109;
        m_tunercache.deltaF = 1.0/16.0;
        m_tunercache.valid  = true;
    }

    int r = 0;
    if (isPowerOn()) {

        // v4l1
        if (m_caps.version == 1) {

            r = ioctl(m_radio_fd, VIDIOCGTUNER, m_tuner);

            if (r == 0) {
                if ((m_tuner->flags & VIDEO_TUNER_LOW) != 0)
                    m_tunercache.deltaF = 1.0 / 16000.0;
                m_tunercache.minF = float(m_tuner->rangelow)  * m_tunercache.deltaF;
                m_tunercache.maxF = float(m_tuner->rangehigh) * m_tunercache.deltaF;
                m_tunercache.valid = true;
                m_signalQuality = float(m_tuner->signal) / 32767.0;
            }
        }
#ifdef HAVE_V4L2
        // v4l2
        else if (m_caps.version == 2) {

            r = ioctl(m_radio_fd, VIDIOC_G_TUNER, m_tuner2);

            if (r == 0) {
                if ((m_tuner2->capability & V4L2_TUNER_CAP_LOW) != 0)
                    m_tunercache.deltaF = 1.0 / 16000.0;
                m_tunercache.minF = float(m_tuner2->rangelow)  * m_tunercache.deltaF;
                m_tunercache.maxF = float(m_tuner2->rangehigh) * m_tunercache.deltaF;
                m_tunercache.valid = true;
                m_signalQuality = float(m_tuner2->signal) / 32767.0;
            }
        }
#endif
        else {
            logError("V4LRadio::readTunerInfo: " +
                     i18n("don't known how to handle V4L-version %1")
                     .arg(QString().setNum(m_caps.version)));
        }

        if (r != 0) {
            m_signalQuality = 0;
            logError("V4LRadio::readTunerInfo: " +
                     i18n("cannot get tuner info (error %1)").arg(QString().setNum(r)));
        }
    } else {
        m_signalQuality = 0;
    }

    // prevent loops, if noticeXYZ-method is reading my state
    m_blockReadTuner = true;

    if (oldminf != m_tunercache.minF || oldmaxf != m_tunercache.maxF)
        notifyDeviceMinMaxFrequencyChanged(m_tunercache.minF, m_tunercache.maxF);

    if (  ! m_minFrequency && (oldminf != m_tunercache.minF)
       || ! m_maxFrequency && (oldmaxf != m_tunercache.maxF))
        notifyMinMaxFrequencyChanged(getMinFrequency(), getMaxFrequency());


    if (m_signalQuality != oldq)
        notifySignalQualityChanged(m_SoundStreamID, m_signalQuality);
    if ( (m_signalQuality >= m_minQuality) != (oldq >= m_minQuality))
        notifySignalQualityBoolChanged(m_SoundStreamID, m_signalQuality > m_minQuality);

    m_blockReadTuner = false;

    return true;
}



#define V4L2_S_CTRL(what,val) \
 {  ctl.value = (val); \
    ctl.id    = (what); \
    /* Problem: Current V4L2 development has changed the IOCTL-IDs for VIDIOC_S_CTRL */ \
    /* => we must du "try and error" to figure out what version we should use */ \
    r = ioctl (m_radio_fd,      m_useOldV4L2Calls ? VIDIOC_S_CTRL_OLD : VIDIOC_S_CTRL, &ctl); \
    /* in case this did not work, try the other version of the call */ \
    if (r) { \
        r = ioctl (m_radio_fd, !m_useOldV4L2Calls ? VIDIOC_S_CTRL_OLD : VIDIOC_S_CTRL, &ctl); \
        if (!r) m_useOldV4L2Calls = !m_useOldV4L2Calls; \
    } \
    x = x ? x : r; \
    if (r) \
        logError(i18n("error setting %1: %2").arg(#what).arg(QString().setNum(r))); \
 }

#define V4L2_G_CTRL(what) \
 {    ctl.id    = (what); \
    r = ioctl (m_radio_fd, VIDIOC_G_CTRL, &ctl); \
    x = x ? x : r; \
    if (r) \
        logError(i18n("error reading %1: %2").arg(#what).arg(QString().setNum(r))); \
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
        if (m_caps.version == 1) {
            m_audio->audio = 0;
            if (m_muted) m_audio->flags |=  VIDEO_AUDIO_MUTE;
            else         m_audio->flags &= ~VIDEO_AUDIO_MUTE;

            m_audio->volume  = m_caps.intGetVolume (m_deviceVolume);
            m_audio->treble  = m_caps.intGetTreble (m_treble);
            m_audio->bass    = m_caps.intGetBass   (m_bass);
            m_audio->balance = m_caps.intGetBalance(m_balance);

            r = ioctl(m_radio_fd, write ? VIDIOCSAUDIO : VIDIOCGAUDIO, m_audio);

            m_stereo = (r == 0) && ((m_audio->mode  & VIDEO_SOUND_STEREO) != 0);

            m_muted  = m_caps.hasMute &&
                       ((r != 0) || ((m_audio->flags & VIDEO_AUDIO_MUTE) != 0));

            /* Some drivers seem to set volumes to zero if they are muted.
               Thus we do not reload them if radio is muted */
            if (!m_muted && !write) {
                m_deviceVolume = m_caps.hasVolume  && !r ? m_caps.floatGetVolume (m_audio->volume)  : 1;
                m_treble       = m_caps.hasTreble  && !r ? m_caps.floatGetTreble (m_audio->treble)  : 1;
                m_bass         = m_caps.hasBass    && !r ? m_caps.floatGetBass   (m_audio->bass)    : 1;
                m_balance      = m_caps.hasBalance && !r ? m_caps.floatGetBalance(m_audio->balance) : 0;
            }
        }
#ifdef HAVE_V4L2
        else if (m_caps.version == 2) {
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
                     i18n("don't known how to handle V4L-version %1")
                     .arg(QString().setNum(m_caps.version)));
        }

        if (r) {
            logError("V4LRadio::updateAudioInfo: " +
                     i18n("error updating radio audio info (%1): %2")
                     .arg(write ? i18n("write") : i18n("read"))
                     .arg(QString().setNum(r)));
            return false;
        }
    }

    // prevent loops, if noticeXYZ-method is reading my state
    bool oldBlock = m_blockReadAudio;
    m_blockReadAudio = true;

    // send notifications

    if (oldStereo != m_stereo)
        notifyStereoChanged(m_SoundStreamID, m_stereo);
    if (oldMute != m_muted)
        notifyMuted(m_SoundStreamID, m_muted);
    if (iOldDeviceVolume != m_caps.intGetVolume(m_deviceVolume))
        notifyDeviceVolumeChanged(m_deviceVolume);
    if (iOldTreble       != m_caps.intGetTreble(m_treble))
        notifyTrebleChanged(m_SoundStreamID, m_treble);
    if (iOldBass         != m_caps.intGetBass(m_bass))
        notifyBassChanged(m_SoundStreamID, m_bass);
    if (iOldBalance      != m_caps.intGetBalance(m_balance))
        notifyBalanceChanged(m_SoundStreamID, m_balance);

    m_blockReadAudio = oldBlock;

    return isPowerOn();
}




void V4LRadio::poll()
{
    readTunerInfo();
    readAudioInfo();
}


bool V4LRadio::setPlaybackVolume(SoundStreamID id, float volume)
{
    if (isPowerOff() && id == m_SoundStreamID) {
        m_defaultPlaybackVolume = min(max(volume, 0.0), 1.0);
        return true;
    } else {
        return false;
    }
}

bool V4LRadio::getPlaybackVolume(SoundStreamID id, float &volume) const
{
    if (isPowerOff() && id == m_SoundStreamID) {
        volume = m_defaultPlaybackVolume;
        return true;
    } else {
        return false;
    }
}



bool V4LRadio::getSoundStreamDescription(SoundStreamID id, QString &descr) const
{
    if (id == m_SoundStreamID) {
        descr = name() + " - " + m_currentStation.name();
        return true;
    }
    else {
        return false;
    }
}


bool V4LRadio::getSoundStreamRadioStation(SoundStreamID id, const RadioStation *&rs) const
{
    if (id == m_SoundStreamID) {
        rs = &m_currentStation;
        return true;
    }
    else {
        return false;
    }
}


bool V4LRadio::enumerateSoundStreams(QMap<QString, SoundStreamID> &list) const
{
    if (m_SoundStreamID.isValid()) {
        QString tmp = QString::null;
        getSoundStreamDescription(m_SoundStreamID, tmp);
        list[tmp] = m_SoundStreamID;
        return true;
    }
    return false;
}



#include "v4lradio.moc"
