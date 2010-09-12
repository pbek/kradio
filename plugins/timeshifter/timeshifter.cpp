/***************************************************************************
                          timeshifter.cpp  -  description
                             -------------------
    begin                : Mon May 16 13:39:31 CEST 2005
    copyright            : (C) 2005 by Ernst Martin Witte
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

#include <klocale.h>
#include <kuser.h>
#include <linux/soundcard.h>

#include "utils.h"
#include "timeshifter.h"
#include "timeshifter-configuration.h"

///////////////////////////////////////////////////////////////////////

PLUGIN_LIBRARY_FUNCTIONS(TimeShifter, PROJECT_NAME, i18n("TimeShift Support"));

///////////////////////////////////////////////////////////////////////

TimeShifter::TimeShifter (const QString &instanceID, const QString &name)
    : PluginBase(instanceID, name, i18n("TimeShifter Plugin")),
      m_TempFileMaxSize(256*1024*1024),
      m_PlaybackMixerID(QString::null),
      m_PlaybackMixerChannel("PCM"),
      m_orgVolume(0.0),
      m_PlaybackMetaData(0,0,0),
      m_PlaybackDataLeftInBuffer(0),
      m_RingBuffer(m_TempFileName, m_TempFileMaxSize),
      m_currentQuality(0.0),
      m_currentGoodQuality(false),
      m_currentStereo(false)
{
    KUser userid;
    m_TempFileName = "/tmp/" + userid.loginName() + "-kradio-timeshifter-tempfile";
}


TimeShifter::~TimeShifter ()
{
}


bool TimeShifter::connectI (Interface *i)
{
    bool a = PluginBase::connectI(i);
    bool b = ISoundStreamClient::connectI(i);
    return a || b;
}


bool TimeShifter::disconnectI (Interface *i)
{
    bool a = PluginBase::disconnectI(i);
    bool b = ISoundStreamClient::disconnectI(i);
    return a || b;
}


void TimeShifter::noticeConnectedI (ISoundStreamServer *s, bool pointer_valid)
{
    ISoundStreamClient::noticeConnectedI(s, pointer_valid);
    if (s && pointer_valid) {
        s->register4_notifySoundStreamClosed(this);
        s->register4_sendStartPlayback(this);
        s->register4_sendStopPlayback(this);
        s->register4_sendPausePlayback(this);
        s->register4_sendResumePlayback(this);
        s->register4_notifySoundStreamData(this);
        s->register4_notifyReadyForPlaybackData(this);
        s->register4_querySoundStreamDescription(this);
        s->register4_sendStartCaptureWithFormat(this);
        s->register4_sendStopCapture(this);
        s->register4_queryIsPlaybackPaused(this);
        s->register4_querySoundStreamRadioStation(this);
    }
}


void   TimeShifter::saveState    (KConfigGroup &config) const
{
    PluginBase::saveState(config);

    config.writeEntry("temp-file-name",       m_TempFileName);
    config.writeEntry("max-file-size",        (quint64)m_TempFileMaxSize / 1024 / 1024);

    config.writeEntry("PlaybackMixerID",      m_PlaybackMixerID);
    config.writeEntry("PlaybackMixerChannel", m_PlaybackMixerChannel);
}


void   TimeShifter::restoreState (const KConfigGroup &config)
{
    PluginBase::restoreState(config);

    KUser    userid;
    QString  defTempFileName = "/tmp/" + userid.loginName() + "-kradio-timeshifter-tempfile";
    QString  fname =               config.readEntry("temp-file-name", defTempFileName);
    quint64  fsize = 1024 * 1024 * config.readEntry("max-file-size",  (quint64)256);

    QString mixerID = config.readEntry ("PlaybackMixerID", QString());
    QString channel = config.readEntry ("PlaybackMixerChannel", "PCM");

    setPlaybackMixer(mixerID, channel);
    setTempFile(fname, fsize);

    emit sigUpdateConfig();
}


ConfigPageInfo  TimeShifter::createConfigurationPage()
{
    TimeShifterConfiguration *conf = new TimeShifterConfiguration(NULL, this);
    QObject::connect(this, SIGNAL(sigUpdateConfig()), conf, SLOT(slotUpdateConfig()));
    return ConfigPageInfo (conf,
                           i18n("Timeshifter"),
                           i18n("Timeshifter Options"),
                           "media-playback-pause");
}


bool TimeShifter::noticeSoundStreamClosed(SoundStreamID id)
{
    if (id == m_InputStreamID || ((id == m_OutputStreamSinkID) && (m_OutputStreamSinkID != m_OutputStreamSourceID))) {
        return stopPlayback(m_OutputStreamSourceID);
    }
    else {
        return false;
    }
}

bool TimeShifter::startPlayback(SoundStreamID id)
{
    if (id == m_OutputStreamSourceID) {
        m_StreamPaused = false;
        return true;
    }
    return false;
}

bool TimeShifter::stopPlayback(SoundStreamID id)
{
    if (id == m_InputStreamID) {

        return sendStopPlayback(m_OutputStreamSourceID);

    } else if (id == m_OutputStreamSourceID) {

        sendStopCapture (m_InputStreamID);
        closeSoundStream(m_OutputStreamSourceID);

        m_InputStreamID       .invalidate();
        m_OutputStreamSourceID.invalidate();
        m_OutputStreamSinkID  .invalidate();

        m_RingBuffer.clear();
        m_PlaybackMetaData         = SoundMetaData(0,0,0);
        m_PlaybackDataLeftInBuffer = 0;
        if (m_RingBuffer.error()) {
            logError(m_RingBuffer.errorString());
        }
        return  true;
    }
    return false;
}


bool TimeShifter::pausePlayback(SoundStreamID id)
{
    if (m_RingBuffer.error()) {
        logError(m_RingBuffer.errorString());
        return false;
    }

    if (!m_InputStreamID.isValid()) {

        QString orgdescr;
        querySoundStreamDescription(id, orgdescr);
        m_SoundStreamDescription = i18n("Time shift of %1 (%2)", orgdescr, name());

        SoundStreamID   input_id  = id;
        SoundStreamID   output_id = createNewSoundStream(m_InputStreamID, false);

        // mute and stop org stream playback
        queryPlaybackVolume(input_id, m_orgVolume);
        sendMuteSink(input_id);
        sendPlaybackVolume(input_id, 0);
        sendStopPlayback(input_id);

        // redirect

        m_InputStreamID        = input_id;
        m_OutputStreamSourceID = output_id;
        m_OutputStreamSinkID   = output_id;

        notifySoundStreamCreated(m_OutputStreamSourceID);
        notifySoundStreamSinkRedirected(m_InputStreamID, m_OutputStreamSourceID);

        m_StreamPaused = true;

        m_RingBuffer.clear();
        m_PlaybackMetaData         = SoundMetaData(0,0,0);
        m_PlaybackDataLeftInBuffer = 0;

        sendStartCaptureWithFormat(m_InputStreamID, m_SoundFormat, m_realSoundFormat);

        ISoundStreamClient *playback_mixer = searchPlaybackMixer();
        if (playback_mixer) {
            playback_mixer->preparePlayback(m_OutputStreamSinkID, m_PlaybackMixerChannel, /*active*/true, /*startimmediately*/ true);
            m_PlaybackMixerID = playback_mixer->getSoundStreamClientID();
        }

        return true;

    } else if (id == m_OutputStreamSinkID && !m_StreamPaused) {
        m_StreamPaused = true;
        queryPlaybackVolume(m_OutputStreamSinkID, m_orgVolume);
        return true;
    }
    return false;
}


