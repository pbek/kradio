/***************************************************************************
                          alsa-sound.cpp  -  description
                             -------------------
    begin                : Thu May 26 2005
    copyright            : (C) 2005 by Martin Witte
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

#include <klocale.h>
#include <kaboutdata.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>

#include <sys/soundcard.h>
#include <alsa/asoundlib.h>

#include "alsa-sound.h"
#include "alsa-sound-configuration.h"
// #include "capture-thread.h"
#include "../../src/libkradio-gui/aboutwidget.h"

///////////////////////////////////////////////////////////////////////
//// plugin library functions

PLUGIN_LIBRARY_FUNCTIONS(AlsaSoundDevice, "Advanced Linux Sound Architecture (ALSA) Support");

/////////////////////////////////////////////////////////////////////////////

struct _lrvol { unsigned char l, r; short dummy; };

AlsaSoundDevice::AlsaSoundDevice(const QString &name)
    : QObject(NULL, NULL),
      PluginBase(name, i18n("KRadio ALSA Sound Plugin")),
      m_hPlayback(NULL),
      m_hCapture(NULL),
      m_hPlaybackMixer(NULL),
      m_hCaptureMixer(NULL),
      m_PlaybackFormat(),
      m_CaptureFormat(),
      m_PlaybackCard(-1),
      m_PlaybackDevice(-1),
      m_CaptureCard(-1),
      m_CaptureDevice(-1),
      m_PlaybackLatency(50),
      m_CaptureLatency(50),
      m_PassivePlaybackStreams(),
      m_PlaybackStreamID(),
      m_CaptureStreamID(),
      m_HWBufferSize(2048),
      m_BufferSize(16384),
      m_PlaybackBuffer(m_BufferSize),
      m_CaptureBuffer(m_BufferSize),
      m_CaptureRequestCounter(0),
      m_CapturePos(0),
      m_CaptureStartTime(0),
      m_PlaybackSkipCount(0),
      m_CaptureSkipCount(0),
      m_EnablePlayback(true),
      m_EnableCapture(true)//,
//       m_captureThread(NULL)
{
    QObject::connect(&m_PlaybackPollingTimer, SIGNAL(timeout()), this, SLOT(slotPollPlayback()));
    QObject::connect(&m_CapturePollingTimer,  SIGNAL(timeout()), this, SLOT(slotPollCapture()));
}


AlsaSoundDevice::~AlsaSoundDevice()
{
    stopCapture(m_CaptureStreamID);
    stopPlayback(m_PlaybackStreamID);
    closePlaybackDevice();
    closeCaptureDevice();
    closePlaybackMixerDevice();
    closeCaptureMixerDevice();
}


bool AlsaSoundDevice::connectI(Interface *i)
{
    bool a = PluginBase::connectI(i);
    bool b = ISoundStreamClient::connectI(i);
    return a || b;
}


bool AlsaSoundDevice::disconnectI(Interface *i)
{
    bool a = PluginBase::disconnectI(i);
    bool b = ISoundStreamClient::disconnectI(i);
    return a || b;
}

void AlsaSoundDevice::noticeConnectedI (ISoundStreamServer *s, bool pointer_valid)
{
    ISoundStreamClient::noticeConnectedI(s, pointer_valid);
    if (s && pointer_valid) {
        s->register4_sendReleasePlayback(this);
        s->register4_sendReleaseCapture(this);
        s->register4_sendPlaybackVolume(this);
        s->register4_sendMute(this);
        s->register4_sendUnmute(this);
        s->register4_sendCaptureVolume(this);
        s->register4_queryPlaybackVolume(this);
        s->register4_queryCaptureVolume(this);
        s->register4_sendStartPlayback(this);
        s->register4_sendPausePlayback(this);
        s->register4_sendStopPlayback(this);
        s->register4_queryIsPlaybackRunning(this);
        s->register4_sendStartCaptureWithFormat(this);
        s->register4_sendStopCapture(this);
        s->register4_queryIsCaptureRunning(this);
        s->register4_notifySoundStreamClosed(this);
        s->register4_notifySoundStreamRedirected(this);
        s->register4_notifySoundStreamData(this);
    }
}

// PluginBase

void AlsaSoundDevice::saveState (KConfig *c) const
{
    c->setGroup(QString("alsa-sound-") + PluginBase::name());

    c->writeEntry("playback-card",   m_PlaybackCard);
    c->writeEntry("playback-device", m_PlaybackDevice);
    c->writeEntry("capture-card",    m_CaptureCard);
    c->writeEntry("capture-device",  m_CaptureDevice);
    c->writeEntry("enable-playback", m_EnablePlayback);
    c->writeEntry("enable-capture",  m_EnableCapture);
    c->writeEntry("hwbuffer-size",   m_HWBufferSize);
    c->writeEntry("buffer-size",     m_BufferSize);
    c->writeEntry("soundstreamclient-id", m_SoundStreamClientID);

    c->writeEntry("mixer-settings",  m_CaptureMixerSettings.count());
    int i = 0;
    for (QMapConstIterator<QString, AlsaConfigMixerSetting> it = m_CaptureMixerSettings.begin(); it != m_CaptureMixerSettings.end(); ++it, ++i) {

        QString prefix = QString("mixer-setting-%1-").arg(i);
        (*it).saveState(c, prefix);
    }

}


void AlsaSoundDevice::restoreState (KConfig *c)
{
    c->setGroup(QString("alsa-sound-") + PluginBase::name());

    m_EnablePlayback  = c->readBoolEntry("enable-playback",  true);
    m_EnableCapture   = c->readBoolEntry("enable-capture",   true);
    m_HWBufferSize    = c->readNumEntry ("hwbuffer-size",    2048);
    m_BufferSize      = c->readNumEntry ("buffer-size",      16384);
    int card = c->readNumEntry  ("playback-card",   0);
    int dev  = c->readNumEntry  ("playback-device", 0);
    setPlaybackDevice(card, dev);
    card = c->readNumEntry  ("capture-card",   0);
    dev  = c->readNumEntry  ("capture-device", 0);
    setCaptureDevice(card, dev);

    m_PlaybackBuffer.resize(m_BufferSize);
    m_CaptureBuffer.resize(m_BufferSize);

    setSoundStreamClientID(c->readEntry("soundstreamclient-id", getSoundStreamClientID()));

    int n = c->readNumEntry("mixer-settings",  0);
    for (int i = 0; i < n; ++i) {
        QString prefix = QString("mixer-setting-%1-").arg(i);
        AlsaConfigMixerSetting  s(c, prefix);
        m_CaptureMixerSettings.insert(s.getIDString(), s);
    }

    emit sigUpdateConfig();
}


ConfigPageInfo  AlsaSoundDevice::createConfigurationPage()
{
    AlsaSoundConfiguration *conf = new AlsaSoundConfiguration(NULL, this);
    QObject::connect(this, SIGNAL(sigUpdateConfig()), conf, SLOT(slotUpdateConfig()));
    return ConfigPageInfo (conf,
                           i18n("ALSA Sound Device Options"),
                           i18n("ALSA Sound Device Options"),
                           "kradio_alsa");
}


AboutPageInfo AlsaSoundDevice::createAboutPage()
{
/*    KAboutData aboutData("kradio",
                         NULL,
                         NULL,
                         I18N_NOOP("ALSA Sound Plugin for KRadio"),
                         KAboutData::License_GPL,
                         "(c) 2005 Martin Witte",
                         0,
                         "http://sourceforge.net/projects/kradio",
                         0);
    aboutData.addAuthor("Martin Witte",  "", "witte@kawo1.rwth-aachen.de");

    return AboutPageInfo(
              new KRadioAboutWidget(aboutData, KRadioAboutWidget::AbtTabbed),
              i18n("ALSA Sound"),
              i18n("ALSA Sound"),
              "kradio_alsa_sound"
           );
*/
    return AboutPageInfo();
}



