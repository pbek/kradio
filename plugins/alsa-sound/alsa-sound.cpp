/***************************************************************************
                          alsa-sound.cpp  -  description
                             -------------------
    begin                : Thu May 26 2005
    copyright            : (C) 2005 by Martin Witte
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

#include <klocale.h>
#include <kaboutdata.h>
#include <QtCore/QFile>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <string.h>

#include <sys/soundcard.h>
#include <alsa/asoundlib.h>

#include "alsa-sound.h"
#include "alsa-sound-configuration.h"
// #include "capture-thread.h"
// #warning "FIXME: port about widgets"
//#include "aboutwidget.h"
#include "utils.h"
#include "debug-profiler.h"

///////////////////////////////////////////////////////////////////////
//// plugin library functions

PLUGIN_LIBRARY_FUNCTIONS(AlsaSoundDevice, PROJECT_NAME, i18n("Advanced Linux Sound Architecture (ALSA) Support"));

/////////////////////////////////////////////////////////////////////////////

struct _lrvol { unsigned char l, r; short dummy; };


AlsaSoundDevice::AlsaSoundDevice(const QString &instanceID, const QString &name)
    : PluginBase(instanceID, name, i18n("KRadio ALSA Sound Plugin")),
      m_hPlayback(NULL),
      m_hCapture(NULL),
      m_hPlaybackMixer(NULL),
      m_hCaptureMixer(NULL),
      m_PlaybackFormat(),
      m_CaptureFormat(),
      m_PlaybackDeviceName("default"),
      m_PlaybackMixerName ("default"),
      m_CaptureDeviceName ("default"),
      m_CaptureMixerName  ("default"),
      m_PlaybackLatency(30),
      m_CaptureLatency(30),
      m_PassivePlaybackStreams(),
      m_PlaybackStreamID(),
      m_CaptureStreamID(),
      m_nonBlockingPlayback(false),
      m_nonBlockingCapture (false),
      m_PlaybackChunkSize  (16*1024),
      m_PlaybackBufferSize (96*1024),
      m_CaptureChunkSize   (16*1024),
      m_CaptureBufferSize  (96*1024),
      m_PlaybackBuffer(m_PlaybackBufferSize, /*synchronized =*/ true),
      m_CaptureBuffer (m_CaptureBufferSize,  /*synchronized =*/ true),
      m_PlaybackBufferWaitForMinFill(90 /*percent*/),
      m_CaptureRequestCounter(0),
      m_CapturePos(0),
      m_CaptureStartTime(0),
      m_EnablePlayback(true),
      m_EnableCapture(true),
      m_SoftPlaybackVolumeCorrectionFactor(1),
      m_SoftPlaybackVolumeEnabled(false),
      m_SoftPlaybackVolume(1.0),
      m_SoftPlaybackVolumeMuted(false),
      m_CaptureFormatOverrideEnable(false),
      m_CaptureFormatOverride(),
      m_use_threads(true),
      m_playbackThread(NULL),
      m_captureThread(NULL)

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
        s->register4_sendMuteSink              (this);
        s->register4_sendMuteSourcePlayback    (this);
        s->register4_sendUnmuteSink            (this);
        s->register4_sendUnmuteSourcePlayback  (this);
        s->register4_queryIsSinkMuted          (this);
        s->register4_queryIsSourcePlaybackMuted(this);
        s->register4_sendCaptureVolume(this);
        s->register4_queryPlaybackVolume(this);
        s->register4_queryCaptureVolume(this);
        s->register4_sendStartPlayback(this);
        s->register4_sendPausePlayback(this);
        s->register4_sendResumePlayback(this);
        s->register4_sendStopPlayback(this);
        s->register4_queryIsPlaybackRunning(this);
        s->register4_sendStartCaptureWithFormat(this);
        s->register4_sendStopCapture(this);
        s->register4_queryIsCaptureRunning(this);
        s->register4_notifySoundStreamClosed(this);
        s->register4_notifySoundStreamSourceRedirected(this);
        s->register4_notifySoundStreamSinkRedirected(this);
        s->register4_notifySoundStreamData(this);
    }
}

// PluginBase

void AlsaSoundDevice::saveState (KConfigGroup &c) const
{
    PluginBase::saveState(c);

    c.writeEntry("use-threads",     m_use_threads);

    c.writeEntry("playback-device-name", m_PlaybackDeviceName);
    c.writeEntry("playback-mixer-name",  m_PlaybackMixerName);
    c.writeEntry("capture-device-name",  m_CaptureDeviceName);
    c.writeEntry("capture-mixer-name",   m_CaptureMixerName);
    c.writeEntry("enable-playback",      m_EnablePlayback);
    c.writeEntry("enable-capture",       m_EnableCapture);
    c.writeEntry("playback-buffer-size",          (unsigned int) m_PlaybackBufferSize);
    c.writeEntry("playback-buffer-chunk-size",    (unsigned int) m_PlaybackChunkSize);
    c.writeEntry("capture-buffer-size",           (unsigned int) m_CaptureBufferSize);
    c.writeEntry("capture-buffer-chunk-size",     (unsigned int) m_CaptureChunkSize);
    c.writeEntry("nonblocking-playback",          m_nonBlockingPlayback);
    c.writeEntry("nonblocking-capture",           m_nonBlockingCapture);
    c.writeEntry("soundstreamclient-id", m_SoundStreamClientID);

    c.writeEntry("mixer-settings",       m_CaptureMixerSettings.count());
    int i = 0;
    for (QMap<QString, AlsaConfigMixerSetting>::const_iterator it = m_CaptureMixerSettings.begin(); it != m_CaptureMixerSettings.end(); ++it, ++i) {

        QString prefix = QString("mixer-setting-%1-").arg(i);
        (*it).saveState(c, prefix);
    }

    c.writeEntry("soft_playback_volume_correction_factor", m_SoftPlaybackVolumeCorrectionFactor);
    c.writeEntry("soft_playback_volume_enable",            m_SoftPlaybackVolumeEnabled);
    c.writeEntry("soft_playback_volume",                   m_SoftPlaybackVolume);
    c.writeEntry("soft_playback_volume_muted",             m_SoftPlaybackVolumeMuted);

    c.writeEntry("sound_format_override_enable",           m_CaptureFormatOverrideEnable);
    m_CaptureFormatOverride.saveConfig("sound_format_override_", c);

    c.writeEntry("workaroundSleepPlaybackMilliSeconds", m_workaroundSleepPlaybackMilliSeconds);
    c.writeEntry("workaroundSleepCaptureMilliSeconds",  m_workaroundSleepCaptureMilliSeconds);
}