bool TimeShifter::resumePlayback(SoundStreamID id)
{
    if (m_RingBuffer.error()) {
        logError(m_RingBuffer.errorString());
        return false;
    }

    if (m_OutputStreamSinkID.isValid() && id == m_OutputStreamSinkID && m_StreamPaused) {
        m_StreamPaused = false;
        sendUnmuteSink(m_OutputStreamSinkID);
        sendPlaybackVolume(m_OutputStreamSinkID, m_orgVolume);
        return true;
    }
    return false;
}


template<class T> static void addToBuffer(char *&buffer, const T &data)
{
    *(T*)buffer = data;
    buffer += sizeof(T);
}

static void addToBuffer(char *&buffer, const KUrl &url)
{
    QByteArray data = url.pathOrUrl().toUtf8();
    size_t size = data.size();
    addToBuffer(buffer, size);
    memcpy(buffer, data.data(), size);
    buffer += size;
}

template<class T> static void readFromBuffer(const char *&buffer, T &data)
{
    data = *(T*)buffer;
    buffer += sizeof(T);
}

static void readFromBuffer(const char *&buffer, KUrl &url)
{
    size_t size = 0;
    readFromBuffer(buffer, size);
    QByteArray data(buffer, size);
    buffer += size;
    url = KUrl(data);
}