bool AlsaSoundDevice::preparePlayback(SoundStreamID id, const QString &channel, bool active_mode, bool start_immediately)
{
    if (id.isValid()) {
        m_PlaybackStreams.insert(id, SoundStreamConfig(channel, active_mode));
        if (start_immediately)
            startPlayback(id);
        return true;
        // FIXME: what to do if stream is already playing?
    }
    return false;
}

bool AlsaSoundDevice::prepareCapture(SoundStreamID id, const QString &channel)
{
    if (id.isValid()) {
        m_CaptureStreams.insert(id, SoundStreamConfig(channel));
        return true;
        // FIXME: what to do if stream is already playing?
    }
    return false;
}

bool AlsaSoundDevice::releasePlayback(SoundStreamID id)
{
    if (id.isValid() && m_PlaybackStreams.contains(id)) {
        if (m_PlaybackStreamID == id || m_PassivePlaybackStreams.contains(id)) {
            stopPlayback(id);
        }
        m_PlaybackStreams.remove(id);
        return true;
    }
    return false;
}

bool AlsaSoundDevice::releaseCapture(SoundStreamID id)
{
    if (id.isValid() && m_CaptureStreams.contains(id)) {
        if (m_CaptureStreamID == id) {
            stopCapture(id);
        }
        m_CaptureStreams.remove(id);
        return true;
    }
    return false;
}

bool AlsaSoundDevice::supportsPlayback()   const
{
    return m_EnablePlayback;
}


bool AlsaSoundDevice::supportsCapture() const
{
    return m_EnableCapture;
}


bool AlsaSoundDevice::startPlayback(SoundStreamID id)
{
    if (id.isValid() && m_PlaybackStreams.contains(id) && m_EnablePlayback) {

        SoundStreamConfig &cfg = m_PlaybackStreams[id];

        bool ok = false;
        if (cfg.m_ActiveMode) {
            if (!m_PlaybackStreamID.isValid()) {
                m_PlaybackStreamID = id;
                ok = true;
            }
        } else {
            if (!m_PassivePlaybackStreams.contains(id))
                m_PassivePlaybackStreams.append(id);
            ok = true;
        }

        if (ok) {
            openPlaybackMixerDevice();
            if (cfg.m_Volume >= 0 && writePlaybackMixerVolume(cfg.m_Channel, cfg.m_Volume, cfg.m_Muted)) {
                notifyPlaybackVolumeChanged(id, cfg.m_Volume);
                notifyMuted(id, cfg.m_Volume);
            }
            m_PlaybackPollingTimer.start(m_PlaybackLatency);
        }

        // error handling?
        return true;
    } else {
        return false;
    }
}


bool AlsaSoundDevice::pausePlayback(SoundStreamID /*id*/)
{
    //return stopPlayback(id);
    return false;
}


bool AlsaSoundDevice::stopPlayback(SoundStreamID id)
{
    if (id.isValid() && m_PlaybackStreams.contains(id)) {

        SoundStreamConfig &cfg = m_PlaybackStreams[id];

        if (!cfg.m_ActiveMode) {
            if  (m_PassivePlaybackStreams.contains(id)) {
                float tmp = 0;
                writePlaybackMixerVolume(cfg.m_Channel, tmp, true);
                m_PassivePlaybackStreams.remove(id);
            }
        } else if (m_PlaybackStreamID == id) {
            m_PlaybackStreamID = SoundStreamID::InvalidID;
            m_PlaybackBuffer.clear();
            closePlaybackDevice();
        }

        closePlaybackMixerDevice();
        return true;
    } else {
        return false;
    }
}

bool AlsaSoundDevice::isPlaybackRunning(SoundStreamID id, bool &b) const
{
    if (id.isValid() && m_PlaybackStreamID == id || m_PassivePlaybackStreams.contains(id)) {
        b = true;
        return true;
    } else {
        return false;
    }
}

bool AlsaSoundDevice::startCaptureWithFormat(SoundStreamID      id,
                                  const SoundFormat &proposed_format,
                                  SoundFormat       &real_format,
                                  bool force_format)
{
    if (m_CaptureStreams.contains(id) && m_EnableCapture) {

        if (m_CaptureStreamID != id) {
            m_CapturePos       = 0;
            m_CaptureStartTime = time(NULL);
        }

        if (m_CaptureStreamID != id || (force_format && proposed_format != m_CaptureFormat)) {

            m_CaptureStreamID = id;
            SoundStreamConfig &cfg = m_CaptureStreams[id];

            openCaptureMixerDevice();
            selectCaptureChannel(cfg.m_Channel);
            if (cfg.m_Volume >= 0 && writeCaptureMixerVolume(cfg.m_Channel, cfg.m_Volume)) {
                notifyCaptureVolumeChanged(m_CaptureStreamID, cfg.m_Volume);
            }

            openCaptureDevice(proposed_format);

            // FIXME: error handling?
        }

        real_format = m_CaptureFormat;
        m_CaptureRequestCounter++;

//         m_captureThread = new AlsaCaptureThread(this, m_hCapture, m_CaptureFormat, 5, m_BufferSize);
//         m_captureThread->start();

        slotPollCapture();

        return true;
    } else {
        return false;
    }
}