void AlsaSoundDevice::restoreState (const KConfigGroup &c)
{
    PluginBase::restoreState(c);
    setSoundStreamClientID(c.readEntry("soundstreamclient-id", getSoundStreamClientID()));

    m_use_threads         = c.readEntry("use-threads",                true);

    m_EnablePlayback      = c.readEntry("enable-playback",            true);
    m_EnableCapture       = c.readEntry("enable-capture",             true);
    m_PlaybackBufferSize  = c.readEntry("playback-buffer-size",       c.readEntry("buffer-size",          96*1024));
    m_PlaybackChunkSize   = c.readEntry("playback-buffer-chunk-size",                                     16*1024 );
    m_CaptureBufferSize   = c.readEntry("capture-buffer-size",        c.readEntry("buffer-size",          96*1024));
    m_CaptureChunkSize    = c.readEntry("capture-buffer-chunk-size",                                      16*1024 );
    m_nonBlockingPlayback = c.readEntry("nonblocking-playback",       false);
    m_nonBlockingCapture  = c.readEntry("nonblocking-capture",        false);

    QString dev       = c.readEntry("playback-device-name", "default");
    setPlaybackDevice(dev,  true);
    dev               = c.readEntry("playback-mixer-name",  "default");
    setPlaybackMixer (dev,  true);
    dev               = c.readEntry("capture-device-name",  "default");
    setCaptureDevice (dev,  true);
    dev               = c.readEntry("capture-mixer-name",   "default");
    setCaptureMixer  (dev,  true);

    setBufferSizes(m_PlaybackBufferSize, m_PlaybackChunkSize, m_CaptureBufferSize, m_CaptureChunkSize);


    int n = c.readEntry("mixer-settings",  0);
    for (int i = 0; i < n; ++i) {
        QString prefix = QString("mixer-setting-%1-").arg(i);
        AlsaConfigMixerSetting  s(c, prefix);
        m_CaptureMixerSettings.insert(s.getIDString(), s);
    }

    m_SoftPlaybackVolumeCorrectionFactor = c.readEntry("soft_playback_volume_correction_factor", 1.0);
    m_SoftPlaybackVolumeEnabled          = c.readEntry("soft_playback_volume_enable",            false);
    m_SoftPlaybackVolume                 = c.readEntry("soft_playback_volume",                   1.0);
    m_SoftPlaybackVolumeMuted            = c.readEntry("soft_playback_volume_muted",             false);

    m_CaptureFormatOverrideEnable        = c.readEntry("sound_format_override_enable",           false);
    m_CaptureFormatOverride.restoreConfig("sound_format_override_", c);

    m_workaroundSleepPlaybackMilliSeconds = c.readEntry("workaroundSleepPlaybackMilliSeconds",  0);
    m_workaroundSleepCaptureMilliSeconds  = c.readEntry("workaroundSleepCaptureMilliSeconds",   0);

    emit sigUpdateConfig();
}


ConfigPageInfo  AlsaSoundDevice::createConfigurationPage()
{
    AlsaSoundConfiguration *conf = new AlsaSoundConfiguration(NULL, this);
    QObject::connect(this, SIGNAL(sigUpdateConfig()), conf, SLOT(slotUpdateConfig()));
    return ConfigPageInfo (conf,
                           i18n("ALSA Sound"),
                           i18n("ALSA Sound Device Options"),
                           "kradio_alsa2");
}


/*AboutPageInfo AlsaSoundDevice::createAboutPage()
{*/
/*    KAboutData aboutData("kradio4",
                         NULL,
                         NULL,
                         I18N_NOOP("ALSA Sound Plugin for KRadio"),
                         KAboutData::License_GPL,
                         "(c) 2005 Martin Witte",
                         0,
                         "http://sourceforge.net/projects/kradio",
                         0);
    aboutData.addAuthor("Martin Witte",  "", "emw-kradio@nocabal.de");

    return AboutPageInfo(
              new KRadioAboutWidget(aboutData, KRadioAboutWidget::AbtTabbed),
              i18n("ALSA Sound"),
              i18n("ALSA Sound"),
              "kradio_alsa_sound"
           );
*/
//     return AboutPageInfo();
// }



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
            if (m_PlaybackChannels2ID.contains(cfg.m_Channel)) {
                if (cfg.m_Volume < 0) {
                    bool dummy_mute_info = false;
                    cfg.m_Volume = readPlaybackMixerVolume(cfg.m_Channel, dummy_mute_info);
                }
                if (writePlaybackMixerVolume(cfg.m_Channel, cfg.m_Volume, cfg.m_Muted)) {
                    notifyPlaybackVolumeChanged(id, cfg.m_Volume);
                    notifySinkMuted(id, cfg.m_Volume);
                }
                m_PlaybackPollingTimer.start(m_PlaybackLatency);
            }
        }

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


bool AlsaSoundDevice::resumePlayback(SoundStreamID id)
{
    if (id.isValid() && m_PlaybackStreamID == id) {
        SoundFormat f = m_PlaybackFormat;
        // tribute to pulse audio: With pulse audio, latency increases for times no samples are send to the device
        // (as in paused mode). Thus we have to reopen the device.
        openPlaybackDevice(f, /*reopen*/true);
        return true;
    } else {
        return false;
    }
}