static size_t my_sizeof(const KUrl &url)
{
    return sizeof(size_t) + url.pathOrUrl().toUtf8().size();
}

size_t TimeShifter::writeMetaDataToBuffer(const SoundMetaData &md, char *buffer, size_t buffer_size)
{
    if (m_RingBuffer.error()) {
        logError(m_RingBuffer.errorString());
        return 0;
    }

    quint64    pos      = md.position();
    time_t     abs      = md.absoluteTimestamp();
    time_t     rel      = md.relativeTimestamp();
    KUrl       url      = md.url();
    size_t     req_size = sizeof(req_size) + sizeof(pos) + sizeof(abs) + sizeof(rel) + my_sizeof(url);
    if (req_size <= buffer_size) {
        addToBuffer(buffer, req_size);
        addToBuffer(buffer, pos);
        addToBuffer(buffer, abs);
        addToBuffer(buffer, rel);
        addToBuffer(buffer, url);
        return req_size;
    }
    // FIXME: does that really make sense?
    else if (buffer_size >= sizeof(req_size)) {
        *(size_t*)buffer = sizeof(req_size);
        return sizeof(req_size);
    } else {
        return 0;
    }
}

size_t TimeShifter::writeCurrentStreamPropertiesToBuffer(char *buffer, size_t buffer_size)
{
    if (m_RingBuffer.error()) {
        logError(m_RingBuffer.errorString());
        return 0;
    }

    bool  is_stereo = false;
    bool  good_qual = false;
    float qual      = 0.0;
    queryIsStereo      (m_InputStreamID, is_stereo);
    queryHasGoodQuality(m_InputStreamID, good_qual);
    querySignalQuality (m_InputStreamID, qual);

    size_t   req_size = sizeof(req_size) + sizeof(is_stereo) + sizeof(good_qual) + sizeof(qual);
    if (req_size <= buffer_size) {
        addToBuffer(buffer, req_size);
        addToBuffer(buffer, is_stereo);
        addToBuffer(buffer, good_qual);
        addToBuffer(buffer, qual);
        return req_size;
    }
    // FIXME: does that really make sense?
    else if (buffer_size >= sizeof(req_size)) {
        *(size_t*)buffer = sizeof(req_size);
        return sizeof(req_size);
    } else {
        return 0;
    }
}



size_t TimeShifter::readMetaDataFromBuffer(SoundMetaData &md, const char *buffer, size_t buffer_size)
{
    if (m_RingBuffer.error()) {
        logError(m_RingBuffer.errorString());
        return 0;
    }

    size_t   req_size = 0;
    quint64  pos = 0;
    time_t   abs = 0;
    time_t   rel = 0;
    KUrl     url;
    if (buffer_size >= sizeof(req_size)) {
        readFromBuffer(buffer, req_size);
        if (req_size > sizeof(req_size)) {
            readFromBuffer(buffer, pos);
            readFromBuffer(buffer, abs);
            readFromBuffer(buffer, rel);
            readFromBuffer(buffer, url);
        }
    }
    md = SoundMetaData(pos, rel, abs, url);
    return req_size;
}