bool AlsaSoundDevice::stopCapture(SoundStreamID id)
{
    if (id.isValid() && m_CaptureStreamID == id) {

        if (--m_CaptureRequestCounter == 0) {

//             m_captureThread->setDone();
//             if (!m_captureThread->wait(4000)) { //wait at maximum 4 seconds
//                 logError("AlsaPlugin: capture thread did not terminate. Killing it.");
//                 m_captureThread->terminate();
//                 m_captureThread->wait();
//             }

            slotPollCapture();

//             if (m_captureThread->error()) {
//                 logError(i18n("ALSA Plugin, device plughw:%1,%2: %3").arg(m_CaptureCard)
//                                                                      .arg(m_CaptureDevice)
//                                                                      .arg(i18n("unknown error")));
//             }
//
//             delete m_captureThread;
//             m_captureThread = NULL;

            m_CaptureStreamID = SoundStreamID::InvalidID;
            m_CaptureBuffer.clear();

            closeCaptureMixerDevice();
            closeCaptureDevice();
        }
        return true;
    } else {
        return false;
    }
}


bool AlsaSoundDevice::isCaptureRunning(SoundStreamID id, bool &b, SoundFormat &sf) const
{
    if (id.isValid() && m_CaptureStreamID == id) {
        b  = true;
        sf = m_CaptureFormat;
        return true;
    } else {
        return false;
    }
}


bool AlsaSoundDevice::noticeSoundStreamClosed(SoundStreamID id)
{
    bool found = false;
    if (m_PlaybackStreamID == id || m_PassivePlaybackStreams.contains(id)) {
        stopPlayback(id);
        found = true;
    }
    if (m_CaptureStreamID == id) {
        stopCapture(id);
        found = true;
    }
    m_PlaybackStreams.remove(id);
    m_CaptureStreams.remove(id);
    return found;
}


bool AlsaSoundDevice::noticeSoundStreamRedirected(SoundStreamID oldID, SoundStreamID newID)
{
    bool found = false;
    if (m_PlaybackStreams.contains(oldID)) {
        m_PlaybackStreams.insert(newID, m_PlaybackStreams[oldID]);
        if (newID != oldID)
            m_PlaybackStreams.remove(oldID);
        found = true;
    }
    if (m_CaptureStreams.contains(oldID)) {
        m_CaptureStreams.insert(newID, m_CaptureStreams[oldID]);
        if (newID != oldID)
            m_CaptureStreams.remove(oldID);
        found = true;
    }

    if (m_PlaybackStreamID == oldID)
        m_PlaybackStreamID = newID;
    if (m_CaptureStreamID == oldID)
        m_CaptureStreamID = newID;
    if (m_PassivePlaybackStreams.contains(oldID)) {
        m_PassivePlaybackStreams.remove(oldID);
        m_PassivePlaybackStreams.append(newID);
    }
    return found;
}


bool AlsaSoundDevice::noticeSoundStreamData(SoundStreamID id,
                                           const SoundFormat &format,
                                           const char *data, size_t size,
                                           const SoundMetaData &/*md*/
                                          )
{
    if (!id.isValid() || id != m_PlaybackStreamID)
        return false;

    if (!m_hPlayback) {
        openPlaybackDevice(format);
    } else if (format != m_PlaybackFormat) {
        // flush playback buffer
        size_t buffersize = 0;
        char *buffer = m_PlaybackBuffer.getData(buffersize);

        snd_pcm_writei(m_hPlayback, buffer, buffersize / m_PlaybackFormat.sampleSize());

        // if not all could be written, it must be discarded
        m_PlaybackBuffer.clear();
        closePlaybackDevice();
        openPlaybackDevice(format);
        // error handling ?
    }

    size_t n = m_PlaybackBuffer.addData(data, size);
    if (n < size) {
        m_PlaybackSkipCount += size - n;
    } else if (m_PlaybackSkipCount > 0) {
        logWarning(i18n("plughw:%1,%2: Playback buffer overflow. Skipped %3 bytes").arg(m_PlaybackCard).arg(m_PlaybackDevice).arg(QString::number(m_PlaybackSkipCount)));
        m_PlaybackSkipCount = 0;
    }
    return m_PlaybackSkipCount == 0;
}



void AlsaSoundDevice::slotPollPlayback()
{
    if (m_PlaybackStreamID.isValid()) {

        if (m_PlaybackBuffer.getFillSize() > 0 && m_hPlayback) {

            size_t   buffersize    = 0;
            int      frameSize     = m_CaptureFormat.frameSize();
            char     *buffer       = m_PlaybackBuffer.getData(buffersize);
            int      framesWritten = snd_pcm_writei(m_hPlayback, buffer, buffersize / frameSize);
            int      bytesWritten  = framesWritten * frameSize;

            if (framesWritten > 0) {
                m_PlaybackBuffer.removeData(bytesWritten);
            } else if (framesWritten == 0) {
                logError(i18n("ALSA Plugin: cannot write data for device plughw:%1,%2").arg(m_PlaybackCard).arg(m_PlaybackDevice));
            } else if (framesWritten == -EAGAIN) {
                // do nothing
            } else {
                snd_pcm_prepare(m_hPlayback);
                logWarning(i18n("ALSA Plugin: buffer underrun for device plughw:%1,%2").arg(m_PlaybackCard).arg(m_PlaybackDevice));
            }
        }

        if (m_PlaybackBuffer.getFreeSize() > m_PlaybackBuffer.getSize() / 3) {
            notifyReadyForPlaybackData(m_PlaybackStreamID, m_PlaybackBuffer.getFreeSize());
        }

        checkMixerVolume(m_PlaybackStreamID);
    }

    QValueListConstIterator<SoundStreamID> end = m_PassivePlaybackStreams.end();
    for (QValueListConstIterator<SoundStreamID> it = m_PassivePlaybackStreams.begin(); it != end; ++it)
        checkMixerVolume(*it);
}