bool AlsaSoundDevice::stopPlayback(SoundStreamID id)
{
    if (id.isValid() && m_PlaybackStreams.contains(id)) {

        SoundStreamConfig &cfg = m_PlaybackStreams[id];

        if (!cfg.m_ActiveMode) {
            if  (m_PassivePlaybackStreams.contains(id)) {
/*                float tmp = 0;
                writePlaybackMixerVolume(cfg.m_Channel, tmp, true);*/
                m_PassivePlaybackStreams.removeAll(id);
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
    if ((id.isValid() && (m_PlaybackStreamID == id)) || m_PassivePlaybackStreams.contains(id)) {
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
            if (m_CaptureChannels2ID.contains(cfg.m_Channel)) {
                if (cfg.m_Volume < 0) {
                    cfg.m_Volume = readCaptureMixerVolume(cfg.m_Channel);
                }
                if (writeCaptureMixerVolume(cfg.m_Channel, cfg.m_Volume)) {
                    notifyCaptureVolumeChanged(m_CaptureStreamID, cfg.m_Volume);
                }
            }

            openCaptureDevice(proposed_format);

            if (!m_hCapture) {
                return false;
            }
        }

        real_format = m_CaptureFormat;
        m_CaptureRequestCounter++;

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

            slotPollCapture();

            m_CaptureStreamID = SoundStreamID::InvalidID;

            closeCaptureMixerDevice();
            closeCaptureDevice();

            m_CaptureBuffer.clear();
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


bool AlsaSoundDevice::noticeSoundStreamSinkRedirected(SoundStreamID oldID, SoundStreamID newID)
{
    bool found = false;
    if (m_PlaybackStreams.contains(oldID)) {
        m_PlaybackStreams.insert(newID, m_PlaybackStreams[oldID]);
        if (newID != oldID)
            m_PlaybackStreams.remove(oldID);
        found = true;
    }
    if (m_PlaybackStreamID == oldID)
        m_PlaybackStreamID = newID;
    if (m_PassivePlaybackStreams.contains(oldID)) {
        m_PassivePlaybackStreams.removeAll(oldID);
        m_PassivePlaybackStreams.append(newID);
    }
    return found;
}


bool AlsaSoundDevice::noticeSoundStreamSourceRedirected(SoundStreamID oldID, SoundStreamID newID)
{
    bool found = false;
    if (m_CaptureStreams.contains(oldID)) {
        m_CaptureStreams.insert(newID, m_CaptureStreams[oldID]);
        if (newID != oldID)
            m_CaptureStreams.remove(oldID);
        found = true;
    }

    if (m_CaptureStreamID == oldID)
        m_CaptureStreamID = newID;
    return found;
}


bool AlsaSoundDevice::noticeSoundStreamData(SoundStreamID id,
                                            const SoundFormat &format,
                                            const char *data, size_t size, size_t &consumed_size,
                                            const SoundMetaData &/*md*/
                                           )
{
    if (!id.isValid() || id != m_PlaybackStreamID)
        return false;

    if (!m_hPlayback) {
        openPlaybackDevice(format);
    } else if (format != m_PlaybackFormat) {
        m_PlaybackBuffer.clear();
        closePlaybackDevice(/*force = */true);
        openPlaybackDevice(format);
        // error handling ?
    }

    // in case of error, return false
    if (!m_hPlayback) {
        return false;
    }

    const char *buffer        = data;
    size_t n = m_PlaybackBuffer.addData(buffer, size);
    consumed_size  = (consumed_size == SIZE_T_DONT_CARE) ? n : min (consumed_size, n);

    return true;
}


// pure virtual members of ThreadLoggingClient
IErrorLogClient *AlsaSoundDevice::getErrorLogClient()
{
    return this;
}

void AlsaSoundDevice::slotPollPlayback()
{
    //     logDebug("AlsaSoundDevice::slotPollPlayback()");
    BlockProfiler p("AlsaSoundDevice::slotPollPlayback()");
    
//     printf ("slotPollPlayback\n");      

    if (m_PlaybackStreamID.isValid()) {

//         logDebug(i18n("AlsaSoundDevice::slotPollPlayback: buffer fill = %1%% (%2 bytes, w=%3)", 100*m_PlaybackBuffer.getFillSize() / m_PlaybackBuffer.getSize(), m_PlaybackBuffer.getFillSize(), m_PlaybackBufferWaitForMinFill));

        size_t required_fill = getPlaybackBufferMinFill();

        if (m_hPlayback && m_PlaybackBuffer.getFillSize() >= required_fill) {
            setWaitForMinPlaybackBufferFill(0);
            if (m_use_threads && m_playbackThread) {
//                 m_playbackThread->awake();
            } else {
                while (!m_use_threads && m_PlaybackBuffer.getFillSize() > 0 && m_hPlayback) {

                    size_t   buffersize       = 0;
                    size_t   maxAvailableSize = 0;
                    int      frameSize        = m_PlaybackFormat.frameSize();
                    char     *buffer          = getPlaybackData(buffersize, maxAvailableSize);
                    int      framesWritten    = snd_pcm_writei(m_hPlayback, buffer, buffersize / frameSize);
                    int      bytesWritten     = framesWritten * frameSize;

                    if (framesWritten > 0) {
                        m_PlaybackBuffer.removeData(bytesWritten);
                    } else if (framesWritten == 0) {
                        logError(i18n("ALSA Plugin: cannot write data for device %1", m_PlaybackDeviceName));
                        break;
                    } else if (framesWritten == -EAGAIN) {
                        // do nothing
                        break;
                    } else {
                        snd_pcm_prepare(m_hPlayback);
                        logWarning(i18n("ALSA Plugin: buffer underrun for device %1", m_PlaybackDeviceName));
                    }
                }
            }
        }
        checkThreadErrorsAndWarning();

        // printf ("    slotPollPlayback: before buffer size check: free size = %zi, size = %zi, free ratio = %4.1f %%\n", m_PlaybackBuffer.getFreeSize(), m_PlaybackBuffer.
	// 	getSize(), (double)m_PlaybackBuffer.getFreeSize() / (double) m_PlaybackBuffer.getSize() * 100.0 );
        // size_t startFill = m_PlaybackBuffer.getFillSize();
        size_t oldFree   = 0;
        size_t curFree   = m_PlaybackBuffer.getFreeSize();
        while (oldFree != curFree && curFree > 0) {
            // printf ("    slotPollPlayback: buffer check successful: requesting data\n");
            notifyReadyForPlaybackData(m_PlaybackStreamID, curFree);
	    oldFree = curFree;
            curFree = m_PlaybackBuffer.getFreeSize();
        }
        // printf ("    slotPollPlayback: free = %zi, fill = %zi, added %zi\n", m_PlaybackBuffer.getFreeSize(), m_PlaybackBuffer.getFillSize(), m_PlaybackBuffer.getFillSize() - startFill);

        checkMixerVolume(m_PlaybackStreamID);
    }

    QList<SoundStreamID>::const_iterator end = m_PassivePlaybackStreams.end();
    for (QList<SoundStreamID>::const_iterator it = m_PassivePlaybackStreams.begin(); it != end; ++it)
        checkMixerVolume(*it);
}


void AlsaSoundDevice::slotPollCapture()
{
    //     logDebug("AlsaSoundDevice::slotPollCapture()");
    BlockProfiler p("AlsaSoundDevice::slotPollCapture()");

    if (m_CaptureStreamID.isValid() && m_hCapture) {

        size_t bufferSize = 0;
        char  *buffer     = getFreeCaptureBuffer(bufferSize);

        while (!m_use_threads && bufferSize) {

            size_t frameSize  = m_CaptureFormat.frameSize();
            int    framesRead = snd_pcm_readi(m_hCapture, buffer, bufferSize / frameSize);
            size_t bytesRead  = framesRead > 0 ? framesRead * frameSize : 0;

            if (framesRead > 0) {
                m_CaptureBuffer.removeFreeSpace(bytesRead);
            } else if (framesRead == 0) {
                snd_pcm_prepare(m_hCapture);
                logError(i18n("ALSA Plugin: cannot read data from device %1", m_CaptureDeviceName));
                break;
            } else if (framesRead == -EAGAIN) {
                // do nothing
                break;
            } else {
                snd_pcm_prepare(m_hCapture);
                logWarning(i18n("ALSA Plugin: buffer overrun for device %1 (buffersize=%2, buffer=%3)", m_CaptureDeviceName, bufferSize, (long long unsigned)buffer));
            }
            // prepare for next read try
            buffer     = getFreeCaptureBuffer(bufferSize);
        }

        if (m_use_threads && m_captureThread && m_hCapture && m_CaptureBuffer.getFreeSize() > 0) {
//             m_captureThread->awake();
        }

//         logDebug(i18n("AlsaSoundDevice::slotPollCapture: buffer fill = %1%% (%2 bytes)", 100*m_CaptureBuffer.getFillSize() / m_CaptureBuffer.getSize(), m_CaptureBuffer.getFillSize()));

        checkThreadErrorsAndWarning();

        QString dev = QString("alsa://%1").arg(m_CaptureDeviceName);
        while (m_CaptureBuffer.getFillSize() >= m_CaptureBuffer.getSize() / 8) { // FIXME: why / 8? kind of hysteresis, having buffer min fill before starting playing to avoid subsequend underflows?
            size_t size = 0;
            buffer = m_CaptureBuffer.getData(size);
            time_t cur_time = time(NULL);
            size_t consumed_size = SIZE_T_DONT_CARE;

            notifySoundStreamData(m_CaptureStreamID, m_CaptureFormat, buffer, size, consumed_size, SoundMetaData(m_CapturePos, cur_time - m_CaptureStartTime, cur_time, i18n("internal stream, not stored (%1)", dev)));

            if (consumed_size == SIZE_T_DONT_CARE)
                consumed_size = size;
            m_CaptureBuffer.removeData(consumed_size);
            m_CapturePos += consumed_size;
            if (consumed_size < size)
                break;
        }
    }
    if (m_CaptureStreamID.isValid())
        checkMixerVolume(m_CaptureStreamID);
}


bool AlsaSoundDevice::openPlaybackDevice(const SoundFormat &format, bool reopen)
{
    if (!m_PlaybackDeviceName.length())
        return false;

    if (m_hPlayback) {

        if (reopen) {

            logDebug(QString("AlsaSoundDevice::openPlaybackDevice: re-opening playback device %1").arg(m_PlaybackDeviceName));

            closePlaybackDevice ( /* force = */ true);

        } else {

            if (format != m_PlaybackFormat)
                return false;

            return true;
        }
    } else {
        logDebug(QString("AlsaSoundDevice::openPlaybackDevice: opening playback device %1").arg(m_PlaybackDeviceName));

        if (reopen)         // FIXME: emw: please check if this makes sense !?!?
            return true;
    }

    m_PlaybackFormat = format;

    setWaitForMinPlaybackBufferFill(90/*percent*/);

    bool error = !openAlsaDevice(m_hPlayback, m_PlaybackFormat, QFile::encodeName(m_PlaybackDeviceName), SND_PCM_STREAM_PLAYBACK, (m_nonBlockingPlayback ? SND_PCM_NONBLOCK : 0), m_PlaybackLatency, m_PlaybackBufferSize, m_PlaybackChunkSize);

    if (!error) {

        if (m_use_threads) {
            if (m_playbackThread) {
                logError(i18n("AlsaPlugin: internal error. Expected non-existing playback thread, but found one"));
                m_playbackThread->setDone();
                if (!m_playbackThread->wait(1000)) {
                    m_playbackThread->terminate();
                }
                delete m_playbackThread;
                m_playbackThread = NULL;
            }
            m_playbackThread = new AlsaThread(this, /*playback_not_capture = */ true, m_hPlayback, m_PlaybackFormat);
            m_playbackThread->setLatency(m_workaroundSleepPlaybackMilliSeconds * 1000);
            m_playbackThread->start();
            m_PlaybackPollingTimer.start(40); // polling still necessary, however mainly for pushing sound around and getting volume
	    QObject::connect(m_playbackThread, SIGNAL(sigRequestPlaybackData()), this, SLOT(slotPollPlayback()), Qt::QueuedConnection);
        } else {
            m_PlaybackPollingTimer.start(m_PlaybackLatency);
        }
    } else {
        closePlaybackDevice(true);
        closeSoundStream(m_PlaybackStreamID);
    }

    return !error;
}


bool AlsaSoundDevice::openCaptureDevice(const SoundFormat &format, bool reopen)
{
    if (!m_CaptureDeviceName.length())
        return false;

    if (m_hCapture) {

        if (reopen) {

            logDebug(QString("AlsaSoundDevice::openCaptureDevice: re-opening capture device %1").arg(m_CaptureDeviceName));
            closeCaptureDevice ( /* force = */ true);

        } else {

            if (format != m_CaptureFormat)
                return false;

            return true;
        }
    } else {

        logDebug(QString("AlsaSoundDevice::openCaptureDevice: opening capture device %1").arg(m_CaptureDeviceName));

        if (reopen)         // FIXME: emw: please check if this makes sense !?!?
            return true;
    }

    m_CaptureFormat = format;

    if (m_CaptureFormatOverrideEnable) {
        m_CaptureFormat = m_CaptureFormatOverride;
    }

    bool error = !openAlsaDevice(m_hCapture, m_CaptureFormat, QFile::encodeName(m_CaptureDeviceName), SND_PCM_STREAM_CAPTURE, (m_nonBlockingCapture ? SND_PCM_NONBLOCK : 0), m_CaptureLatency, m_CaptureBufferSize, m_CaptureChunkSize);

    if (!error) {

        if (m_use_threads) {
            if (m_captureThread) {
                logError(i18n("AlsaPlugin: internal error. Expected non-existing capture thread, but found one"));
                m_captureThread->setDone();
                if (!m_captureThread->wait(1000)) {
                    m_captureThread->terminate();
                }
                delete m_captureThread;
                m_captureThread = NULL;
            }
            m_captureThread = new AlsaThread(this, /*playback_not_capture = */ false, m_hCapture, m_CaptureFormat);
            m_captureThread->setLatency(m_workaroundSleepCaptureMilliSeconds * 1000);
            m_captureThread->start();
            m_CapturePollingTimer.start(40); // polling still necessary, however mainly for pushing sound around and getting volume
	    QObject::connect(m_captureThread, SIGNAL(sigCaptureDataAvailable()), this, SLOT(slotPollCapture()), Qt::QueuedConnection);
        } else {
            m_CapturePollingTimer.start(m_CaptureLatency);
        }
    } else {
        closeCaptureDevice(true);
        closeSoundStream(m_CaptureStreamID);
    }

    return !error;
}


bool AlsaSoundDevice::openAlsaDevice(snd_pcm_t *&alsa_handle, SoundFormat &format, const QByteArray &pcm_name_ba, snd_pcm_stream_t stream, int flags, unsigned &latency, size_t buffer_size, size_t chunk_size)
{
    bool                 error    = false;
    int                  dir      = 0;
    snd_output_t        *log      = NULL;
    snd_pcm_hw_params_t *hwparams = NULL;
    snd_pcm_sw_params_t *swparams = NULL;
    const char          *pcm_name = pcm_name_ba.constData();

    snd_pcm_hw_params_alloca(&hwparams); // allocate on stack, needs no free
    snd_pcm_sw_params_alloca(&swparams); // allocate on stack, needs no free
    snd_output_stdio_attach(&log, stderr, 0);


    /* OPEN */

    int err = 0;
    if (!error && (err = snd_pcm_open(&alsa_handle, pcm_name, stream, flags)) < 0) {
        logError(i18n("ALSA Plugin: Error opening PCM device %1: %2", pcm_name, snd_strerror(err)));
        error = true;
    }

    if (!error && (err = snd_pcm_hw_params_any(alsa_handle, hwparams)) < 0) {
        logError(i18n("ALSA Plugin: Can not configure PCM device %1: %2", pcm_name, snd_strerror(err)));
        error = true;
    }

    /* interleaved access type */

    if (!error && (err = snd_pcm_hw_params_set_access(alsa_handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        logError(i18n("ALSA Plugin: Error setting access for %1", pcm_name));
        error = true;
    }

    /* sample format */
    snd_pcm_format_t sample_format = snd_pcm_build_linear_format(format.m_SampleBits,
                                                                 format.m_SampleBits,
                                                                 !format.m_IsSigned,
                                                                 format.m_Endianness == BIG_ENDIAN);
    if (!error && (err = snd_pcm_hw_params_set_format(alsa_handle, hwparams, sample_format)) < 0) {
        logError(i18n("ALSA Plugin: Error setting sample format for %1: %2", pcm_name, snd_strerror(err)));
        error = true;
    }

    /* channels */
    if (!error && (err = snd_pcm_hw_params_set_channels(alsa_handle, hwparams, format.m_Channels)) < 0) {
        logError(i18n("ALSA Plugin: Error setting channels for %1: %2", pcm_name, snd_strerror(err)));
        error = true;
    }

    /* sample rate */
    unsigned orgrate = format.m_SampleRate;
    if (!error && (err = snd_pcm_hw_params_set_rate_near(alsa_handle, hwparams, &format.m_SampleRate, &dir)) < 0) {
        logError(i18n("ALSA Plugin: Error setting rate for %1: %2", pcm_name, snd_strerror(err)));
        error = true;
    }
    if (!error && orgrate != format.m_SampleRate) {
        logWarning(i18n("ALSA Plugin: The rate %1 Hz is not supported by your hardware %2. Using %3 Hz instead", orgrate, pcm_name, format.m_SampleRate));
    }

    // again reenabling periods and buffer sizes, this time hopefully more correctly
    snd_pcm_uframes_t  max_buffer_frames = 0;
    if (!error && (err = snd_pcm_hw_params_get_buffer_size_max(hwparams, &max_buffer_frames)) < 0) {
        logError(i18n("ALSA Plugin: Error reading max buffer size for %1: %2", pcm_name, snd_strerror(err)));
        error = true;
    }

    snd_pcm_uframes_t  period_frames     = qMin((size_t)max_buffer_frames / 3, chunk_size / format.frameSize());
    if (!error && (err = snd_pcm_hw_params_set_period_size_near(alsa_handle, hwparams, &period_frames, 0)) < 0) {
        logError(i18n("ALSA Plugin: Error setting period size to %1 for %2: %3", period_frames, pcm_name, snd_strerror(err)));
        error = true;
    }
    snd_pcm_uframes_t  buffer_frames     = qMin((size_t)max_buffer_frames, buffer_size / format.frameSize());
    if (!error && (err = snd_pcm_hw_params_set_buffer_size_near(alsa_handle, hwparams, &buffer_frames)) < 0) {
        logError(i18n("ALSA Plugin: Error setting buffer size to %1 for %2: %3", buffer_frames, pcm_name, snd_strerror(err)));
        error = true;
    }
    logDebug(i18n("ALSA Plugin(%1) setting parameters near: period size = %2 [frames], hwbuffer size = %3 [frames]", pcm_name, period_frames, buffer_frames));

    /* set all params */

    if (!error && (err = snd_pcm_hw_params(alsa_handle, hwparams)) < 0) {
        logError(i18n("ALSA Plugin: Error setting HW params: %1", QString(snd_strerror(err))));
        error = true;
    }

    snd_pcm_uframes_t period_frames_real = 0;
    if (!error && (err = snd_pcm_hw_params_get_period_size(hwparams, &period_frames_real, 0)) < 0) {
        logError(i18n("ALSA Plugin: Error getting period size for %1: %2", pcm_name, snd_strerror(err)));
        error = true;
    }

    snd_pcm_uframes_t hwbuffer_frames    = 0;
    if (!error && (err = snd_pcm_hw_params_get_buffer_size(hwparams, &hwbuffer_frames))     < 0) {
        logError(i18n("ALSA Plugin: Error getting hw buffer size for %1: %2", pcm_name, snd_strerror(err)));
        error = true;
    }

    logDebug(i18n("ALSA Plugin(%1): period size = %2 [frames], hwbuffer size = %3 [frames]", pcm_name, period_frames_real, hwbuffer_frames));

    latency = (1000 * period_frames_real) / format.m_SampleRate / 2;
    if (latency < 40) {
        latency = 40;
    }
    logDebug(i18n("ALSA Plugin(%1): Setting timer latency to %2", pcm_name, latency));


    if (!error && (err = snd_pcm_sw_params_current(alsa_handle, swparams)) < 0) {
        logError(i18n("ALSA Plugin: Error getting sw params for %1: %2", pcm_name, snd_strerror(err)));
        error = true;
    }
    if (!error && (err = snd_pcm_sw_params_set_avail_min(alsa_handle, swparams, period_frames_real)) < 0) {
        logError(i18n("ALSA Plugin: Error setting sw params min available frames to %1 for %2: %3", period_frames_real, pcm_name, snd_strerror(err)));
        error = true;
    }
    if (!error && (err = snd_pcm_sw_params(alsa_handle, swparams)) < 0) {
        logError(i18n("ALSA Plugin: Error installing sw params for %1: %2", pcm_name, snd_strerror(err)));
        error = true;
    }


    if (!error) {
        snd_pcm_prepare(alsa_handle);
#ifdef DEBUG
        snd_pcm_dump(alsa_handle, log);
#endif
    }
    snd_output_close(log);

    return !error;
}


bool AlsaSoundDevice::closePlaybackDevice(bool force)
{
    if (!m_PlaybackStreamID.isValid() || force) {

        m_PlaybackPollingTimer.stop();

        checkThreadErrorsAndWarning();
        if (m_use_threads && m_playbackThread) {
            m_playbackThread->setDone();
            if(!m_playbackThread->wait(1000)) {
                m_playbackThread->terminate();
                logWarning(i18n("Alsa Plugin (device %1): oops, had to kill the playback thread. It did not stop as desired", m_PlaybackDeviceName));
            }
            delete m_playbackThread;
            m_playbackThread = NULL;
        }

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

        m_CapturePollingTimer.stop();

        checkThreadErrorsAndWarning();
        if (m_use_threads && m_captureThread) {
            m_captureThread->setDone();
            if(!m_captureThread->wait(1000)) {
                m_captureThread->terminate();
                logWarning(i18n("Alsa Plugin (device %1): oops, had to kill the capture thread. It did not stop as desired", m_PlaybackDeviceName));
            }
            delete m_captureThread;
            m_captureThread = NULL;
        }


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
    return openMixerDevice(m_hPlaybackMixer, m_PlaybackMixerName, reopen, &m_PlaybackPollingTimer, m_PlaybackLatency);
}


bool AlsaSoundDevice::openCaptureMixerDevice(bool reopen)
{
    //     logDebug("AlsaSoundDevice::openCaptureMixerDevice: card == " + m_CaptureDeviceName);
    return openMixerDevice(m_hCaptureMixer, m_CaptureMixerName, reopen, &m_CapturePollingTimer, m_CaptureLatency);
}


bool AlsaSoundDevice::closePlaybackMixerDevice(bool force)
{
    return closeMixerDevice(m_hPlaybackMixer, m_PlaybackMixerName, m_PlaybackStreamID, m_hPlayback, force, &m_PlaybackPollingTimer);
}

bool AlsaSoundDevice::closeCaptureMixerDevice(bool force)
{
    return closeMixerDevice(m_hCaptureMixer, m_CaptureMixerName, m_CaptureStreamID, m_hCapture, force, &m_CapturePollingTimer);
}


static int mixer_dummy_callback(snd_mixer_t *, unsigned int /*mask*/, snd_mixer_elem_t */*elem*/)
{
    return 0;
}

bool AlsaSoundDevice::openMixerDevice(snd_mixer_t *&mixer_handle, const QString &mixerName, bool reopen, QTimer *timer, int timer_latency)
{
    if (reopen) {
        if (mixer_handle != NULL)
            closeMixerDevice(mixer_handle, mixerName, SoundStreamID::InvalidID, NULL, /* force = */ true, timer);
        else
            return true;
    }

    if (!mixer_handle) {
        bool error = false;
        if (snd_mixer_open (&mixer_handle, 0) < 0) {
            staticLogError(i18n("ALSA Plugin: Error opening mixer"));
            error = true;
        }
        bool attached = false;
        if (!error) {
            if (snd_mixer_attach (mixer_handle, mixerName.toLocal8Bit()) < 0) {
                staticLogError(i18n("ALSA Plugin: ERROR: snd_mixer_attach for card %1", mixerName));
                error = true;
            } else {
                attached = true;
            }
        }
        if (!error && snd_mixer_selem_register(mixer_handle, NULL, NULL) < 0) {
            staticLogError(i18n("ALSA Plugin: Error: snd_mixer_selem_register for card %1", mixerName));
            error = true;
        }
        if (!error && snd_mixer_load (mixer_handle) < 0) {
            staticLogError(i18n("ALSA Plugin: Error: snd_mixer_load for card %1", mixerName));
            error = true;
        }
        if (mixer_handle) {
            snd_mixer_set_callback (mixer_handle, mixer_dummy_callback);
        }

        if (error) {
            if (attached) {
                snd_mixer_detach(mixer_handle, mixerName.toLocal8Bit());
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


bool AlsaSoundDevice::closeMixerDevice(snd_mixer_t *&mixer_handle, const QString &mixerName, SoundStreamID id, snd_pcm_t *pcm_handle, bool force, QTimer *timer)
{
    if (!id.isValid() || force) {

        if (!pcm_handle && timer)
            timer->stop();

        if (mixer_handle) {
            snd_mixer_free(mixer_handle);
            snd_mixer_detach(mixer_handle, mixerName.toLocal8Bit());
            snd_mixer_close (mixer_handle);
        }
        mixer_handle = NULL;
    }
    return mixer_handle == NULL;
}

void AlsaSoundDevice::getPlaybackMixerChannels(
    const QString &mixerName,
    snd_mixer_t   *__mixer_handle,
    QStringList   &retval, QMap<QString, AlsaMixerElement> &ch2id,
    bool           playback_enabled)
{
    retval.clear();
    ch2id.clear();

    if (!playback_enabled) {
        return;
    }

    snd_mixer_t *mixer_handle = __mixer_handle/*m_hPlaybackMixer*/;
    bool         use_tmp_handle = false;

    if (!mixer_handle) {
        openMixerDevice(mixer_handle, mixerName, false, NULL, 0);
        use_tmp_handle = true;
    }

    if (mixer_handle) {
        snd_mixer_elem_t *elem = NULL;

        for (elem = snd_mixer_first_elem(mixer_handle); elem; elem = snd_mixer_elem_next(elem)) {
            AlsaMixerElement sid;
            if (!snd_mixer_selem_is_active(elem))
                continue;
            snd_mixer_selem_get_id(elem, sid);
            QString name = snd_mixer_selem_id_get_name(sid);
            int idx = snd_mixer_selem_id_get_index(sid);
            if (idx)
                name = i18nc("context-mixername-number", "%1 %2", name, idx);
            if (snd_mixer_selem_has_playback_volume(elem)) {
                ch2id[name] = sid;
                retval.append(name);
            }
        }
    }

    if (use_tmp_handle && mixer_handle) {
        closeMixerDevice(mixer_handle, mixerName, SoundStreamID::InvalidID, NULL, true, NULL);
    }
}

void AlsaSoundDevice::getCaptureMixerChannels(
    const QString &mixerName,
    snd_mixer_t   *__mixer_handle,
    QStringList   &vol_list, QMap<QString, AlsaMixerElement> &vol_ch2id,
    QStringList   &sw_list,  QMap<QString, AlsaMixerElement> &sw_ch2id,
    QStringList   *all_list,
    bool           capture_enabled
)
{
    vol_list.clear();
    sw_list.clear();
    if (all_list) all_list->clear();
    vol_ch2id.clear();
    sw_ch2id.clear();

    if (!capture_enabled) {
        return;
    }


    snd_mixer_t *mixer_handle = __mixer_handle /*m_hCaptureMixer*/;
    bool         use_tmp_handle = false;

    if (!mixer_handle) {
        //         staticLogDebug("AlsaSoundDevice::getCaptureMixerChannels: card == " + mixerName);
        openMixerDevice(mixer_handle, mixerName, false, NULL, 0);
        use_tmp_handle = true;
    }

    if (mixer_handle) {
        snd_mixer_elem_t *elem = NULL;

        for (elem = snd_mixer_first_elem(mixer_handle); elem; elem = snd_mixer_elem_next(elem)) {
            AlsaMixerElement sid;
            if (!snd_mixer_selem_is_active(elem))
                continue;
            snd_mixer_selem_get_id(elem, sid);
            QString name = snd_mixer_selem_id_get_name(sid);
            int idx = snd_mixer_selem_id_get_index(sid);
            if (idx)
                name = i18nc("context-mixerelement-name-number", "%1 %2", name, idx);

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
    }

    if (use_tmp_handle && mixer_handle) {
        closeMixerDevice(mixer_handle, mixerName, SoundStreamID::InvalidID, NULL, true, NULL);
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

        if ((m_hPlaybackMixer && m_PassivePlaybackStreams.contains(id)) || m_PlaybackStreamID == id) {
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
                notifySinkMuted(id, m);
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
    // do we have soft playback volume? 
    // ... Then we do not need to query any device below
    if (m_SoftPlaybackVolumeEnabled                        &&
        m_PlaybackStreamID.isValid()                       &&
        m_PlaybackStreams.contains(m_PlaybackStreamID)     &&
        m_PlaybackStreams[m_PlaybackStreamID].m_ActiveMode &&
        m_PlaybackStreams[m_PlaybackStreamID].m_Channel == channel)
    {
        muted = m_SoftPlaybackVolumeMuted;
        return m_SoftPlaybackVolume;
    }
    
    if (m_PlaybackChannels2ID.contains(channel) && m_hPlaybackMixer) {
        AlsaMixerElement  sid  = m_PlaybackChannels2ID[channel];
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
        // if we end up here, we have entered some mixer element but alsa either didn't 
        // find it or we could not successfully query the mixer status.
        logError("AlsaSound::readPlaybackMixerVolume: " +
                 i18n("error while reading volume from %1, channel %2", m_PlaybackMixerName, channel));
    }
    // silently fail here in case we do not have any mixer element
    return 0;
}


float AlsaSoundDevice::readCaptureMixerVolume(const QString &channel) const
{
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
        // if we end up here, we have entered some mixer element but alsa either didn't 
        // find it or we could not successfully query the mixer status.
        logError("AlsaSound::readCaptureMixerVolume: " +
                i18n("error while reading volume from %1, channel %2", m_CaptureMixerName, channel));
    }
    // silently fail here in case we do not have any mixer element
    return 0;
}


bool AlsaSoundDevice::writePlaybackMixerVolume (const QString &channel, float &vol, bool muted)
{
    if (vol > 1.0) vol = 1.0;
    if (vol < 0) vol = 0.0;

    // do we have soft playback volume? 
    // ... Then we do not need to write any device below
    if (m_SoftPlaybackVolumeEnabled                        &&
        m_PlaybackStreamID.isValid()                       &&
        m_PlaybackStreams.contains(m_PlaybackStreamID)     &&
        m_PlaybackStreams[m_PlaybackStreamID].m_ActiveMode &&
        m_PlaybackStreams[m_PlaybackStreamID].m_Channel == channel)
    {
        m_SoftPlaybackVolume = vol;
        m_SoftPlaybackVolumeMuted   = muted;
        return true;
    }
    
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
        // if we end up here, we have entered some mixer element but alsa either didn't 
        // find it or we could not successfully query the mixer status.
        logError("AlsaSound::writePlaybackMixerVolume: " +
        i18n("error while writing volume %1 to %2, channel %3", vol, m_PlaybackMixerName, channel));
        return false;
    }
    // silently fail here in case we do not have any mixer element
    return false;
}




bool AlsaSoundDevice::writeCaptureMixerVolume (const QString &channel, float &vol)
{
    if (vol > 1.0) vol = 1.0;
    if (vol < 0) vol = 0.0;

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
        // if we end up here, we have entered some mixer element but alsa either didn't 
        // find it or we could not successfully query the mixer status.
        logError("AlsaSound::writeCaptureMixerVolume: " +
                i18n("error while writing volume %1 to %2, channel %3", vol, m_CaptureMixerName, channel));
        return false;
    }
    // silently fail here in case we do not have any mixer element
    return false;
}


bool AlsaSoundDevice::writeCaptureMixerSwitch (const QString &channel, bool capture)
{
    if (m_CaptureChannelsSwitch2ID.contains(channel) && m_hCaptureMixer) {
        AlsaMixerElement sid = m_CaptureChannelsSwitch2ID[channel];
        snd_mixer_elem_t *elem = snd_mixer_find_selem(m_hCaptureMixer, sid);
        if (elem) {
            if (snd_mixer_selem_set_capture_switch_all(elem, capture) == 0) {
                return true;
            }
        }
        logError("AlsaSound::writeCaptureMixerSwitch: " +
                i18n("error while setting capture switch %1 for %2", channel, m_CaptureMixerName));
    }
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

    for (QMap<QString, AlsaConfigMixerSetting>::const_iterator it = m_CaptureMixerSettings.begin(); it != m_CaptureMixerSettings.end(); ++it) {
        const AlsaConfigMixerSetting &s = *it;
        if (s.mixerName() == m_CaptureMixerName && s.use()) {
            float vol = s.volume();
            if (m_CaptureChannels2ID.contains(s.name()))
                writeCaptureMixerVolume(s.name(), vol);
            if (m_CaptureChannelsSwitch2ID.contains(s.name()))
                writeCaptureMixerSwitch(s.name(), s.active());
        }
    }
}


void AlsaSoundDevice::setBufferSizes(size_t playback_size, size_t playback_chunk_size, size_t capture_size, size_t capture_chunk_size)
{
    m_PlaybackBufferSize = playback_size;
    m_PlaybackChunkSize  = playback_chunk_size;
    m_CaptureBufferSize  = capture_size;
    m_CaptureChunkSize   = capture_chunk_size;

    lockPlaybackBufferTransaction();
    m_PlaybackBuffer.resize(m_PlaybackBufferSize);
    unlockPlaybackBufferTransaction();

    lockCaptureBufferTransaction();
    m_CaptureBuffer.resize(m_CaptureBufferSize);
    unlockCaptureBufferTransaction();
}


void AlsaSoundDevice::setNonBlockingFlags(bool playback_flag, bool capture_flag)
{
    m_nonBlockingPlayback = playback_flag;
    m_nonBlockingCapture  = capture_flag;
}


void AlsaSoundDevice::enablePlayback(bool on)
{
    if (m_EnablePlayback != on) {
        m_EnablePlayback = on;
        getPlaybackMixerChannels(m_PlaybackMixerName,
                                 m_hPlaybackMixer,
                                 m_PlaybackChannels,
                                 m_PlaybackChannels2ID,
                                 isPlaybackEnabled()
                                );
        notifyPlaybackChannelsChanged(m_SoundStreamClientID, m_PlaybackChannels);
    }
}


void AlsaSoundDevice::enableCapture(bool on)
{
    if (m_EnableCapture != on) {
        m_EnableCapture = on;
        getCaptureMixerChannels(m_CaptureMixerName,
                                m_hCaptureMixer,
                                m_CaptureChannels,
                                m_CaptureChannels2ID,
                                m_CaptureChannelsSwitch,
                                m_CaptureChannelsSwitch2ID,
                                NULL,
                                isCaptureEnabled()
                               );
        notifyCaptureChannelsChanged(m_SoundStreamClientID,  m_CaptureChannels);
    }
}


void AlsaSoundDevice::setSoftPlaybackVolume(bool enable, double correction_factor)
{
    m_SoftPlaybackVolumeCorrectionFactor = correction_factor;
    m_SoftPlaybackVolumeEnabled          = enable;
}



bool AlsaSoundDevice::getCaptureFormatOverride(SoundFormat &sf)
{
    sf = m_CaptureFormatOverride;
    return m_CaptureFormatOverrideEnable;
}

void AlsaSoundDevice::setCaptureFormatOverride(bool override_enabled, const SoundFormat &sf)
{
    m_CaptureFormatOverride       = sf;
    m_CaptureFormatOverrideEnable = override_enabled;
}

void AlsaSoundDevice::setPlaybackDevice(const QString &deviceName, bool force)
{
    if ((m_PlaybackDeviceName == deviceName) && !force)
        return;

    m_PlaybackDeviceName = deviceName;
    SoundFormat f        = m_PlaybackFormat;
    if (m_hPlayback)
        openPlaybackDevice(f, /* reopen = */ true);
}


void AlsaSoundDevice::setPlaybackMixer(const QString &mixerName, bool force)
{
    if ((m_PlaybackMixerName == mixerName) && !force)
        return;

    m_PlaybackMixerName = mixerName;
    if (m_hPlaybackMixer)
        openPlaybackMixerDevice(/* reopen = */ true);

    getPlaybackMixerChannels(m_PlaybackMixerName,
                             m_hPlaybackMixer,
                             m_PlaybackChannels,
                             m_PlaybackChannels2ID,
                             isPlaybackEnabled()
                            );
    notifyPlaybackChannelsChanged(m_SoundStreamClientID, m_PlaybackChannels);
}





void AlsaSoundDevice::setCaptureDevice(const QString &deviceName, bool force)
{
    if ((m_CaptureDeviceName == deviceName) && !force)
        return;

    m_CaptureDeviceName = deviceName;
    SoundFormat f       = m_CaptureFormat;
    if (m_hCapture)
        openCaptureDevice(f, /* reopen = */ true);
}


void AlsaSoundDevice::setCaptureMixer(const QString &mixerName, bool force)
{
    if ((m_CaptureMixerName == mixerName) && !force)
        return;

    m_CaptureMixerName = mixerName;
    if (m_hCaptureMixer)
        openCaptureMixerDevice(/* reopen = */ true);

    getCaptureMixerChannels(m_CaptureMixerName,
                            m_hCaptureMixer,
                            m_CaptureChannels,
                            m_CaptureChannels2ID,
                            m_CaptureChannelsSwitch,
                            m_CaptureChannelsSwitch2ID,
                            NULL,
                            isCaptureEnabled()
                           );
    notifyCaptureChannelsChanged(m_SoundStreamClientID,  m_CaptureChannels);
}


QString AlsaSoundDevice::getSoundStreamClientDescription() const
{
    return i18n("ALSA Sound Device %1", PluginBase::name());
}


bool AlsaSoundDevice::muteSink (SoundStreamID id, bool mute)
{
    if (id.isValid() && (m_PlaybackStreamID == id || m_PassivePlaybackStreams.contains(id))) {
        SoundStreamConfig &cfg = m_PlaybackStreams[id];
        logDebug(QString("AlsaSoundDevice::muteSink: ch=%1, old=%2, new=%3").arg(cfg.m_Channel).arg(cfg.m_Muted ? "muted" : "unmuted").arg(mute ? "muted" : "unmuted"));
        if (mute != cfg.m_Muted) {
            if (writePlaybackMixerVolume(cfg.m_Channel, cfg.m_Volume, cfg.m_Muted = mute)) {
                notifySinkMuted(id, cfg.m_Muted);
            }
        }
        return true;
    }
    return false;
}

bool AlsaSoundDevice::unmuteSink (SoundStreamID id, bool unmute)
{
    return muteSink(id, !unmute);
}

bool AlsaSoundDevice::isSinkMuted(SoundStreamID id, bool &m) const
{
    if (id.isValid() && (m_PlaybackStreamID == id || m_PassivePlaybackStreams.contains(id))) {
        const SoundStreamConfig &cfg = m_PlaybackStreams[id];
        m = cfg.m_Muted;
        return true;
    }
    return false;
}


bool AlsaSoundDevice::muteSourcePlayback (SoundStreamID id, bool mute)
{
    if (id.isValid() && m_CaptureStreams.contains(id)) {
        SoundStreamConfig &cfg = m_CaptureStreams[id];

        // only do something if the source channel is also available for playback
        if (m_PlaybackChannels2ID.contains(cfg.m_Channel)) {

            bool  m = false;
            float v = readPlaybackMixerVolume(cfg.m_Channel, m);
            logDebug(QString("AlsaSoundDevice::muteSourcePlayback: ch=%1, old=%2, new=%3").arg(cfg.m_Channel).arg(m ? "muted" : "unmuted").arg(mute ? "muted" : "unmuted"));
            if (mute != m) {
                if (writePlaybackMixerVolume(cfg.m_Channel, v, mute)) {
                    notifySourcePlaybackMuted(id, cfg.m_Muted);
                }
                else {
                    return false;
                }
            }
        }
        return true;
    }
    return false;
}


bool AlsaSoundDevice::unmuteSourcePlayback (SoundStreamID id, bool unmute)
{
    return unmuteSource(id, !unmute);
}


bool AlsaSoundDevice::isSourcePlaybackMuted(SoundStreamID id, bool &m) const
{
    if (id.isValid() && m_CaptureStreams.contains(id)) {
        const SoundStreamConfig &cfg = m_CaptureStreams[id];
        m = false;
        readPlaybackMixerVolume(cfg.m_Channel, m);
        return true;
    }
    return false;
}


void AlsaSoundDevice::setCaptureMixerSettings(const QMap<QString, AlsaConfigMixerSetting> &map)
{
    m_CaptureMixerSettings = map;
}












char *AlsaSoundDevice::getPlaybackData(size_t &buffersize, size_t &maxAvailableData)
{
    buffersize       = 0;
    char *buffer     = m_PlaybackBuffer.getData(buffersize);
    // buffersize       = qMin(buffersize, m_PlaybackChunkSize);
    maxAvailableData = m_PlaybackBuffer.getFillSize();

    return buffer;
}


void  AlsaSoundDevice::freePlaybackData(size_t bytes)
{
    m_PlaybackBuffer.removeData(bytes);
}


void   AlsaSoundDevice::setWaitForMinPlaybackBufferFill(int percent)
{
    m_PlaybackBufferWaitForMinFill = percent;
}

size_t AlsaSoundDevice::getPlaybackBufferMinFill() const
{
    return m_PlaybackBufferWaitForMinFill > 0 ? m_PlaybackBufferWaitForMinFill * m_PlaybackBuffer.getSize() / 100 : 1;
}


char *AlsaSoundDevice::getFreeCaptureBuffer(size_t &bufsize)
{
    bufsize      = 0;
    char *buffer =  m_CaptureBuffer.getFreeSpace(bufsize);
    bufsize      = qMin(bufsize, m_CaptureChunkSize);
    return buffer;
}

void  AlsaSoundDevice::removeFreeCaptureBufferSpace(size_t bytesRead)
{
    m_CaptureBuffer.removeFreeSpace(bytesRead);
}



void   AlsaSoundDevice::lockPlaybackBufferTransaction()
{
    m_PlaybackBuffer.lockTransaction();
}

void   AlsaSoundDevice::unlockPlaybackBufferTransaction()
{
    m_PlaybackBuffer.unlockTransaction();
}

void   AlsaSoundDevice::lockCaptureBufferTransaction()
{
    m_CaptureBuffer.lockTransaction();
}

void   AlsaSoundDevice::unlockCaptureBufferTransaction()
{
    m_CaptureBuffer.unlockTransaction();
}





void  AlsaSoundDevice::checkThreadErrorsAndWarning()
{
    if (m_captureThread) {
        checkLogs(m_captureThread, i18n("AlsaPlugin(capture thread on %1): ",   m_CaptureDeviceName),    /*resetLogs = */ true);
        m_captureThread->resetError();
    }
    if (m_playbackThread) {
        checkLogs(m_playbackThread, i18n("AlsaPlugin(playback thread on %1): ", m_PlaybackDeviceName), /*resetLogs = */ true);
        m_playbackThread->resetError();
    }
}


void AlsaSoundDevice::setName(const QString &n)
{
    PluginBase::setName(n);
    notifyPlaybackChannelsChanged(m_SoundStreamClientID, m_PlaybackChannels);
    notifyCaptureChannelsChanged (m_SoundStreamClientID, m_CaptureChannels);
}





QList<AlsaSoundDeviceMetaData> AlsaSoundDevice::getPCMCaptureDeviceDescriptions()
{
    return getPCMDeviceDescriptions("Input");
}

QList<AlsaSoundDeviceMetaData> AlsaSoundDevice::getPCMPlaybackDeviceDescriptions()
{
    return getPCMDeviceDescriptions("Output");
}


#define ALSA_BUG_3461_WORKAROUND
// see https://bugtrack.alsa-project.org/alsa-bug/view.php?id=3461
// snd_device_name_hint will only return correct list at first call.

QList<AlsaSoundDeviceMetaData> AlsaSoundDevice::getPCMDeviceDescriptions(const QString &filter)
{
    void                    **hints = NULL;
#ifdef ALSA_BUG_3461_WORKAROUND
    static
#endif
    QMap<QString, QList<AlsaSoundDeviceMetaData> >    descriptions;  // key = DeviceName,

#ifdef ALSA_BUG_3461_WORKAROUND
    if (descriptions[filter].size()) {
        return descriptions[filter];
    }
#endif

    descriptions[filter].append(AlsaSoundDeviceMetaData("default", "Default ALSA Device"));

    // let's do the clean up once globally as long as we need the workaround
    // otherwise we might miss some devices
#ifdef ALSA_BUG_3461_WORKAROUND
    snd_config_update_free_global();
#endif

    if (snd_device_name_hint(-1, "pcm", &hints) < 0)
        return descriptions[filter];

    for (void **current_hint = hints; *current_hint; ++current_hint) {
        char *name  = snd_device_name_get_hint(*current_hint, "NAME");
        char *descr = snd_device_name_get_hint(*current_hint, "DESC");
        char *dir   = snd_device_name_get_hint(*current_hint, "IOID");
        if (!dir || filter == dir) {
            descriptions[filter].append(AlsaSoundDeviceMetaData(name, descr));
        }
        if (name != NULL) {
            free(name);
        }
        if (descr != NULL) {
            free(descr);
        }
        if (dir != NULL) {
            free(dir);
        }
    }
    snd_device_name_free_hint(hints);

    // the following line is prohibitiv - having pcm devices opend will crash kradio

    // one day of debugging later: BUT: if we don't do it after the first and only call,
    // the blah..._hint stuff will have deleted some entries from the config and thus
    // selecting some devices listed above will fail, e.g. for snd_pcm_open
#ifdef ALSA_BUG_3461_WORKAROUND
    snd_config_update_free_global();
#endif

    /* debugging code only
    QString devString;
    snd_mixer_t         *mixer     = NULL;
    int err = snd_mixer_open (&mixer, 0);
    if (!err) {
        foreach(devString, deviceStrings) {
            err = snd_mixer_attach (mixer, extractMixerName(devString).toLocal8Bit());
            if (!err) {
            }
            else {
                printf("%s\n", snd_strerror(err));
            }
            snd_mixer_detach(mixer, devString.toLocal8Bit());
        }
        snd_mixer_close(mixer);
    } else {
        printf("%s\n", snd_strerror(err));
    }
    */

    return descriptions[filter];
}



QList<AlsaMixerMetaData> AlsaSoundDevice::getCaptureMixerDescriptions()
{
    return getMixerDeviceDescriptions("Input");
}

QList<AlsaMixerMetaData> AlsaSoundDevice::getPlaybackMixerDescriptions()
{
    return getMixerDeviceDescriptions("Output");
}

QList<AlsaMixerMetaData> AlsaSoundDevice::getMixerDeviceDescriptions(const QString &filter)
{
    QList<AlsaSoundDeviceMetaData> pcms = getPCMDeviceDescriptions(filter);

    QList<AlsaMixerMetaData> ret;
    QMap<QString, bool> already_handled;

    AlsaSoundDeviceMetaData m;
    foreach (m, pcms) {
        if (!already_handled.contains(m.mixerCardName())) {
            ret.append(AlsaMixerMetaData(m));
            already_handled.insert(m.mixerCardName(), true);
        }
    }
    return ret;
}

QString AlsaSoundDevice::extractMixerNameFromPCMDevice(const QString &devString)
{
    QString mixerString = devString;

    int idx_colon = mixerString.indexOf(":");
    if (idx_colon >= 0) {
        mixerString = mixerString.mid(idx_colon + 1);
    }
    QString card_prefix = "CARD=";
    if (mixerString.startsWith(card_prefix)) {
        mixerString = mixerString.mid(card_prefix.length());
    }
    int idx_comma = mixerString.indexOf(",");
    if (idx_comma >= 0) {
        mixerString = mixerString.left(idx_comma);
    }

    if (mixerString != "default") {
        mixerString = "hw:" + mixerString;
    }
    return mixerString;
}






#include "alsa-sound.moc"