size_t TimeShifter::readCurrentStreamPropertiesFromBuffer(const char *buffer, size_t buffer_size)
{
    if (m_RingBuffer.error()) {
        logError(m_RingBuffer.errorString());
        return 0;
    }

    size_t  req_size  = 0;
    bool    is_stereo = false;
    bool    good_qual = false;
    float   qual      = 0.0;
    if (buffer_size >= sizeof(req_size)) {
        readFromBuffer(buffer, req_size);
        if (req_size > sizeof(req_size)) {
            readFromBuffer(buffer, is_stereo);
            readFromBuffer(buffer, good_qual);
            readFromBuffer(buffer, qual);
        }
    }
    if (m_currentStereo != is_stereo) {
        m_currentStereo = is_stereo;
        notifyStereoChanged(m_OutputStreamSourceID, m_currentStereo);
    }
    if (m_currentGoodQuality != good_qual) {
        m_currentGoodQuality = good_qual;
        notifySignalQualityBoolChanged(m_OutputStreamSourceID, m_currentGoodQuality);
    }
    if (m_currentQuality != qual) {
        m_currentQuality = qual;
        notifySignalQualityChanged(m_OutputStreamSourceID, m_currentQuality);
    }
    return req_size;
}


bool TimeShifter::noticeSoundStreamData(SoundStreamID id, const SoundFormat &/*sf*/, const char *data, size_t size, size_t &consumed_size, const SoundMetaData &md)
{
    if (id == m_InputStreamID && !m_RingBuffer.error()) {
        char buffer_meta[1024];
        char buffer_prop[ 128];
        size_t meta_buffer_size = writeMetaDataToBuffer               (md, buffer_meta, sizeof(buffer_meta));
        size_t prop_buffer_size = writeCurrentStreamPropertiesToBuffer(    buffer_prop, sizeof(buffer_prop));

        size_t packet_size = meta_buffer_size + prop_buffer_size + sizeof(size) + size;
        if (packet_size > m_RingBuffer.getMaxSize())
            return false;

        qint64 diff = m_RingBuffer.getFreeSize() - packet_size;
        while (diff < 0) {
            skipPacketInRingBuffer();
            diff = m_RingBuffer.getFreeSize() - packet_size;
        }
        m_RingBuffer.addData(buffer_meta, meta_buffer_size);
        m_RingBuffer.addData(buffer_prop, prop_buffer_size);
        m_RingBuffer.addData((const char*)&size, sizeof(size));
        m_RingBuffer.addData(data, size);
        consumed_size = (consumed_size == SIZE_T_DONT_CARE) ? size : min(consumed_size, size);

        if (m_RingBuffer.error()) {
            logError(m_RingBuffer.errorString());
            return false;
        }
        return true;
    }
    return false;
}


void TimeShifter::skipPacketInRingBuffer()
{
    if (m_RingBuffer.error()) {
        logError(m_RingBuffer.errorString());
        return;
    }

    if (m_PlaybackDataLeftInBuffer > 0) {
        m_RingBuffer.removeData(m_PlaybackDataLeftInBuffer);
    } else {
        size_t meta_size = 0;
        m_RingBuffer.takeData((char*)&meta_size, sizeof(meta_size));
        m_RingBuffer.removeData(meta_size - sizeof(meta_size));
        size_t prop_size = 0;
        m_RingBuffer.takeData((char*)&prop_size, sizeof(prop_size));
        m_RingBuffer.removeData(prop_size - sizeof(prop_size));
        size_t packet_size = 0;
        m_RingBuffer.takeData((char*)&packet_size, sizeof(packet_size));
        m_RingBuffer.removeData(packet_size - sizeof(packet_size));
    }
}


static char zero_buffer[65536];
static bool zero_buffer_init_done = false;

