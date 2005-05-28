/***************************************************************************
                          oss-sound.cpp  -  description
                             -------------------
    begin                : Sun Mar 21 2004
    copyright            : (C) 2004 by Martin Witte
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

#include "oss-sound.h"

#include "../../src/libkradio-gui/aboutwidget.h"
#include <klocale.h>
#include <kaboutdata.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>

#include "oss-sound-configuration.h"

///////////////////////////////////////////////////////////////////////
//// plugin library functions

PLUGIN_LIBRARY_FUNCTIONS(OSSSoundDevice, "Open Sound System (OSS) Support");

/////////////////////////////////////////////////////////////////////////////

struct _lrvol { unsigned char l, r; short dummy; };

OSSSoundDevice::OSSSoundDevice(const QString &name)
    : QObject(NULL, NULL),
      PluginBase(name, i18n("KRadio OSS Sound Plugin")),
      m_DSPDeviceName("/dev/dsp"),
      m_MixerDeviceName("/dev/mixer"),
      m_DSP_fd(-1),
      m_Mixer_fd(-1),
      m_DuplexMode(DUPLEX_UNKNOWN),
      m_DSPFormat(),
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
    QObject::connect(&m_PollingTimer, SIGNAL(timeout()), this, SLOT(slotPoll()));
}


OSSSoundDevice::~OSSSoundDevice()
{
    stopCapture(m_CaptureStreamID);
    stopPlayback(m_PlaybackStreamID);
    closeDSPDevice();
    closeMixerDevice();
}


bool OSSSoundDevice::connectI(Interface *i)
{
    bool a = PluginBase::connectI(i);
    bool b = ISoundStreamClient::connectI(i);
    return a || b;
}


bool OSSSoundDevice::disconnectI(Interface *i)
{
    bool a = PluginBase::disconnectI(i);
    bool b = ISoundStreamClient::disconnectI(i);
    return a || b;
}

void OSSSoundDevice::noticeConnectedI (ISoundStreamServer *s, bool pointer_valid)
{
    ISoundStreamClient::noticeConnectedI(s, pointer_valid);
    if (s && pointer_valid) {
        s->register4_sendPlaybackVolume(this);
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

void OSSSoundDevice::saveState (KConfig *c) const
{
    c->setGroup(QString("oss-sound-") + PluginBase::name());

    c->writeEntry("dsp-device",      m_DSPDeviceName);
    c->writeEntry("mixer-device",    m_MixerDeviceName);
    c->writeEntry("enable-playback", m_EnablePlayback);
    c->writeEntry("enable-capture",  m_EnableCapture);
    c->writeEntry("buffer-size",     m_BufferSize);
    c->writeEntry("soundstreamclient-id", m_SoundStreamClientID);
}


void OSSSoundDevice::restoreState (KConfig *c)
{
    c->setGroup(QString("oss-sound-") + PluginBase::name());

    setSoundStreamClientID(c->readEntry("soundstreamclient-id", getSoundStreamClientID()));

    m_EnablePlayback  = c->readBoolEntry("enable-playback",  true);
    m_EnableCapture   = c->readBoolEntry("enable-capture",   true);
    m_BufferSize      = c->readNumEntry ("buffer-size",      65536);

    setDSPDeviceName   (c->readEntry    ("dsp-device",       "/dev/dsp"));
    setMixerDeviceName (c->readEntry    ("mixer-device",     "/dev/mixer"));

    m_PlaybackBuffer.resize(m_BufferSize);
    m_CaptureBuffer.resize(m_BufferSize);

    emit sigUpdateConfig();
}


void OSSSoundDevice::setMixerDeviceName(const QString &dev_name)
{
    m_MixerDeviceName  = dev_name;
    if (m_Mixer_fd >= 0)
        openMixerDevice(true);
    getMixerChannels(SOUND_MIXER_DEVMASK, m_PlaybackChannels, m_revPlaybackChannels);
    getMixerChannels(SOUND_MIXER_RECMASK, m_CaptureChannels, m_revCaptureChannels);
    notifyPlaybackChannelsChanged(m_SoundStreamClientID, m_PlaybackChannels);
    notifyCaptureChannelsChanged(m_SoundStreamClientID,  m_CaptureChannels);
}


ConfigPageInfo  OSSSoundDevice::createConfigurationPage()
{
    OSSSoundConfiguration *conf = new OSSSoundConfiguration(NULL, this);
    QObject::connect(this, SIGNAL(sigUpdateConfig()), conf, SLOT(slotUpdateConfig()));
    return ConfigPageInfo (conf,
                           i18n("OSS Sound Device Options"),
                           i18n("OSS Sound Device Options"),
                           "kmix");
}


AboutPageInfo OSSSoundDevice::createAboutPage()
{
/*    KAboutData aboutData("kradio",
                         NULL,
                         NULL,
                         I18N_NOOP("OSS Sound Plugin for KRadio"),
                         KAboutData::License_GPL,
                         "(c) 2004 Martin Witte",
                         0,
                         "http://sourceforge.net/projects/kradio",
                         0);
    aboutData.addAuthor("Martin Witte",  "", "witte@kawo1.rwth-aachen.de");

    return AboutPageInfo(
              new KRadioAboutWidget(aboutData, KRadioAboutWidget::AbtTabbed),
              i18n("OSS Sound"),
              i18n("OSS Sound"),
              "kradio_oss_sound"
           );
*/
    return AboutPageInfo();
}