void AlsaSoundDevice::slotPollCapture()
{
    if (m_CaptureStreamID.isValid() && m_hCapture) {

//         while (m_captureThread && m_captureThread->getAvailableReadBuffers()) {
//             QString dev = QString("alsa://plughw:%1,%2").arg(m_CaptureCard).arg(m_CaptureDevice);
//             size_t  size     = 0;
//             char   *buffer   = m_captureThread->getReadBuffer(size);
//             time_t  cur_time = time(NULL);
//             notifySoundStreamData(m_CaptureStreamID, m_CaptureFormat, buffer, size, SoundMetaData(m_CapturePos, cur_time - m_CaptureStartTime, cur_time, dev));
//             m_CapturePos += size;
//         }

        size_t bufferSize = 0;
        char  *buffer = m_CaptureBuffer.getFreeSpace(bufferSize);

        if (bufferSize) {

            size_t frameSize  = m_CaptureFormat.frameSize();
            int    framesRead = snd_pcm_readi(m_hCapture, buffer, bufferSize / frameSize);
            size_t bytesRead  = framesRead > 0 ? framesRead * frameSize : 0;

//             //BEGIN DEBUG
//             static unsigned int debug_val = 0;
//             short *debug_buf = (short*)buffer;
//             for (int i = 0; i < bytesRead / 2 / sizeof(short); ++i) {
//                 debug_buf[2*i]   = debug_val >> 10;
//                 debug_buf[2*i+1] = debug_val >> 10;
//                 ++debug_val;
//             }
//             //END DEBUG

            if (framesRead > 0) {
                m_CaptureBuffer.removeFreeSpace(bytesRead);
            } else if (framesRead == 0) {
                snd_pcm_prepare(m_hCapture);
                logError(i18n("ALSA Plugin: cannot read data from device plughw:%1,%2").arg(m_CaptureCard).arg(m_CaptureDevice));
            } else if (framesRead == -EAGAIN) {
                    // do nothing
            } else {
                snd_pcm_prepare(m_hCapture);
                logWarning(i18n("ALSA Plugin: buffer overrun for device plughw:%1,%2 (buffersize=%3, buffer=%4)").arg(m_CaptureCard).arg(m_CaptureDevice).arg(bufferSize).arg((long long unsigned)buffer));
            }

            QString dev = QString("alsa://plughw:%1,%2").arg(m_CaptureCard).arg(m_CaptureDevice);
            while (m_CaptureBuffer.getFillSize() > m_CaptureBuffer.getSize() / 3) {
                size_t size = 0;
                buffer = m_CaptureBuffer.getData(size);
                time_t cur_time = time(NULL);
                notifySoundStreamData(m_CaptureStreamID, m_CaptureFormat, buffer, size, SoundMetaData(m_CapturePos, cur_time - m_CaptureStartTime, cur_time, dev));
                m_CaptureBuffer.removeData(size);
                m_CapturePos += size;
            }
        }
    }
    if (m_CaptureStreamID.isValid())
        checkMixerVolume(m_CaptureStreamID);
}


bool AlsaSoundDevice::openPlaybackDevice(const SoundFormat &format, bool reopen)
{
    if (m_PlaybackCard < 0 || m_PlaybackDevice < 0)
        return false;

    if (m_hPlayback) {

        if (reopen) {

            closePlaybackDevice ( /* force = */ true);

        } else {

            if (format != m_PlaybackFormat)
                return false;

            return true;
        }
    } else {
        if (reopen)         // FIXME: emw: please check if this makes sense !?!?
            return true;
    }

    m_PlaybackFormat = format;

    QString dev = QString("plughw:%1,%2").arg(m_PlaybackCard).arg(m_PlaybackDevice);
    bool error = !openAlsaDevice(m_hPlayback, m_PlaybackFormat, dev.ascii(), SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK, m_PlaybackLatency);

    if (!error) {
        m_PlaybackPollingTimer.start(m_PlaybackLatency);
    } else {
        closePlaybackDevice();
    }

    m_PlaybackSkipCount = 0;

    return !error;
}


bool AlsaSoundDevice::openCaptureDevice(const SoundFormat &format, bool reopen)
{
    if (m_PlaybackCard < 0 || m_PlaybackDevice < 0)
        return false;

    if (m_hCapture) {

        if (reopen) {

            closeCaptureDevice ( /* force = */ true);

        } else {

            if (format != m_CaptureFormat)
                return false;

            return true;
        }
    } else {
        if (reopen)         // FIXME: emw: please check if this makes sense !?!?
            return true;
    }

    m_CaptureFormat = format;

    QString dev = QString("plughw:%1,%2").arg(m_CaptureCard).arg(m_CaptureDevice);
//     bool error = !openAlsaDevice(m_hCapture, m_CaptureFormat, dev.ascii(), SND_PCM_STREAM_CAPTURE, /*flags = block*/0, m_CaptureLatency);
    bool error = !openAlsaDevice(m_hCapture, m_CaptureFormat, dev.ascii(), SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK, m_CaptureLatency);

    if (!error) {
        m_CapturePollingTimer.start(m_CaptureLatency);
    } else {
        closeCaptureDevice();
    }

    m_CaptureSkipCount = 0;

    return !error;
}


