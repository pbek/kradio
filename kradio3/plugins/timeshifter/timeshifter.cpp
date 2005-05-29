/***************************************************************************
                          timeshifter.cpp  -  description
                             -------------------
    begin                : Mon May 16 13:39:31 CEST 2005
    copyright            : (C) 2005 by Ernst Martin Witte
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

#include <klocale.h>
#include <linux/soundcard.h>

#include "../../src/libkradio/utils.h"
#include "timeshifter.h"
#include "timeshifter-configuration.h"

///////////////////////////////////////////////////////////////////////

PLUGIN_LIBRARY_FUNCTIONS(TimeShifter, "TimeShift Support");

///////////////////////////////////////////////////////////////////////

TimeShifter::TimeShifter (const QString &name)
    : PluginBase(name, i18n("TimeShifter Plugin")),
      m_TempFileName("/tmp/kradio-timeshifter-tempfile"),
      m_TempFileMaxSize(256*1024*1024),
      m_PlaybackMixerID(QString::null),
      m_PlaybackMixerChannel("pcm"),
      m_PlaybackMetaData(0,0,0),
      m_PlaybackDataLeftInBuffer(0),
      m_RingBuffer(m_TempFileName, m_TempFileMaxSize)
{
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
        s->register4_notifySoundStreamData(this);
        s->register4_notifyReadyForPlaybackData(this);
    }
}


void   TimeShifter::saveState    (KConfig *config) const
{
    config->setGroup(QString("timeshifter-") + name());

    config->writeEntry("temp-file-name",         m_TempFileName);
    config->writeEntry("max-file-size",          m_TempFileMaxSize / 1024 / 1024);

    config->writeEntry("PlaybackMixerID",      m_PlaybackMixerID);
    config->writeEntry("PlaybackMixerChannel", m_PlaybackMixerChannel);
}


void   TimeShifter::restoreState (KConfig *config)
{
    config->setGroup(QString("timeshifter-") + name());

    QString  fname =               config->readEntry("temp-file-name", "/tmp/kradio-timeshifter-tempfile");
    Q_UINT64 fsize = 1024 * 1024 * config->readNumEntry("max-file-size",  256);

    QString mixerID = config->readEntry ("PlaybackMixerID", QString::null);
    QString channel = config->readEntry ("PlaybackMixerChannel", "Line");

    setPlaybackMixer(mixerID, channel);
    setTempFile(fname, fsize);

    emit sigUpdateConfig();
}


ConfigPageInfo  TimeShifter::createConfigurationPage()
{
    TimeShifterConfiguration *conf = new TimeShifterConfiguration(NULL, this);
    QObject::connect(this, SIGNAL(sigUpdateConfig()), conf, SLOT(slotCancel()));
    return ConfigPageInfo (conf,
                           i18n("Timeshifter Options"),
                           i18n("Timeshifter Options"),
                           "player_pause");
}

AboutPageInfo   TimeShifter::createAboutPage()
{
    return AboutPageInfo();
}


bool TimeShifter::noticeSoundStreamClosed(SoundStreamID id)
{
    return stopPlayback(id);
}

bool TimeShifter::startPlayback(SoundStreamID id)
{
    if (id == m_OrgStreamID) {
        m_StreamPaused = false;
        return true;
    }
    return false;
}

bool TimeShifter::stopPlayback(SoundStreamID id)
{
    if (id == m_NewStreamID) {

        return sendStopPlayback(m_OrgStreamID);

    } else if (id == m_OrgStreamID) {

        SoundStreamID tmp_newID = m_NewStreamID;
        SoundStreamID tmp_orgID = m_OrgStreamID;

        m_OrgStreamID.invalidate();
        m_NewStreamID.invalidate();

        stopCapture(tmp_newID);
        closeSoundStream(tmp_newID);
        stopPlayback(tmp_newID);
        m_RingBuffer.clear();
        m_PlaybackMetaData = SoundMetaData(0,0,0);
        m_PlaybackDataLeftInBuffer = 0;
        return  true;
    }
    return false;
}


bool TimeShifter::pausePlayback(SoundStreamID id)
{
    if (!m_OrgStreamID.isValid()) {
        SoundStreamID orgid = id;
        SoundStreamID newid = createNewSoundStream(orgid, false);
        notifySoundStreamCreated(newid);
        notifySoundStreamRedirected(orgid, newid);
        sendMute(newid);
        sendPlaybackVolume(newid, 0);
        sendStopPlayback(newid);
        m_OrgStreamID  = orgid;
        m_NewStreamID  = newid;
        m_StreamPaused = true;

        m_RingBuffer.clear();
        m_PlaybackMetaData = SoundMetaData(0,0,0);
        m_PlaybackDataLeftInBuffer = 0;

        sendStartCaptureWithFormat(m_NewStreamID, m_SoundFormat, m_realSoundFormat);

        ISoundStreamClient *playback_mixer = searchPlaybackMixer();
        if (playback_mixer) {
            playback_mixer->preparePlayback(m_OrgStreamID, m_PlaybackMixerChannel, true);
            m_PlaybackMixerID = playback_mixer->getSoundStreamClientID();
        }

        return true;

    } else if (id == m_OrgStreamID) {
        m_StreamPaused = !m_StreamPaused;
        if (!m_StreamPaused) {
            sendStartPlayback(m_OrgStreamID);
        }
        return true;
    }
    return false;
}


unsigned TimeShifter::writeMetaDataToBuffer(const SoundMetaData &md, char *buffer, size_t buffer_size)
{
    Q_UINT64 pos     = md.position();
    time_t   abs     = md.absoluteTimestamp();
    time_t   rel     = md.relativeTimestamp();
    size_t   url_len = md.url().url().length() + 1;
    size_t   req_size = sizeof(req_size) + sizeof(pos) + sizeof(abs) + sizeof(rel) + sizeof(url_len) + url_len;
    if (req_size <= buffer_size) {
        *(size_t*)buffer = req_size;
        buffer += sizeof(req_size);
        *(Q_UINT64*)buffer = pos;
        buffer += sizeof(pos);
        *(time_t*)buffer = abs;
        buffer += sizeof(abs);
        *(time_t*)buffer = rel;
        buffer += sizeof(rel);
        *(size_t*)buffer = url_len;
        buffer += sizeof(url_len);
        memcpy(buffer, md.url().url().ascii(), url_len);
        buffer += url_len;
        return req_size;
    } else if (buffer_size >= sizeof(req_size)) {
        *(size_t*)buffer = sizeof(req_size);
        return sizeof(req_size);
    } else {
        return 0;
    }
}

size_t TimeShifter::readMetaDataFromBuffer(SoundMetaData &md, const char *buffer, size_t buffer_size)
{
    size_t   req_size = 0;
    Q_UINT64 pos = 0;
    time_t   abs = 0;
    time_t   rel = 0;
    size_t   url_len = 0;
    KURL     url;
    if (buffer_size >= sizeof(req_size)) {
        req_size = *(size_t*)buffer;
        buffer += sizeof(req_size);
        if (req_size > sizeof(req_size)) {
            pos = *(Q_UINT64*)buffer;
            buffer += sizeof(Q_UINT64);
            abs = *(time_t*)buffer;
            buffer += sizeof(abs);
            rel = *(time_t*)buffer;
            buffer += sizeof(rel);
            url_len = *(size_t*)buffer;
            buffer += sizeof(url_len);
            url = buffer;
            buffer += url_len;
        }
    }
    md = SoundMetaData(pos, rel, abs, url);
    return req_size;
}


bool TimeShifter::noticeSoundStreamData(SoundStreamID id, const SoundFormat &/*sf*/, const char *data, size_t size, const SoundMetaData &md)
{
    if (id == m_NewStreamID) {
        char buffer_meta[1024];
        size_t meta_buffer_size = writeMetaDataToBuffer(md, buffer_meta, 1024);
        size_t packet_size = meta_buffer_size + sizeof(size) + size;
        if (packet_size > m_RingBuffer.getMaxSize())
            return false;
        Q_INT64 diff = m_RingBuffer.getFreeSize() - packet_size;
        while (diff < 0) {
            skipPacketInRingBuffer();
            diff = m_RingBuffer.getFreeSize() - packet_size;
        }
        m_RingBuffer.addData(buffer_meta, meta_buffer_size);
        m_RingBuffer.addData((const char*)&size, sizeof(size));
        m_RingBuffer.addData(data, size);
        return true;
    }
    return false;
}