bool OSSSoundDevice::preparePlayback(SoundStreamID id, const QString &channel, bool active_mode)
{
    if (id.isValid() && m_revPlaybackChannels.contains(channel)) {
        m_PlaybackStreams.insert(id, SoundStreamConfig(m_revPlaybackChannels[channel], active_mode));
        return true;
        // FIXME: what to do if stream is already playing?
    }
    return false;
}

bool OSSSoundDevice::prepareCapture(SoundStreamID id, const QString &channel)
{
    if (id.isValid() && m_revCaptureChannels.contains(channel)) {
        m_CaptureStreams.insert(id, SoundStreamConfig(m_revCaptureChannels[channel]));
        return true;
        // FIXME: what to do if stream is already playing?
    }
    return false;
}

bool OSSSoundDevice::supportsPlayback()   const
{
    return m_EnablePlayback;
}


bool OSSSoundDevice::supportsCapture() const
{
    return m_EnableCapture;
}


bool OSSSoundDevice::startPlayback(SoundStreamID id)
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
            openMixerDevice();
            if (cfg.m_Volume >= 0)
                writeMixerVolume(cfg.m_Channel, cfg.m_Volume);
        }

        // error handling?
        return true;
    } else {
        return false;
    }
}


bool OSSSoundDevice::pausePlayback(SoundStreamID /*id*/)
{
    //return stopPlayback(id);
    return false;
}


bool OSSSoundDevice::stopPlayback(SoundStreamID id)
{
    if (id.isValid() && m_PlaybackStreams.contains(id)) {

        SoundStreamConfig &cfg = m_PlaybackStreams[id];

        if (!cfg.m_ActiveMode) {
            if  (m_PassivePlaybackStreams.contains(id)) {
                writeMixerVolume(cfg.m_Channel, 0);
                m_PassivePlaybackStreams.remove(id);
            }
        } else if (m_PlaybackStreamID == id) {
            m_PlaybackStreamID = SoundStreamID::InvalidID;
            m_PlaybackBuffer.clear();
            closeDSPDevice();
        }

        closeMixerDevice();
        return true;
    } else {
        return false;
    }
}

bool OSSSoundDevice::isPlaybackRunning(SoundStreamID id, bool &b) const
{
    if (id.isValid() && m_PlaybackStreams.contains(id)) {
        b = true;
        return true;
    } else {
        return false;
    }
}

bool OSSSoundDevice::startCaptureWithFormat(SoundStreamID      id,
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

            openMixerDevice();
            selectCaptureChannel(cfg.m_Channel);
            if (cfg.m_Volume >= 0)
                writeMixerVolume(cfg.m_Channel, cfg.m_Volume);

            openDSPDevice(proposed_format);

            // FIXME: error handling?
        }

        real_format = m_DSPFormat;
        m_CaptureRequestCounter++;

        return true;
    } else {
        return false;
    }
}