bool AlsaSoundDevice::openAlsaDevice(snd_pcm_t *&alsa_handle, SoundFormat &format, const char *pcm_name, snd_pcm_stream_t stream, int flags, unsigned &latency)
{
    bool error = false;
    int dir = 0;

    snd_pcm_hw_params_t *hwparams = NULL;

    snd_pcm_hw_params_alloca(&hwparams);


    /* OPEN */

    if (!error && snd_pcm_open(&alsa_handle, pcm_name, stream, flags) < 0) {
        logError(i18n("ALSA Plugin: Error opening PCM device %1").arg(pcm_name));
        error = true;
    }

    if (!error && snd_pcm_hw_params_any(alsa_handle, hwparams) < 0) {
        logError(i18n("ALSA Plugin: Can not configure PCM device %1").arg(pcm_name));
        error = true;
    }

    /* interleaved access type */

    if (!error && snd_pcm_hw_params_set_access(alsa_handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
        logError(i18n("ALSA Plugin: Error setting access for %1").arg(pcm_name));
        error = true;
    }

    /* sample format */
    snd_pcm_format_t sample_format = snd_pcm_build_linear_format(format.m_SampleBits,
                                                                 format.m_SampleBits,
                                                                 !format.m_IsSigned,
                                                                 format.m_Endianess == BIG_ENDIAN);
    if (!error && snd_pcm_hw_params_set_format(alsa_handle, hwparams, sample_format) < 0) {
        logError(i18n("ALSA Plugin: Error setting sample format for %1").arg(pcm_name));
        error = true;
    }

    /* channels */
    if (!error && snd_pcm_hw_params_set_channels(alsa_handle, hwparams, format.m_Channels) < 0) {
        logError(i18n("ALSA Plugin: Error setting channels for %1").arg(pcm_name));
        error = true;
    }

    /* sample rate */
    int rate = format.m_SampleRate;
    if (!error && snd_pcm_hw_params_set_rate_near(alsa_handle, hwparams, &format.m_SampleRate, &dir) < 0) {
        logError(i18n("ALSA Plugin: Error setting rate for %1").arg(pcm_name));
        error = true;
    }
    if (!error && format.m_SampleRate != format.m_SampleRate) {
        logWarning(i18n("ALSA Plugin: The rate %1 Hz is not supported by your hardware %2. Using %3 Hz instead").arg(rate).arg(pcm_name).arg(format.m_SampleRate));
    }


    size_t period_size = m_HWBufferSize / format.frameSize();
    if (!error && snd_pcm_hw_params_set_period_size_near(alsa_handle, hwparams, &period_size, &dir) < 0) {
        logError(i18n("ALSA Plugin: Error setting period size for %1").arg(pcm_name));
        error = true;
    }

//     size_t buffersize_frames = m_HWBufferSize / format.frameSize();
//     int    periods           = 4;
//     //int period_size       = m_BufferSize / periods;
//
//     /* fragments */
//     if (!error && snd_pcm_hw_params_set_periods(alsa_handle, hwparams, periods, 0) < 0) {
//         logError(i18n("ALSA Plugin: Error setting periods for %1").arg(pcm_name));
//         error = true;
//     }

//     /* Set buffer size (in frames). */
//
//     snd_pcm_uframes_t exact_buffersize_frames = buffersize_frames;
//     if (!error && snd_pcm_hw_params_set_buffer_size_near(alsa_handle, hwparams, &exact_buffersize_frames) < 0) {
//         exact_buffersize_frames = 4096;
//         if (!error && snd_pcm_hw_params_set_buffer_size_near(alsa_handle, hwparams, &exact_buffersize_frames) < 0) {
//             logError(i18n("ALSA Plugin: Error setting buffersize for %1").arg(pcm_name));
//             error = true;
//         }
//     }

//     size_t exact_buffersize = exact_buffersize_frames * format.frameSize();
//     if (!error && m_HWBufferSize != exact_buffersize) {
//         logWarning(i18n("ALSA Plugin: Hardware %1 does not support buffer size of %2. Using buffer size of %3 instead.").arg(pcm_name).arg(m_HWBufferSize).arg(exact_buffersize));
//         size_t  tmp = (((m_HWBufferSize - 1) / exact_buffersize) + 1) * exact_buffersize;
//         setHWBufferSize(tmp);
//         logInfo(i18n("ALSA Plugin: adjusted buffer size for %1 to %2 bytes").arg(pcm_name).arg(QString::number(tmp)));
//     }

    /* set all params */

    if (!error && snd_pcm_hw_params(alsa_handle, hwparams) < 0) {
        logError(i18n("ALSA Plugin: Error setting HW params"));
        error = true;
    }

    if (!error && snd_pcm_hw_params_get_period_size(hwparams, &period_size, &dir) < 0) {
        logError(i18n("ALSA Plugin: Error getting period size for %1").arg(pcm_name));
        error = true;
    }

//     latency = (exact_buffersize_frames * 1000) / format.m_SampleRate / periods; /* in milli seconds */
    latency = (period_size * format.frameSize() * 1000) / format.m_SampleRate; /* in milli seconds */

    if (!error) {
        snd_pcm_prepare(alsa_handle);
    }

    return !error;
}


bool AlsaSoundDevice::closePlaybackDevice(bool force)
{
    if (!m_PlaybackStreamID.isValid() || force) {

        if (!m_hPlaybackMixer)
            m_PlaybackPollingTimer.stop();

        if (m_hPlayback) {
            snd_pcm_drop(m_hPlayback);
            snd_pcm_close(m_hPlayback);
        }

        m_hPlayback = NULL;

        m_PlaybackBuffer.clear();
        return true;
    }
    return false;
}


bool AlsaSoundDevice::closeCaptureDevice(bool force)
{
    if (!m_CaptureStreamID.isValid() || force) {

        if (!m_hCaptureMixer)
            m_CapturePollingTimer.stop();

        if (m_hCapture) {
            snd_pcm_drop(m_hCapture);
            snd_pcm_close(m_hCapture);
        }

        m_hCapture = NULL;

        m_CaptureBuffer.clear();
        return true;
    }
    return false;
}


bool AlsaSoundDevice::openPlaybackMixerDevice(bool reopen)
{
    return openMixerDevice(m_hPlaybackMixer, m_PlaybackCard, reopen, &m_PlaybackPollingTimer, m_PlaybackLatency);
}


bool AlsaSoundDevice::openCaptureMixerDevice(bool reopen)
{
//     logDebug("AlsaSoundDevice::openCaptureMixerDevice: card == " + QString::number(m_CaptureCard));
    return openMixerDevice(m_hCaptureMixer, m_CaptureCard, reopen, &m_CapturePollingTimer, m_CaptureLatency);
}


bool AlsaSoundDevice::closePlaybackMixerDevice(bool force)
{
    return closeMixerDevice(m_hPlaybackMixer, m_PlaybackCard, m_PlaybackStreamID, m_hPlayback, force, &m_PlaybackPollingTimer);
}

bool AlsaSoundDevice::closeCaptureMixerDevice(bool force)
{
    return closeMixerDevice(m_hCaptureMixer, m_CaptureCard, m_CaptureStreamID, m_hCapture, force, &m_CapturePollingTimer);
}


static int mixer_dummy_callback(snd_mixer_t *, unsigned int /*mask*/, snd_mixer_elem_t */*elem*/)
{
    return 0;
}

bool AlsaSoundDevice::openMixerDevice(snd_mixer_t *&mixer_handle, int card, bool reopen, QTimer *timer, int timer_latency)
{
    if (reopen) {
        if (mixer_handle >= 0)
            closeMixerDevice(mixer_handle, card, SoundStreamID::InvalidID, NULL, /* force = */ true, timer);
        else
            return true;
    }

    if (!mixer_handle) {
        bool error = false;
        if (snd_mixer_open (&mixer_handle, 0) < 0) {
            staticLogError("ALSA Plugin: Error opening mixer");
            error = true;
        }
        QString cardid = "hw:" + QString::number(card);
        bool attached = false;
        if (!error && snd_mixer_attach (mixer_handle, cardid.ascii()) < 0) {
            staticLogError(i18n("ALSA Plugin: ERROR: snd_mixer_attach for card %1").arg(card));
            error = true;
        } else {
            attached = true;
        }
        if (!error && snd_mixer_selem_register(mixer_handle, NULL, NULL) < 0) {
            staticLogError(i18n("ALSA Plugin: Error: snd_mixer_selem_register for card %1").arg(card));
            error = true;
        }
        if (!error && snd_mixer_load (mixer_handle) < 0) {
            staticLogError(i18n("ALSA Plugin: Error: snd_mixer_load for card %1").arg(card));
            error = true;
        }
        snd_mixer_set_callback (mixer_handle, mixer_dummy_callback);

        if (error) {
            if (attached) {
                snd_mixer_detach(mixer_handle, cardid.ascii());
            }
            snd_mixer_close(mixer_handle);
            mixer_handle = NULL;
        }
    }

    if (mixer_handle && timer) {
        timer->start(timer_latency);
    }
    return mixer_handle != NULL;
}


bool AlsaSoundDevice::closeMixerDevice(snd_mixer_t *&mixer_handle, int card, SoundStreamID id, snd_pcm_t *pcm_handle, bool force, QTimer *timer)
{
    if (!id.isValid() || force) {

        if (!pcm_handle && timer)
            timer->stop();

        if (mixer_handle) {
            QString cardid = "hw:" + QString::number(card);
            snd_mixer_free(mixer_handle);
            snd_mixer_detach(mixer_handle, cardid.ascii());
            snd_mixer_close (mixer_handle);
        }
        mixer_handle = NULL;
    }
    return mixer_handle == NULL;
}

void AlsaSoundDevice::getPlaybackMixerChannels(
    int card,
    snd_mixer_t *__mixer_handle,
    QStringList &retval, QMap<QString, AlsaMixerElement> &ch2id)
{
    retval.clear();
    ch2id.clear();

    snd_mixer_t *mixer_handle = __mixer_handle/*m_hPlaybackMixer*/;
    bool         use_tmp_handle = false;

    if (!mixer_handle) {
        openMixerDevice(mixer_handle, card/*m_PlaybackCard*/, false, NULL, 0);
        use_tmp_handle = true;
    }

    snd_mixer_elem_t *elem = NULL;

    for (elem = snd_mixer_first_elem(mixer_handle); elem; elem = snd_mixer_elem_next(elem)) {
        AlsaMixerElement sid;
        if (!snd_mixer_selem_is_active(elem))
            continue;
        snd_mixer_selem_get_id(elem, sid);
        QString name = snd_mixer_selem_id_get_name(sid);
        int idx = snd_mixer_selem_id_get_index(sid);
        if (idx)
            name += " " + QString::number(idx);
        if (snd_mixer_selem_has_playback_volume(elem)) {
            ch2id[name] = sid;
            retval.append(name);
        }
    }

    if (use_tmp_handle) {
        closeMixerDevice(mixer_handle, card /*m_PlaybackCard*/, SoundStreamID::InvalidID, NULL, true, NULL);
    }
}

void AlsaSoundDevice::getCaptureMixerChannels(
    int card,
    snd_mixer_t *__mixer_handle,
    QStringList &vol_list, QMap<QString, AlsaMixerElement> &vol_ch2id,
    QStringList &sw_list,  QMap<QString, AlsaMixerElement> &sw_ch2id,
    QStringList *all_list
)
{
    vol_list.clear();
    sw_list.clear();
    if (all_list) all_list->clear();
    vol_ch2id.clear();
    sw_ch2id.clear();

    snd_mixer_t *mixer_handle = __mixer_handle /*m_hCaptureMixer*/;
    bool         use_tmp_handle = false;

    if (!mixer_handle) {
//         staticLogDebug("AlsaSoundDevice::getCaptureMixerChannels: card == " + QString::number(card/*m_CaptureCard*/));
        openMixerDevice(mixer_handle, card /*m_CaptureCard*/, false, NULL, 0);
        use_tmp_handle = true;
    }

    snd_mixer_elem_t *elem = NULL;

    for (elem = snd_mixer_first_elem(mixer_handle); elem; elem = snd_mixer_elem_next(elem)) {
        AlsaMixerElement sid;
        if (!snd_mixer_selem_is_active(elem))
            continue;
        snd_mixer_selem_get_id(elem, sid);
        QString name = snd_mixer_selem_id_get_name(sid);
        int idx = snd_mixer_selem_id_get_index(sid);
        if (idx)
            name += " " + QString::number(idx);

        bool add2all = false;
        if (snd_mixer_selem_has_capture_switch(elem)) {
            sw_ch2id[name] = sid;
            sw_list.append(name);
            add2all = true;
        }
        if (snd_mixer_selem_has_capture_volume(elem)) {
            vol_ch2id[name] = sid;
            vol_list.append(name);
            add2all = true;
        }
        if (add2all && all_list) {
            all_list->append(name);
        }
    }

    if (use_tmp_handle) {
        closeMixerDevice(mixer_handle, card /*m_CaptureCard*/, SoundStreamID::InvalidID, NULL, true, NULL);
    }
}

const QStringList &AlsaSoundDevice::getPlaybackChannels() const
{
    return m_PlaybackChannels;
}


const QStringList &AlsaSoundDevice::getCaptureChannels() const
{
    return m_CaptureChannelsSwitch;
}


bool AlsaSoundDevice::setPlaybackVolume(SoundStreamID id, float volume)
{
    if (id.isValid() && (m_PlaybackStreamID == id || m_PassivePlaybackStreams.contains(id))) {
        SoundStreamConfig &cfg = m_PlaybackStreams[id];

        if (rint(100*volume) != rint(100*cfg.m_Volume)) {
            if (writePlaybackMixerVolume(cfg.m_Channel, cfg.m_Volume = volume, cfg.m_Muted)) {
                notifyPlaybackVolumeChanged(id, cfg.m_Volume);
            }
        }
        return true;
    }
    return false;
}


bool AlsaSoundDevice::setCaptureVolume(SoundStreamID id, float volume)
{
    if (id.isValid() && m_CaptureStreamID == id) {
        SoundStreamConfig &cfg = m_CaptureStreams[id];

        if (rint(100*volume) != rint(100*cfg.m_Volume)) {
            if (writeCaptureMixerVolume(cfg.m_Channel, cfg.m_Volume = volume)) {
                notifyCaptureVolumeChanged(id, cfg.m_Volume);
            }
        }
        return true;
    }
    return false;
}


bool AlsaSoundDevice::getPlaybackVolume(SoundStreamID id, float &volume) const
{
    if (id.isValid() && (m_PlaybackStreamID == id || m_PassivePlaybackStreams.contains(id))) {
        const SoundStreamConfig &cfg = m_PlaybackStreams[id];
        volume = cfg.m_Volume;
        return true;
    }
    return false;
}


bool AlsaSoundDevice::getCaptureVolume(SoundStreamID id, float &volume) const
{
    if (id.isValid() && m_CaptureStreamID == id) {
        const SoundStreamConfig &cfg = m_CaptureStreams[id];
        volume = cfg.m_Volume;
        return true;
    }
    return false;
}


void AlsaSoundDevice::checkMixerVolume(SoundStreamID id)
{
    if (id.isValid()) {

        if (m_hPlaybackMixer && m_PassivePlaybackStreams.contains(id) || m_PlaybackStreamID == id) {
            snd_mixer_handle_events(m_hPlaybackMixer);
            SoundStreamConfig &cfg = m_PlaybackStreams[id];

            bool  m = false;
            float v = readPlaybackMixerVolume(cfg.m_Channel, m);
            if (rint(100*cfg.m_Volume) != rint(100*v)) {
                cfg.m_Volume = v;
                notifyPlaybackVolumeChanged(id, v);
            }
            if (m != cfg.m_Muted) {
                cfg.m_Muted = m;
                notifyMuted(id, m);
            }
        }

        if (m_hCaptureMixer && m_CaptureStreamID == id) {
            snd_mixer_handle_events(m_hCaptureMixer);
            SoundStreamConfig &cfg = m_CaptureStreams[id];

            if (m_CaptureChannels2ID.contains(cfg.m_Channel)) {
                float v = readCaptureMixerVolume(cfg.m_Channel);
                if (rint(100*cfg.m_Volume) != rint(100*v)) {
                    cfg.m_Volume = v;
                    notifyCaptureVolumeChanged(id, v);
                }
            }
        }
    }
}


float AlsaSoundDevice::readPlaybackMixerVolume(const QString &channel, bool &muted) const
{
    if (!m_hPlaybackMixer)
        return 0; // without error

    if (m_PlaybackChannels2ID.contains(channel) && m_hPlaybackMixer) {
        AlsaMixerElement sid = m_PlaybackChannels2ID[channel];
        snd_mixer_elem_t *elem = snd_mixer_find_selem(m_hPlaybackMixer, sid);
        if (elem) {
            long min = 0;
            long max = 0;
            snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
            if (min != max) {
                long val = min;

                muted = false;
                int m = false;
                if (snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_FRONT_LEFT, &m) == 0) {
                    muted = !m;
                }
                if (snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &val) == 0) {
                    return ((float)(val - min)) / (float)(max - min);
                }
            }
        }
    }
    logError("AlsaSound::readPlaybackMixerVolume: " +
             i18n("error while reading volume from hwplug:%1,%2")
             .arg(m_PlaybackCard)
             .arg(m_PlaybackDevice));
    return 0;
}


