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
      m_PlaybackCard(0),
      m_PlaybackDevice(0),
      m_CaptureCard(0),
      m_CaptureDevice(0),
      m_PlaybackLatency(0),
      m_CaptureLatency(0),
      m_PassivePlaybackStreams(),
      m_PlaybackStreamID(),
      m_CaptureStreamID(),
      m_BufferSize(65536),
      m_PlaybackBuffer(m_BufferSize),
      m_CaptureBuffer(m_BufferSize),
      m_CaptureRequestCounter(0),
      m_CapturePos(0),
      m_CaptureStartTime(0),
      m_PlaybackSkipCount(0),
      m_CaptureSkipCount(0),
      m_EnablePlayback(true),
      m_EnableCapture(true)
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
    c->writeEntry("buffer-size",     m_BufferSize);
    c->writeEntry("soundstreamclient-id", m_SoundStreamClientID);
}


void AlsaSoundDevice::restoreState (KConfig *c)
{
    c->setGroup(QString("alsa-sound-") + PluginBase::name());

    setSoundStreamClientID(c->readEntry("soundstreamclient-id", getSoundStreamClientID()));
    m_EnablePlayback  = c->readBoolEntry("enable-playback",  true);
    m_EnableCapture   = c->readBoolEntry("enable-capture",   true);
    m_BufferSize      = c->readNumEntry ("buffer-size",      65536);
    int card = c->readNumEntry  ("playback-card",   0);
    int dev  = c->readNumEntry  ("playback-device", 0);
    setPlaybackDevice(card, dev);
    card = c->readNumEntry  ("capture-card",   0);
    dev  = c->readNumEntry  ("capture-device", 0);
    setCaptureDevice(card, dev);

    m_PlaybackBuffer.resize(m_BufferSize);
    m_CaptureBuffer.resize(m_BufferSize);

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



bool AlsaSoundDevice::preparePlayback(SoundStreamID id, const QString &channel, bool active_mode)
{
    if (id.isValid()) {
        m_PlaybackStreams.insert(id, SoundStreamConfig(channel, active_mode));
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
            m_PlaybackPollingTimer.start(100);
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
    if (id.isValid() && m_PlaybackStreams.contains(id)) {
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

        if (m_CaptureStreamID != id || force_format) {

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

        return true;
    } else {
        return false;
    }
}


bool AlsaSoundDevice::stopCapture(SoundStreamID id)
{
    if (id.isValid() && m_CaptureStreamID == id) {

        if (--m_CaptureRequestCounter == 0) {
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


bool AlsaSoundDevice::isCaptureRunning(SoundStreamID id, bool &b) const
{
    if (id.isValid() && m_CaptureStreamID == id) {
        b = true;
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
                                           const char *data, unsigned size,
                                           const SoundMetaData &/*md*/
                                          )
{
    if (!id.isValid() || id != m_PlaybackStreamID)
        return false;

    if (!m_hPlayback) {
        openPlaybackDevice(format);
    } else if (format != m_PlaybackFormat) {
        // flush playback buffer
        unsigned buffersize = 0;
        char *buffer = m_PlaybackBuffer.getData(buffersize);

        snd_pcm_writei(m_hPlayback, buffer, buffersize / m_PlaybackFormat.sampleSize());

        // if not all could be written, it must be discarded
        m_PlaybackBuffer.clear();
        closePlaybackDevice();
        openPlaybackDevice(format);
        // error handling ?
    }

    unsigned n = m_PlaybackBuffer.addData(data, size);
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

            unsigned buffersize    = 0;
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

        unsigned bufferSize = 0;
        char *buffer = m_CaptureBuffer.getFreeSpace(bufferSize);

        if (bufferSize) {

            int frameSize  = m_CaptureFormat.frameSize();
            int framesRead = snd_pcm_readi(m_hCapture, buffer, bufferSize / frameSize);
            int bytesRead  = framesRead * frameSize;

            if (framesRead > 0) {
                m_CaptureBuffer.removeFreeSpace(bytesRead);
            } else if (framesRead == 0) {
                snd_pcm_prepare(m_hCapture);
                logError(i18n("ALSA Plugin: cannot read data from device plughw:%1,%2").arg(m_CaptureCard).arg(m_CaptureDevice));
            } else if (framesRead == -EAGAIN) {
                    // do nothing
            } else {
                snd_pcm_prepare(m_hCapture);
                logWarning(i18n("ALSA Plugin: buffer overrun for device plughw:%1,%2").arg(m_CaptureCard).arg(m_CaptureDevice));
            }

            QString dev = QString("alsa://plughw:%1,%2").arg(m_CaptureCard).arg(m_CaptureDevice);
            while (m_CaptureBuffer.getFillSize() > m_CaptureBuffer.getSize() / 3) {
                unsigned size = 0;
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
    bool error = !openAlsaDevice(m_hPlayback, m_PlaybackFormat, dev.ascii(), SND_PCM_STREAM_PLAYBACK, m_PlaybackLatency);

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
    bool error = !openAlsaDevice(m_hCapture, m_CaptureFormat, dev.ascii(), SND_PCM_STREAM_CAPTURE, m_CaptureLatency);

    if (!error) {
        m_CapturePollingTimer.start(m_CaptureLatency);
    } else {
        closeCaptureDevice();
    }

    m_CaptureSkipCount = 0;

    return !error;
}


bool AlsaSoundDevice::openAlsaDevice(snd_pcm_t *&alsa_handle, SoundFormat &format, const char *pcm_name, snd_pcm_stream_t stream, unsigned &latency)
{
    bool error = false;

    snd_pcm_hw_params_t *hwparams;

    snd_pcm_hw_params_alloca(&hwparams);

    /* OPEN */

    if (!error && snd_pcm_open(&alsa_handle, pcm_name, stream, SND_PCM_NONBLOCK) < 0) {
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

    /* sample rate */
    int rate = format.m_SampleRate;
    if (!error && snd_pcm_hw_params_set_rate_near(alsa_handle, hwparams, &format.m_SampleRate, 0) < 0) {
        logError(i18n("ALSA Plugin: Error setting rate for %1").arg(pcm_name));
        error = true;
    }
    if (!error && format.m_SampleRate != format.m_SampleRate) {
        logWarning(i18n("ALSA Plugin: The rate %1 Hz is not supported by your hardware %2. Using %3 Hz instead").arg(rate).arg(pcm_name).arg(format.m_SampleRate));
    }

    /* channels */
    if (!error && snd_pcm_hw_params_set_channels(alsa_handle, hwparams, format.m_Channels) < 0) {
        logError(i18n("ALSA Plugin: Error setting channels for %1").arg(pcm_name));
        error = true;
    }

    int buffersize_frames = m_BufferSize / format.frameSize();
    int periods           = 4;
    //int period_size       = m_BufferSize / periods;

    /* fragments */
    if (!error && snd_pcm_hw_params_set_periods(alsa_handle, hwparams, periods, 0) < 0) {
        logError(i18n("ALSA Plugin: Error setting periods for %1").arg(pcm_name));
        error = true;
    }

    /* Set buffer size (in frames). */

    snd_pcm_uframes_t exact_buffersize_frames = buffersize_frames;
    if (!error && snd_pcm_hw_params_set_buffer_size_near(alsa_handle, hwparams, &exact_buffersize_frames) < 0) {
        logError(i18n("ALSA Plugin: Error setting buffersize for %1").arg(pcm_name));
        error = true;
    }

    unsigned exact_buffersize = exact_buffersize_frames * format.frameSize();
    if (!error && m_BufferSize != exact_buffersize) {
        logWarning(i18n("ALSA Plugin: Hardware %1 does not support buffer size of %2. Using buffer size of %3 instead.").arg(pcm_name).arg(m_BufferSize).arg(exact_buffersize));
        unsigned tmp = (((m_BufferSize - 1) / exact_buffersize) + 1) * exact_buffersize;
        setBufferSize(tmp);
        logInfo(i18n("ALSA Plugin: adjusted buffer size for %1 to %2 bytes").arg(pcm_name).arg(QString::number(tmp)));
    }

    /* set all params */

    if (!error && snd_pcm_hw_params(alsa_handle, hwparams) < 0) {
        logError(i18n("ALSA Plugin: Error setting HW params"));
        error = true;
    }

    latency = (exact_buffersize_frames * 1000) / format.m_SampleRate / periods; /* in milli seconds */

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

bool AlsaSoundDevice::openMixerDevice(snd_mixer_t *&mixer_handle, int card, bool reopen, QTimer *timer, int timer_latency)  const
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
            logError("ALSA Plugin: Error opening mixer");
            error = true;
        }
        QString cardid = "hw:" + QString::number(card);
        bool attached = false;
        if (!error && snd_mixer_attach (mixer_handle, cardid.ascii()) < 0) {
            logError(i18n("ALSA Plugin: ERROR: snd_mixer_attach for card %1").arg(card));
            error = true;
        } else {
            attached = true;
        }
        if (!error && snd_mixer_selem_register(mixer_handle, NULL, NULL) < 0) {
            logError(i18n("ALSA Plugin: Error: snd_mixer_selem_register for card %1").arg(card));
            error = true;
        }
        if (!error && snd_mixer_load (mixer_handle) < 0) {
            logError(i18n("ALSA Plugin: Error: snd_mixer_load for card %1").arg(card));
            error = true;
        }
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


bool AlsaSoundDevice::closeMixerDevice(snd_mixer_t *&mixer_handle, int card, SoundStreamID id, snd_pcm_t *pcm_handle, bool force, QTimer *timer) const
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

void AlsaSoundDevice::getPlaybackMixerChannels(QStringList &retval, QMap<QString, AlsaMixerElement> &ch2id) const
{
    retval.clear();
    ch2id.clear();

    snd_mixer_t *mixer_handle = m_hPlaybackMixer;
    bool         use_tmp_handle = false;

    if (!mixer_handle) {
        openMixerDevice(mixer_handle, m_PlaybackCard, false, NULL, 0);
        use_tmp_handle = true;
    }

    snd_mixer_elem_t *elem = NULL;

    for (elem = snd_mixer_first_elem(mixer_handle); elem; elem = snd_mixer_elem_next(elem)) {
        AlsaMixerElement sid;
        if (!snd_mixer_selem_is_active(elem))
            continue;
        if (snd_mixer_selem_has_playback_volume(elem)) {
            snd_mixer_selem_get_id(elem, sid);
            const char *name = snd_mixer_selem_id_get_name(sid);
            ch2id[name] = sid;
            retval.append(name);
        }
    }

    if (use_tmp_handle) {
        closeMixerDevice(mixer_handle, m_PlaybackCard, SoundStreamID::InvalidID, NULL, true, NULL);
    }
}

void AlsaSoundDevice::getCaptureMixerChannels(QStringList &retval, QMap<QString, AlsaMixerElement> &ch2id) const
{
    retval.clear();
    ch2id.clear();

    snd_mixer_t *mixer_handle = m_hCaptureMixer;
    bool         use_tmp_handle = false;

    if (!mixer_handle) {
        openMixerDevice(mixer_handle, m_CaptureCard, false, NULL, 0);
        use_tmp_handle = true;
    }

    snd_mixer_elem_t *elem = NULL;

    for (elem = snd_mixer_first_elem(mixer_handle); elem; elem = snd_mixer_elem_next(elem)) {
        AlsaMixerElement sid;
        if (!snd_mixer_selem_is_active(elem))
            continue;
        if (snd_mixer_selem_has_capture_switch(elem)) {
            snd_mixer_selem_get_id(elem, sid);
            const char *name = snd_mixer_selem_id_get_name(sid);
            ch2id[name] = sid;
            retval.append(name);
        }
    }

    if (use_tmp_handle) {
        closeMixerDevice(mixer_handle, m_CaptureCard, SoundStreamID::InvalidID, NULL, true, NULL);
    }
}

const QStringList &AlsaSoundDevice::getPlaybackChannels() const
{
    return m_PlaybackChannels;
}


const QStringList &AlsaSoundDevice::getCaptureChannels() const
{
    return m_CaptureChannels;
}


bool AlsaSoundDevice::setPlaybackVolume(SoundStreamID id, float volume)
{
    if (id.isValid() && (m_PlaybackStreamID == id || m_PassivePlaybackStreams.contains(id))) {
        SoundStreamConfig &cfg = m_PlaybackStreams[id];

        if (rint(100*volume + 0.5) != rint(100*cfg.m_Volume + 0.5)) {
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

        if (rint(100*volume + 0.5) != rint(100*cfg.m_Volume + 0.5)) {
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
            SoundStreamConfig &cfg = m_PlaybackStreams[id];

            bool  m = false;
            float v = readPlaybackMixerVolume(cfg.m_Channel, m);
            if (rint(100*cfg.m_Volume + 0.5) != rint(100*v + 0.5)) {
                cfg.m_Volume = v;
                notifyPlaybackVolumeChanged(id, v);
            }
            if (m != cfg.m_Muted) {
                cfg.m_Muted = m;
                notifyMuted(id, m);
            }
        }

        if (m_hCaptureMixer && m_CaptureStreamID == id) {
            SoundStreamConfig &cfg = m_CaptureStreams[id];

            float v = readCaptureMixerVolume(cfg.m_Channel);
            if (rint(100*cfg.m_Volume + 0.5) != rint(100*v + 0.5)) {
                cfg.m_Volume = v;
                notifyCaptureVolumeChanged(id, v);
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
        return vol;

    if (m_PlaybackChannels2ID.contains(channel) && m_hPlaybackMixer) {
        AlsaMixerElement sid = m_PlaybackChannels2ID[channel];
        snd_mixer_elem_t *elem = snd_mixer_find_selem(m_hPlaybackMixer, sid);
        if (elem) {
            long min = 0;
            long max = 0;
            snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
            if (min != max) {
                long val = (int)rint(min + (max - min) * vol + 0.5);
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
        return vol;

    if (m_CaptureChannels2ID.contains(channel) && m_hCaptureMixer) {
        AlsaMixerElement sid = m_CaptureChannels2ID[channel];
        snd_mixer_elem_t *elem = snd_mixer_find_selem(m_hCaptureMixer, sid);
        if (elem) {
            long min = 0;
            long max = 0;
            snd_mixer_selem_get_capture_volume_range(elem, &min, &max);
            if (min != max) {
                long val = (int)rint(min + (max - min) * vol + 0.5);
                vol = (float)(val - min) / (float)(max - min);
                if (snd_mixer_selem_set_capture_volume_all(elem, val) == 0 &&
                    snd_mixer_selem_set_capture_switch_all(elem, 1)   == 0
                ) {
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


void AlsaSoundDevice::selectCaptureChannel (const QString &channel)
{
    bool ok = false;
    if (m_CaptureChannels2ID.contains(channel) && m_hCaptureMixer) {
        AlsaMixerElement sid = m_CaptureChannels2ID[channel];
        snd_mixer_elem_t *elem = snd_mixer_find_selem(m_hCaptureMixer, sid);
        if (elem) {
            if (snd_mixer_selem_set_capture_switch_all(elem, 1) == 0) {
                ok = true;
            }
        }
    }
    if (!ok) {
        logError("AlsaSound::selectCaptureChannel: " +
                 i18n("error selecting capture channel on hwplug:%1,%2")
                 .arg(m_CaptureCard)
                 .arg(m_CaptureDevice));
    }

    const QString ADC = "ADC";
    if (m_CaptureChannels2ID.contains(ADC)) {
        float v = readCaptureMixerVolume(ADC);
        if (rint(v*100 + 0.5) == 0) {
            float tmp_vol = 1.0;
            writeCaptureMixerVolume(ADC, tmp_vol);
        }
    }
    const QString Capture = "Capture";
    if (m_CaptureChannels2ID.contains(Capture) && m_hCaptureMixer) {
        ok = false;
        AlsaMixerElement sid = m_CaptureChannels2ID[Capture];
        snd_mixer_elem_t *elem = snd_mixer_find_selem(m_hCaptureMixer, sid);
        if (elem) {
            if (snd_mixer_selem_set_capture_switch_all(elem, 1) == 0) {
                ok = true;
            }
        }
        if (!ok) {
            logError("AlsaSound::selectCaptureChannel: " +
                     i18n("error enabling capture on hwplug:%1,%2")
                     .arg(m_CaptureCard)
                     .arg(m_CaptureDevice));
        }
    }
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
    m_PlaybackCard   = card;
    m_PlaybackDevice = dev;
    SoundFormat f = m_PlaybackFormat;
    if (m_hPlayback)
        openPlaybackDevice(f, /* reopen = */ true);

    getPlaybackMixerChannels(m_PlaybackChannels, m_PlaybackChannels2ID);
    notifyPlaybackChannelsChanged(m_SoundStreamClientID, m_PlaybackChannels);
}


void AlsaSoundDevice::setCaptureDevice(int card, int dev)
{
    m_CaptureCard   = card;
    m_CaptureDevice = dev;
    SoundFormat f = m_CaptureFormat;
    if (m_hCapture)
        openCaptureDevice(f, /* reopen = */ true);

    getCaptureMixerChannels(m_CaptureChannels, m_CaptureChannels2ID);
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
        if (unmute != cfg.m_Muted) {
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



#include "alsa-sound.moc"