bool OSSSoundDevice::stopCapture(SoundStreamID id)
{
    if (id.isValid() && m_CaptureStreamID == id) {

        if (--m_CaptureRequestCounter == 0) {
            m_CaptureStreamID = SoundStreamID::InvalidID;
            m_CaptureBuffer.clear();

            closeMixerDevice();
            closeDSPDevice();
        }
        return true;
    } else {
        return false;
    }
}


bool OSSSoundDevice::isCaptureRunning(SoundStreamID id, bool &b) const
{
    if (id.isValid() && m_CaptureStreamID == id) {
        b = true;
        return true;
    } else {
        return false;
    }
}


bool OSSSoundDevice::noticeSoundStreamClosed(SoundStreamID id)
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


bool OSSSoundDevice::noticeSoundStreamRedirected(SoundStreamID oldID, SoundStreamID newID)
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


bool OSSSoundDevice::noticeSoundStreamData(SoundStreamID id,
                                           const SoundFormat &format,
                                           const char *data, unsigned size,
                                           const SoundMetaData &/*md*/
                                          )
{
    if (!id.isValid() || id != m_PlaybackStreamID)
        return false;

    if (m_DSP_fd < 0) {
        openDSPDevice(format);
    } else if (format != m_DSPFormat) {
        if (m_CaptureStreamID.isValid())
            return false;

        // flush playback buffer
        unsigned buffersize = 0;
        char *buffer = m_PlaybackBuffer.getData(buffersize);
        write(m_DSP_fd, buffer, buffersize);

        // if not all could be written, it must be discarded
        m_PlaybackBuffer.clear();

        closeDSPDevice();
        openDSPDevice(format);
        // error handling ?
    }

    unsigned n = m_PlaybackBuffer.addData(data, size);
    if (n < size) {
        m_PlaybackSkipCount += size - n;
    } else if (m_PlaybackSkipCount > 0) {
        logWarning(i18n("%1: Playback buffer overflow. Skipped %1 bytes").arg(m_DSPDeviceName).arg(QString::number(m_PlaybackSkipCount)));
        m_PlaybackSkipCount = 0;
    }

    return m_PlaybackSkipCount == 0;
}



void OSSSoundDevice::slotPoll()
{
    int err = 0;

    if (m_CaptureStreamID.isValid() && m_DSP_fd >= 0) {

        unsigned bufferSize = 0;
        char *buffer = m_CaptureBuffer.getFreeSpace(bufferSize);

        int bytesRead = read(m_DSP_fd, buffer, bufferSize);

        if (bytesRead > 0) {
            m_CaptureBuffer.removeFreeSpace(bytesRead);
        } else if (bytesRead < 0 && errno == EAGAIN) {
            bytesRead = 0;
        } else if (bytesRead == 0) {
            err = -1;
            logError(i18n("OSS device %1: No data to record").arg(m_DSPDeviceName));
        } else {
            err = errno;
        }

        while (m_CaptureBuffer.getFillSize() > m_CaptureBuffer.getSize() / 3) {
            unsigned size = 0;
            buffer = m_CaptureBuffer.getData(size);
            time_t cur_time = time(NULL);
            notifySoundStreamData(m_CaptureStreamID, m_DSPFormat, buffer, size, SoundMetaData(m_CapturePos, cur_time - m_CaptureStartTime, cur_time, m_DSPDeviceName));
            m_CaptureBuffer.removeData(size);
            m_CapturePos += size;
        }
    }

    if (m_PlaybackStreamID.isValid()/* && m_DSP_fd >= 0*/) {

        if (m_PlaybackBuffer.getFillSize() > 0 && m_DSP_fd >= 0) {

            unsigned buffersize = 0;
            char *buffer = m_PlaybackBuffer.getData(buffersize);
            int bytesWritten  = write(m_DSP_fd, buffer, buffersize);

            if (bytesWritten > 0) {
                m_PlaybackBuffer.removeData(bytesWritten);
            } else if (bytesWritten < 0 && errno == EAGAIN) {
                bytesWritten = 0;
            } else {
                err = errno;
            }
        }

        if (m_PlaybackBuffer.getFreeSize() > 0)
            notifyReadyForPlaybackData(m_PlaybackStreamID, m_PlaybackBuffer.getFreeSize());
    }

    if (err) {
        logError(i18n("Error %1 while handling OSS device %2").arg(QString().setNum(err)).arg(m_DSPDeviceName));
    }

    if (m_PlaybackStreamID.isValid())
        checkMixerVolume(m_PlaybackStreamID);
    if (m_CaptureStreamID.isValid())
        checkMixerVolume(m_CaptureStreamID);

    QValueListConstIterator<SoundStreamID> end = m_PassivePlaybackStreams.end();
    for (QValueListConstIterator<SoundStreamID> it = m_PassivePlaybackStreams.begin(); it != end; ++it)
        checkMixerVolume(*it);

}