void TimeShifter::skipPacketInRingBuffer()
{
    if (m_PlaybackDataLeftInBuffer > 0) {
        m_RingBuffer.removeData(m_PlaybackDataLeftInBuffer);
    } else {
        size_t meta_size = 0;
        m_RingBuffer.takeData((char*)&meta_size, sizeof(meta_size));
        m_RingBuffer.removeData(meta_size - sizeof(meta_size));
        size_t packet_size = 0;
        m_RingBuffer.takeData((char*)&packet_size, sizeof(packet_size));
        m_RingBuffer.removeData(packet_size - sizeof(packet_size));
    }
}


bool TimeShifter::noticeReadyForPlaybackData(SoundStreamID id, size_t free_size)
{
    if (id == m_OrgStreamID && !m_StreamPaused) {

        while (!m_RingBuffer.error() && m_RingBuffer.getFillSize() > 0 && free_size > 0) {
            if (m_PlaybackDataLeftInBuffer == 0) {
                char meta_buffer[1024];
                size_t &meta_size = *(size_t*)meta_buffer;
                meta_size = 0;
                m_RingBuffer.takeData(meta_buffer, sizeof(meta_size));
                if (meta_size && meta_size <= 1024) {
                    m_RingBuffer.takeData(meta_buffer + sizeof(meta_size), meta_size - sizeof(meta_size));
                    readMetaDataFromBuffer(m_PlaybackMetaData, meta_buffer, meta_size);
                } else {
                    m_RingBuffer.removeData(meta_size - sizeof(meta_size));
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
                free_size -= s;
                m_PlaybackDataLeftInBuffer -= s;
                notifySoundStreamData(m_OrgStreamID, m_realSoundFormat, buffer, s, m_PlaybackMetaData);
            }
        }
        return true;
    }
    return false;
}



ISoundStreamClient *TimeShifter::searchPlaybackMixer()
{
    ISoundStreamClient *playback_mixer = getSoundStreamClientWithID(m_PlaybackMixerID);

    // some simple sort of autodetection if one mixer isn't present any more
    if (!playback_mixer) {
        QPtrList<ISoundStreamClient> playback_mixers = queryPlaybackMixers();
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
    if (m_OrgStreamID.isValid()) {
        queryPlaybackVolume(m_OrgStreamID, oldVolume);
        sendStopPlayback(m_OrgStreamID);
        sendReleasePlayback(m_OrgStreamID);
    }

    if (playback_mixer)
        playback_mixer->preparePlayback(m_OrgStreamID, m_PlaybackMixerChannel, true);

    if (m_OrgStreamID.isValid()) {
        sendStartPlayback(m_OrgStreamID);
        sendPlaybackVolume(m_OrgStreamID, oldVolume);
    }

    return true;
}


void TimeShifter::setTempFile(const QString &filename, Q_UINT64 s)
{
    m_RingBuffer.clear();
    m_RingBuffer.resize(m_TempFileName = filename, m_TempFileMaxSize = s);
    m_PlaybackMetaData = SoundMetaData(0,0,0);
    m_PlaybackDataLeftInBuffer = 0;
}



#include "timeshifter.moc"