bool TimeShifter::noticeReadyForPlaybackData(SoundStreamID id, size_t free_size)
{
    if (id == m_OutputStreamSourceID) {

        if (m_StreamPaused) {
            // in this case feed zeros in order to implicitly flush the buffers in the sound chain.
            if (!zero_buffer_init_done) {
                for (unsigned int i = 0; i < sizeof(zero_buffer); ++i) {
                    zero_buffer[i] = 0;
                }
            }
            size_t s = qMin(sizeof(zero_buffer), free_size);
            size_t consumed_size = SIZE_T_DONT_CARE;
            notifySoundStreamData(m_OutputStreamSourceID, m_realSoundFormat, zero_buffer, s, consumed_size, m_PlaybackMetaData);
        } else {
            while (!m_RingBuffer.error() && m_RingBuffer.getFillSize() > 0 && free_size > 0) {
                if (m_PlaybackDataLeftInBuffer == 0) {
                    char meta_buffer[1024];
                    char prop_buffer[ 128];
                    size_t &meta_size = *(size_t*)(void*)meta_buffer;
                    size_t &prop_size = *(size_t*)(void*)prop_buffer;
                    meta_size = 0;
                    prop_size = 0;

                    m_RingBuffer.takeData(meta_buffer, sizeof(meta_size));
                    if (meta_size && meta_size <= sizeof(meta_buffer)) {
                        m_RingBuffer.takeData(meta_buffer + sizeof(meta_size), meta_size - sizeof(meta_size));
                        readMetaDataFromBuffer(m_PlaybackMetaData, meta_buffer, meta_size);
                    } else {
                        m_RingBuffer.removeData(meta_size - sizeof(meta_size));
                    }

                    m_RingBuffer.takeData(prop_buffer, sizeof(prop_size));
                    if (prop_size && prop_size <= sizeof(prop_buffer)) {
                        m_RingBuffer.takeData(prop_buffer + sizeof(prop_size), prop_size - sizeof(prop_size));
                        readCurrentStreamPropertiesFromBuffer(prop_buffer, prop_size);
                    } else {
                        m_RingBuffer.removeData(prop_size - sizeof(prop_size));
                    }

                    m_PlaybackDataLeftInBuffer = 0;
                    m_RingBuffer.takeData((char*)&m_PlaybackDataLeftInBuffer, sizeof(m_PlaybackDataLeftInBuffer));
                }

                const size_t buffer_size = 65536;
                char buffer[buffer_size];

                while (!m_RingBuffer.error() && m_PlaybackDataLeftInBuffer > 0 && free_size > 0) {
                    size_t s = m_PlaybackDataLeftInBuffer < free_size ? m_PlaybackDataLeftInBuffer : free_size;

                    if (s > buffer_size)
                        s = buffer_size;
                    s = m_RingBuffer.takeData(buffer, s);

                    size_t consumed_size = SIZE_T_DONT_CARE;
                    notifySoundStreamData(m_OutputStreamSourceID, m_realSoundFormat, buffer, s, consumed_size, m_PlaybackMetaData);
                    if (consumed_size == SIZE_T_DONT_CARE)
                        consumed_size = s;

                    free_size                  -= consumed_size;
                    m_PlaybackDataLeftInBuffer -= consumed_size;
                    if (consumed_size < s) {
                        logError(i18n("TimeShifter::notifySoundStreamData: clients skipped %1 bytes. Data Lost", s - consumed_size));
                        free_size = 0; // break condition for outer loop
                        break;
                    }
                }
            } // end while
        }
        notifyReadyForPlaybackData(m_InputStreamID, m_RingBuffer.getMaxSize());
        return true;
    }
    return false;
}



ISoundStreamClient *TimeShifter::searchPlaybackMixer()
{
    ISoundStreamClient *playback_mixer = getSoundStreamClientWithID(m_PlaybackMixerID);

    // some simple sort of autodetection if one mixer isn't present any more
    if (!playback_mixer) {
        QList<ISoundStreamClient*> playback_mixers = queryPlaybackMixers();
        if (!playback_mixers.isEmpty())
            playback_mixer = playback_mixers.first();
    }
    return playback_mixer;
}