bool OSSSoundDevice::openDSPDevice(const SoundFormat &format, bool reopen)
{
    if (m_DSP_fd >= 0) {

        if (reopen) {

            closeDSPDevice ( /* force = */ true);

        } else {

            if (format != m_DSPFormat)
                return false;

            if (m_DuplexMode != DUPLEX_FULL && m_CaptureStreamID.isValid() && m_PlaybackStreamID.isValid())
                return false;

            return true;
        }
    } else {
        if (reopen)
            return true;
    }

    m_DSPFormat = format;

    // first testopen for CAPS
    m_DSP_fd = open(m_DSPDeviceName.ascii(), O_NONBLOCK | O_RDONLY);
    bool err = m_DSP_fd < 0;
    if (err) {
        logError(i18n("Cannot open DSP device %1").arg(m_DSPDeviceName));
        return false;
    }
    int caps = 0;
    err |= (ioctl (m_DSP_fd, SNDCTL_DSP_GETCAPS, &caps) != 0);
    if (err)
        logError(i18n("Cannot read DSP capabilities for %1").arg(m_DSPDeviceName));

    m_DuplexMode = (caps & DSP_CAP_DUPLEX) ? DUPLEX_FULL : DUPLEX_HALF;
    close (m_DSP_fd);
    m_DSP_fd = -1;

    // opening and seeting up the device file
    int mode = O_NONBLOCK;
    if (m_DuplexMode == DUPLEX_FULL) {
        mode |= O_RDWR;
    } else if (m_CaptureStreamID.isValid()) {
        mode |= O_RDONLY;
    } else {
        mode |= O_WRONLY;
    }

    m_DSP_fd = open(m_DSPDeviceName.ascii(), mode);

    err = m_DSP_fd < 0;
    if (err) {
        logError(i18n("Cannot open DSP device %1").arg(m_DSPDeviceName));
        return false;
    }

    int oss_format = getOSSFormat(m_DSPFormat);
    err |= (ioctl(m_DSP_fd, SNDCTL_DSP_SETFMT, &oss_format) != 0);
    if (err)
        logError(i18n("Cannot set DSP sample format for %1").arg(m_DSPDeviceName));

    int channels = m_DSPFormat.m_Channels;
    err |= (ioctl(m_DSP_fd, SNDCTL_DSP_CHANNELS, &channels) != 0);
    if (err)
        logError(i18n("Cannot set number of channels for %1").arg(m_DSPDeviceName));

    int rate = m_DSPFormat.m_SampleRate;
    err |= (ioctl(m_DSP_fd, SNDCTL_DSP_SPEED, &rate) != 0);
    if (err)
        logError(i18n("Cannot set sampling rate for %1").arg(m_DSPDeviceName));
    if (rate != (int)m_DSPFormat.m_SampleRate) {
        logWarning(i18n("Asking for %1 Hz but %2 uses %3 Hz").
                   arg(QString::number(m_DSPFormat.m_SampleRate)).
                   arg(m_DSPDeviceName).
                   arg(QString::number(rate)));
        m_DSPFormat.m_SampleRate = rate;
    }

    int stereo = m_DSPFormat.m_Channels == 2;
    err |= (ioctl(m_DSP_fd, SNDCTL_DSP_STEREO, &stereo) != 0);
    if (err)
        logError(i18n("Cannot set stereo mode for %1").arg(m_DSPDeviceName));

    unsigned sampleSize = m_DSPFormat.m_SampleBits;
    err |= (ioctl(m_DSP_fd, SNDCTL_DSP_SAMPLESIZE, &sampleSize) != 0);
    if (err || sampleSize != m_DSPFormat.m_SampleBits)
        logError(i18n("Cannot set sample size for %1").arg(m_DSPDeviceName));

    // setup buffer, ask for 40ms latency
    int tmp  = (400 * m_DSPFormat.frameSize() * m_DSPFormat.m_SampleRate) / 1000;
    int mask = -1;    for (; tmp; tmp >>= 1) ++mask;
    if (mask < 8) mask = 12;  // default 4kB
    mask |= 0x7FFF0000;
    err |= ioctl (m_DSP_fd, SNDCTL_DSP_SETFRAGMENT, &mask);
    if (err)
        logError(i18n("Cannot set buffers for %1").arg(m_DSPDeviceName));

    int bufferBlockSize = 0;
    err |= ioctl (m_DSP_fd, SNDCTL_DSP_GETBLKSIZE, &bufferBlockSize);
    if (err) {
        logError(i18n("Cannot read buffer size for %1").arg(m_DSPDeviceName));
    } else {
        logInfo(i18n("%1 uses buffer blocks of %2 bytes").arg(m_DSPDeviceName).arg(QString::number(bufferBlockSize)));
        unsigned tmp = (((m_BufferSize - 1) / bufferBlockSize) + 1) * bufferBlockSize;
        setBufferSize(tmp);
        logInfo(i18n("adjusted own buffer size to %1 bytes").arg(QString::number(tmp)));
    }

    int trigger = ~PCM_ENABLE_INPUT & ~PCM_ENABLE_OUTPUT;
    ioctl(m_DSP_fd, SNDCTL_DSP_SETTRIGGER, &trigger);
    trigger = PCM_ENABLE_INPUT | PCM_ENABLE_OUTPUT;
    ioctl(m_DSP_fd, SNDCTL_DSP_SETTRIGGER, &trigger);

    if (!err) {
        m_PollingTimer.start(40);
    } else {
        closeDSPDevice();
    }

    m_CaptureSkipCount  = 0;
    m_PlaybackSkipCount = 0;

    return !err;
}