float AlsaSoundDevice::readCaptureMixerVolume(const QString &channel) const
{
    if (!m_hCaptureMixer)
        return 0; // without error

    if (m_CaptureChannels2ID.contains(channel) && m_hCaptureMixer) {
        AlsaMixerElement sid = m_CaptureChannels2ID[channel];
        snd_mixer_elem_t *elem = snd_mixer_find_selem(m_hCaptureMixer, sid);
        if (elem) {
            if (!snd_mixer_selem_has_capture_volume(elem))
                return 0;
            long min = 0;
            long max = 0;
            snd_mixer_selem_get_capture_volume_range(elem, &min, &max);
            if (min != max) {
                long val = min;
                if (snd_mixer_selem_get_capture_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &val) == 0) {
                    return ((float)(val - min)) / (float)(max - min);
                }
            }
        }
    }
    logError("AlsaSound::readCaptureMixerVolume: " +
             i18n("error while reading volume from hwplug:%1,%2")
             .arg(m_CaptureCard)
             .arg(m_CaptureDevice));
    return 0;
}


bool AlsaSoundDevice::writePlaybackMixerVolume (const QString &channel, float &vol, bool muted)
{
    if (vol > 1.0) vol = 1.0;
    if (vol < 0) vol = 0.0;

    if (!m_hPlaybackMixer)
        return false;

    if (m_PlaybackChannels2ID.contains(channel) && m_hPlaybackMixer) {
        AlsaMixerElement sid = m_PlaybackChannels2ID[channel];
        snd_mixer_elem_t *elem = snd_mixer_find_selem(m_hPlaybackMixer, sid);
        if (elem) {
            long min = 0;
            long max = 0;
            snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
            if (min != max) {
                long val = (int)rint(min + (max - min) * vol);
                vol = (float)(val - min) / (float)(max - min);
                snd_mixer_selem_set_playback_switch_all(elem, !muted);
                if (snd_mixer_selem_set_playback_volume_all(elem, val)    == 0) {
                    return true;
                }
            }
        }
    }
    logError("AlsaSound::writePlaybackMixerVolume: " +
             i18n("error while writing volume %1 to hwplug:%2,%3")
             .arg(vol)
             .arg(m_PlaybackCard)
             .arg(m_PlaybackDevice));
    return false;
}