bool  TimeShifter::setPlaybackMixer(const QString &soundStreamClientID, const QString &ch)
{
    m_PlaybackMixerID = soundStreamClientID;
    m_PlaybackMixerChannel = ch;

    ISoundStreamClient *playback_mixer = searchPlaybackMixer();

    float  oldVolume;
    if (m_OutputStreamSinkID.isValid()) {
        queryPlaybackVolume(m_OutputStreamSinkID, oldVolume);
        sendStopPlayback(m_OutputStreamSinkID);
        sendReleasePlayback(m_OutputStreamSinkID);
    }

    if (playback_mixer)
        playback_mixer->preparePlayback(m_OutputStreamSinkID, m_PlaybackMixerChannel, /*active*/true, /*start_imm*/false);

    if (m_OutputStreamSinkID.isValid()) {
        sendStartPlayback(m_OutputStreamSinkID);
        sendPlaybackVolume(m_OutputStreamSinkID, oldVolume);
    }

    return true;
}


void TimeShifter::setTempFile(const QString &filename, quint64  s)
{
    m_RingBuffer.clear();
    m_RingBuffer.resize(m_TempFileName = filename, m_TempFileMaxSize = s);
    m_PlaybackMetaData = SoundMetaData(0,0,0, i18n("internal stream, not stored"));
    m_PlaybackDataLeftInBuffer = 0;
    if (m_RingBuffer.error()) {
        logError(m_RingBuffer.errorString());
    }
}

bool TimeShifter::getSoundStreamDescription(SoundStreamID id, QString &descr) const
{
    if (id == m_OutputStreamSourceID) {
        descr = m_SoundStreamDescription;
        return true;
    }
    else {
        return false;
    }
}


bool TimeShifter::getSoundStreamRadioStation(SoundStreamID id, const RadioStation *&rs) const
{
    if (id == m_OutputStreamSourceID) {
        return querySoundStreamRadioStation(m_InputStreamID, rs);
    }
    else {
        return false;
    }
}

bool TimeShifter::startCaptureWithFormat(
    SoundStreamID      id,
    const SoundFormat &proposed_format,
    SoundFormat       &real_format,
    bool               force_format
)
{
    if (id == m_OutputStreamSourceID) {
        if (force_format && m_realSoundFormat != proposed_format) {
            sendStopCapture(m_InputStreamID);
            sendStartCaptureWithFormat(m_InputStreamID, proposed_format, m_realSoundFormat);
        }
        real_format = m_realSoundFormat;
        return true;
    } else {
        return false;
    }
}

bool TimeShifter::stopCapture(SoundStreamID id)
{
    if (id == m_OutputStreamSourceID) {
        return true;
    } else {
        return false;
    }
}


bool TimeShifter::isPlaybackPaused(SoundStreamID id, bool &b) const
{
    if (id == m_OutputStreamSourceID) {
        b = m_StreamPaused;
        return true;
    } else {
        return false;
    }
}


bool TimeShifter::noticeSoundStreamSinkRedirected  (SoundStreamID oldID, SoundStreamID newID)
{
    if (m_InputStreamID == oldID) {
        m_InputStreamID = newID;
        return true;
    } else if (m_OutputStreamSinkID == oldID) {
        m_OutputStreamSinkID = newID;
        return true;
    } else {
        return false;
    }
}

bool TimeShifter::noticeSoundStreamSourceRedirected(SoundStreamID /*oldID*/, SoundStreamID /*newID*/)
{
    // FIXME: create error message or stop time shifting
    return false;
}


bool TimeShifter::getSignalQuality   (SoundStreamID, float &q) const
{
    q = m_currentQuality;
    return true;
}

bool TimeShifter::hasGoodQuality     (SoundStreamID, bool &q)   const
{
    q = m_currentGoodQuality;
    return true;
}

bool TimeShifter::isStereo           (SoundStreamID, bool &s)  const
{
    s = m_currentStereo;
    return true;
}



#include "timeshifter.moc"