bool OSSSoundDevice::closeDSPDevice(bool force)
{
    if ((!m_PlaybackStreamID.isValid() && !m_CaptureStreamID.isValid()) || force) {

        if (m_Mixer_fd < 0)
            m_PollingTimer.stop();

        if (m_DSP_fd >= 0)
            close (m_DSP_fd);
        m_DSP_fd = -1;

        m_PlaybackBuffer.clear();
        m_CaptureBuffer.clear();
    }
    return true;
}


bool OSSSoundDevice::openMixerDevice(bool reopen)
{
    if (reopen) {
        if (m_Mixer_fd >= 0)
            closeMixerDevice(/* force = */ true);
        else
            return true;
    }

    if (m_Mixer_fd < 0)
        m_Mixer_fd = open(m_MixerDeviceName.ascii(), O_RDONLY);

    if (m_Mixer_fd < 0) {
        logError(i18n("Cannot open mixer device %1").arg(m_MixerDeviceName));
    } else {
        m_PollingTimer.start(40);
    }
    return m_Mixer_fd >= 0;
}


bool OSSSoundDevice::closeMixerDevice(bool force)
{
    if ((!m_PlaybackStreamID.isValid() && !m_CaptureStreamID.isValid()) || force) {

        if (m_DSP_fd < 0)
            m_PollingTimer.stop();

        if (m_Mixer_fd >= 0)
            close (m_Mixer_fd);
        m_Mixer_fd = -1;
    }
    return m_Mixer_fd < 0;
}