bool AlsaSoundDevice::writeCaptureMixerVolume (const QString &channel, float &vol)
{
    if (vol > 1.0) vol = 1.0;
    if (vol < 0) vol = 0.0;

    if (!m_hCaptureMixer)
        return false;

    if (m_CaptureChannels2ID.contains(channel) && m_hCaptureMixer) {
        AlsaMixerElement sid = m_CaptureChannels2ID[channel];
        snd_mixer_elem_t *elem = snd_mixer_find_selem(m_hCaptureMixer, sid);
        if (elem) {
            long min = 0;
            long max = 0;
            snd_mixer_selem_get_capture_volume_range(elem, &min, &max);
            if (min != max) {
                long val = (int)rint(min + (max - min) * vol);
                vol = (float)(val - min) / (float)(max - min);
                if (snd_mixer_selem_set_capture_volume_all(elem, val) == 0) {
                    return true;
                }
            }
        }
    }
    logError("AlsaSound::writeCaptureMixerVolume: " +
             i18n("error while writing volume %1 to hwplug:%2,%3")
             .arg(vol)
             .arg(m_CaptureCard)
             .arg(m_CaptureDevice));
    return false;
}


bool AlsaSoundDevice::writeCaptureMixerSwitch (const QString &channel, bool capture)
{
    if (!m_hCaptureMixer)
        return false;

    if (m_CaptureChannelsSwitch2ID.contains(channel) && m_hCaptureMixer) {
        AlsaMixerElement sid = m_CaptureChannelsSwitch2ID[channel];
        snd_mixer_elem_t *elem = snd_mixer_find_selem(m_hCaptureMixer, sid);
        if (elem) {
            if (snd_mixer_selem_set_capture_switch_all(elem, capture) == 0) {
                return true;
            }
        }
    }
    logError("AlsaSound::writeCaptureMixerSwitch: " +
             i18n("error while setting capture switch for hwplug:%1,%2")
             .arg(m_CaptureCard)
             .arg(m_CaptureDevice));
    return false;
}