void OSSSoundDevice::getMixerChannels(int query, QStringList &retval, QMap<QString, int> &revmap) const
{
    retval.clear();
    revmap.clear();

    int fd = m_Mixer_fd;
    if (fd < 0)
        fd = open(m_MixerDeviceName.ascii(), O_RDONLY);

    if (fd < 0) {
        logError(i18n("OSSSoundDevice::getMixerChannels: Cannot open mixer device %1").arg(m_MixerDeviceName));
    }

    if (fd >= 0) {
        int mask = 0;
        if ( ioctl(fd, MIXER_READ(query), &mask) == 0 ) {
            for (int i = 0; i < SOUND_MIXER_NRDEVICES; ++i) {
                if (mask & (1 << i)) {
                    static const char *labels[] = SOUND_DEVICE_LABELS;
                    retval.append(i18n(labels[i]));
                    revmap.insert(i18n(labels[i]), i);
                }
            }
        } else {
            logError(i18n("OSSSoundDevice::getMixerChannels: Cannot read mixer device mask on device %1").arg(m_MixerDeviceName));
        }
    }
    if (fd != m_Mixer_fd)
        close(fd);
}


const QStringList &OSSSoundDevice::getPlaybackChannels() const
{
    return m_PlaybackChannels;
}


const QStringList &OSSSoundDevice::getCaptureChannels() const
{
    return m_CaptureChannels;
}


bool OSSSoundDevice::setPlaybackVolume(SoundStreamID id, float volume)
{
    if (id.isValid() && (m_PlaybackStreamID == id || m_PassivePlaybackStreams.contains(id))) {
        SoundStreamConfig &cfg = m_PlaybackStreams[id];

        if (rint(100*volume) != rint(100*cfg.m_Volume)) {
            cfg.m_Volume = writeMixerVolume(cfg.m_Channel, volume);
            notifyPlaybackVolumeChanged(id, cfg.m_Volume);
        }
        return true;
    }
    return false;
}


bool OSSSoundDevice::setCaptureVolume(SoundStreamID id, float volume)
{
    if (id.isValid() && m_CaptureStreamID == id) {
        SoundStreamConfig &cfg = m_CaptureStreams[id];

        if (rint(100*volume) != rint(100*cfg.m_Volume)) {
            cfg.m_Volume = writeMixerVolume(cfg.m_Channel, volume);
            notifyCaptureVolumeChanged(id, cfg.m_Volume);
        }
        return true;
    }
    return false;
}


bool OSSSoundDevice::getPlaybackVolume(SoundStreamID id, float &volume) const
{
    if (id.isValid() && (m_PlaybackStreamID == id || m_PassivePlaybackStreams.contains(id))) {
        const SoundStreamConfig &cfg = m_PlaybackStreams[id];
        volume = cfg.m_Volume;
        return true;
    }
    return false;
}


bool OSSSoundDevice::getCaptureVolume(SoundStreamID id, float &volume) const
{
    if (id.isValid() && m_CaptureStreamID == id) {
        const SoundStreamConfig &cfg = m_CaptureStreams[id];
        volume = cfg.m_Volume;
        return true;
    }
    return false;
}


void OSSSoundDevice::checkMixerVolume(SoundStreamID id)
{
    if (m_Mixer_fd >= 0 && id.isValid()) {

        if (m_PassivePlaybackStreams.contains(id) || m_PlaybackStreamID == id) {
            SoundStreamConfig &cfg = m_PlaybackStreams[id];

            float v = readMixerVolume(cfg.m_Channel);
            if (rint(100*cfg.m_Volume) != rint(100*v)) {
                cfg.m_Volume = v;
                notifyPlaybackVolumeChanged(id, v);
            }
        }

        if (m_CaptureStreamID == id) {
            SoundStreamConfig &cfg = m_CaptureStreams[id];

            float v = readMixerVolume(cfg.m_Channel);
            if (rint(100*cfg.m_Volume) != rint(100*v)) {
                cfg.m_Volume = v;
                notifyCaptureVolumeChanged(id, v);
            }
        }
    }
}