void AlsaSoundDevice::selectCaptureChannel (const QString &channel)
{
    writeCaptureMixerSwitch(channel, true);

    const QString ADC = "ADC";
    if (m_CaptureChannels2ID.contains(ADC)) {
        float v = readCaptureMixerVolume(ADC);
        if (rint(v*100) == 0) {
            float tmp_vol = 1.0;
            writeCaptureMixerVolume(ADC, tmp_vol);
        }
    }
    const QString Digital = "Digital";
    if (m_CaptureChannels2ID.contains(Digital)) {
        float v = readCaptureMixerVolume(Digital);
        if (rint(v*100) == 0) {
            float tmp_vol = 1.0;
            writeCaptureMixerVolume(Digital, tmp_vol);
        }
    }
    const QString WAVE = "Wave";
    if (m_CaptureChannels2ID.contains(WAVE)) {
        float x = 0;
        writeCaptureMixerVolume(WAVE, x);
    }
    const QString Capture = "Capture";
    if (m_CaptureChannelsSwitch2ID.contains(Capture)) {
        writeCaptureMixerSwitch(Capture, true);
    }

    for (QMapConstIterator<QString, AlsaConfigMixerSetting> it = m_CaptureMixerSettings.begin(); it != m_CaptureMixerSettings.end(); ++it) {
        const AlsaConfigMixerSetting &s = *it;
        if (s.m_card == m_CaptureCard && s.m_use) {
            float vol = s.m_volume;
            if (m_CaptureChannels2ID.contains(s.m_name))
                writeCaptureMixerVolume(s.m_name, vol);
            if (m_CaptureChannelsSwitch2ID.contains(s.m_name))
                writeCaptureMixerSwitch(s.m_name, s.m_active);
        }
    }
}


void AlsaSoundDevice::setHWBufferSize(int s)
{
    m_HWBufferSize = s;
}


void AlsaSoundDevice::setBufferSize(int s)
{
    m_BufferSize = s;
    m_PlaybackBuffer.resize(m_BufferSize);
    m_CaptureBuffer.resize(m_BufferSize);
}


void AlsaSoundDevice::enablePlayback(bool on)
{
    m_EnablePlayback = on;
}


void AlsaSoundDevice::enableCapture(bool on)
{
    m_EnableCapture = on;
}


void AlsaSoundDevice::setPlaybackDevice(int card, int dev)
{
    if (m_PlaybackCard == card && m_PlaybackDevice == dev)
        return;

    m_PlaybackCard   = card;
    m_PlaybackDevice = dev;
    SoundFormat f = m_PlaybackFormat;
    if (m_hPlayback)
        openPlaybackDevice(f, /* reopen = */ true);

    getPlaybackMixerChannels(m_PlaybackCard,
                             m_hPlaybackMixer,
                             m_PlaybackChannels, m_PlaybackChannels2ID);
    notifyPlaybackChannelsChanged(m_SoundStreamClientID, m_PlaybackChannels);
}


void AlsaSoundDevice::setCaptureDevice(int card, int dev)
{
//     logDebug("AlsaSoundDevice::setCaptureDevice-1: m_CaptureCard == " + QString::number(m_CaptureCard) + ", card == " + QString::number(card));
    if (m_CaptureCard == card && m_CaptureDevice == dev)
        return;
//     logDebug("AlsaSoundDevice::setCaptureDevice-2: m_CaptureCard == " + QString::number(m_CaptureCard) + ", card == " + QString::number(card));

    m_CaptureCard   = card;
    m_CaptureDevice = dev;
    SoundFormat f = m_CaptureFormat;
    if (m_hCapture)
        openCaptureDevice(f, /* reopen = */ true);

    getCaptureMixerChannels(m_CaptureCard,
                            m_hCaptureMixer,
                            m_CaptureChannels, m_CaptureChannels2ID, m_CaptureChannelsSwitch, m_CaptureChannelsSwitch2ID);
    notifyCaptureChannelsChanged(m_SoundStreamClientID,  m_CaptureChannels);
}


QString AlsaSoundDevice::getSoundStreamClientDescription() const
{
    return i18n("ALSA Sound Device %1").arg(PluginBase::name());
}


bool AlsaSoundDevice::mute (SoundStreamID id, bool mute)
{
    if (id.isValid() && (m_PlaybackStreamID == id || m_PassivePlaybackStreams.contains(id))) {
        SoundStreamConfig &cfg = m_PlaybackStreams[id];
        if (mute != cfg.m_Muted) {
            if (writePlaybackMixerVolume(cfg.m_Channel, cfg.m_Volume, cfg.m_Muted = mute)) {
                notifyMuted(id, cfg.m_Muted);
            }
        }
        return true;
    }
    return false;
}

bool AlsaSoundDevice::unmute (SoundStreamID id, bool unmute)
{
    if (id.isValid() && (m_PlaybackStreamID == id || m_PassivePlaybackStreams.contains(id))) {
        SoundStreamConfig &cfg = m_PlaybackStreams[id];
        bool mute = !unmute;
        if (mute != cfg.m_Muted) {
            if (writePlaybackMixerVolume(cfg.m_Channel, cfg.m_Volume, cfg.m_Muted = mute)) {
                notifyMuted(id, cfg.m_Muted);
            }
        }
        return true;
    }
    return false;
}

bool AlsaSoundDevice::isMuted(SoundStreamID id, bool &m) const
{
    if (id.isValid() && (m_PlaybackStreamID == id || m_PassivePlaybackStreams.contains(id))) {
        const SoundStreamConfig &cfg = m_PlaybackStreams[id];
        m = cfg.m_Muted;
        return true;
    }
    return false;
}


void AlsaSoundDevice::setCaptureMixerSettings(const QMap<QString, AlsaConfigMixerSetting> &map)
{
    m_CaptureMixerSettings = map;
}



// bool AlsaSoundDevice::event(QEvent *_e)
// {
//     bool retval = false;
//
//     switch (_e->type()) {
//
//         case CaptureTerminated :
//             retval = true;
//             break;
//
//         case CaptureStep :
//
//             slotPollCapture();
//
//             retval = true;
//             break;
//
//         case CaptureError :
//         case CaptureWarning :
//         case CaptureInfo :
//         case CaptureDebug :
//             if (m_captureThread) {
//                 AlsaCaptureEvent *e = static_cast<AlsaCaptureEvent*>(_e);
//                 QString msg = i18n("ALSA Plugin, device plughw:%1,%2: %3")
//                                 .arg(m_CaptureCard)
//                                 .arg(m_CaptureDevice)
//                                 .arg(e->message());
//                 switch (_e->type()) {
//                     case CaptureError :
//                         logError(msg);
//                         m_captureThread->resetError();
//                         break;
//                     case CaptureWarning :
//                         logWarning(msg);
//                         break;
//                     case CaptureInfo :
//                         logInfo(msg);
//                         break;
//                     case CaptureDebug :
//                         logDebug(msg);
//                         break;
//                     default:
//                         break;
//                 }
//             }
//             retval = true;
//             break;
//
//         default:
//             retval = QObject::event(_e);
//             break;
//     }
//
//     return retval;
// }








#include "alsa-sound.moc"