float OSSSoundDevice::readMixerVolume(int channel) const
{
    _lrvol tmpvol;
    int err = ioctl(m_Mixer_fd, MIXER_READ(channel), &tmpvol);
    if (err) {
        logError("OSSSound::readMixerVolume: " +
                i18n("error %1 while reading volume from %2")
                .arg(QString().setNum(err))
                .arg(m_MixerDeviceName));
        tmpvol.l = tmpvol.r = 0;
    }
    return float(tmpvol.l) / 100.0;
}


float OSSSoundDevice::writeMixerVolume (int channel, float vol)
{
    if (vol > 1.0) vol = 1.0;
    if (vol < 0) vol = 0.0;

    const int divs = 100;
    vol = rint(vol * divs) / float(divs);

    if (m_Mixer_fd >= 0) {
        _lrvol tmpvol;
        tmpvol.r = tmpvol.l = (unsigned int)(rint(vol * divs));
        int err = ioctl(m_Mixer_fd, MIXER_WRITE(channel), &tmpvol);
        if (err != 0) {
            logError("OSSSoundDevice::writeMixerVolume: " +
                    i18n("error %1 while setting volume to %2 on device %3")
                    .arg(QString().setNum(err))
                    .arg(QString().setNum(vol))
                    .arg(m_MixerDeviceName));
            return -1;
        }
    }
    return vol;
}


void OSSSoundDevice::selectCaptureChannel (int channel)
{
    int x = 1 << channel;
    int err = ioctl(m_Mixer_fd, SOUND_MIXER_WRITE_RECSRC, &x);
    if (err)
        logError(i18n("Selecting recording source on device %1 failed with error code %2")
                 .arg(m_MixerDeviceName)
                 .arg(QString::number(err)));
    _lrvol tmpvol;
    err = ioctl(m_Mixer_fd, MIXER_READ(SOUND_MIXER_IGAIN), &tmpvol);
    if (err)
        logError(i18n("Reading igain volume on device %1 failed with error code %2")
                 .arg(m_MixerDeviceName)
                 .arg(QString::number(err)));
    if (tmpvol.r == 0 && tmpvol.l == 0) {
        tmpvol.r = tmpvol.l = 1;
        err = ioctl(m_Mixer_fd, MIXER_WRITE(SOUND_MIXER_IGAIN), &tmpvol);
        if (err)
            logError(i18n("Setting igain volume on device %1 failed with error code %2")
                     .arg(m_MixerDeviceName)
                     .arg(QString::number(err)));
    }
}


int OSSSoundDevice::getOSSFormat(const SoundFormat &f)
{
    if (f.m_SampleBits == 16) {
        switch (2 * f.m_IsSigned + (f.m_Endianess == LITTLE_ENDIAN)) {
            case 0: return AFMT_U16_BE;
            case 1: return AFMT_U16_LE;
            case 2: return AFMT_S16_BE;
            case 3: return AFMT_S16_LE;
        }
    }
    if (f.m_SampleBits == 8) {
        switch (f.m_IsSigned) {
            case 0: return AFMT_U8;
            case 1: return AFMT_S8;
        }
    }
    return 0;
}


void OSSSoundDevice::setBufferSize(int s)
{
    m_BufferSize = s;
    m_PlaybackBuffer.resize(m_BufferSize);
    m_CaptureBuffer.resize(m_BufferSize);
}


void OSSSoundDevice::enablePlayback(bool on)
{
    m_EnablePlayback = on;
}


void OSSSoundDevice::enableCapture(bool on)
{
    m_EnableCapture = on;
}


void OSSSoundDevice::setDSPDeviceName(const QString &s)
{
    m_DSPDeviceName = s;
    SoundFormat f = m_DSPFormat;
    if (m_DSP_fd >= 0)
        openDSPDevice(f, /* reopen = */ true);
}


QString OSSSoundDevice::getSoundStreamClientDescription() const
{
    return i18n("OSS Sound Device %1").arg(PluginBase::name());
}



#include "oss-sound.moc"
