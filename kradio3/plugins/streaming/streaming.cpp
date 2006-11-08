/***************************************************************************
                          streaming.cpp  -  description
                             -------------------
    begin                : Sun Sept 3 2006
    copyright            : (C) 2006 by Martin Witte
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

#include "streaming.h"

#include "../../src/libkradio-gui/aboutwidget.h"
#include "../../src/libkradio/utils.h"
#include <klocale.h>
#include <kaboutdata.h>
#include <kurl.h>
#include <klocale.h>

#include "streaming-job.h"
#include "streaming-configuration.h"

///////////////////////////////////////////////////////////////////////
//// plugin library functions

PLUGIN_LIBRARY_FUNCTIONS(StreamingDevice, "kradio-streaming", i18n("Streaming Support"));

/////////////////////////////////////////////////////////////////////////////




StreamingDevice::StreamingDevice(const QString &name)
    : QObject(NULL, NULL),
      PluginBase(name, i18n("KRadio Streaming Plugin"))
{
    m_CaptureChannels.setAutoDelete(true);
    m_PlaybackChannels.setAutoDelete(true);
}


StreamingDevice::~StreamingDevice()
{
    resetPlaybackStreams();
    resetCaptureStreams();
}


bool StreamingDevice::connectI(Interface *i)
{
    bool a = PluginBase::connectI(i);
    bool b = ISoundStreamClient::connectI(i);
    return a || b;
}


bool StreamingDevice::disconnectI(Interface *i)
{
    bool a = PluginBase::disconnectI(i);
    bool b = ISoundStreamClient::disconnectI(i);
    return a || b;
}

void StreamingDevice::noticeConnectedI (ISoundStreamServer *s, bool pointer_valid)
{
    ISoundStreamClient::noticeConnectedI(s, pointer_valid);
    if (s && pointer_valid) {
        s->register4_sendReleasePlayback(this);
        s->register4_sendReleaseCapture(this);
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
        s->register4_notifyReadyForPlaybackData(this);
    }
}

// PluginBase

void StreamingDevice::saveState (KConfig *c) const
{
    c->setGroup(QString("streaming-") + PluginBase::name());
    c->writeEntry("soundstreamclient-id", m_SoundStreamClientID);

    c->writeEntry("playback-channels", m_PlaybackChannelList.size());
    for (unsigned int i = 0; i < m_PlaybackChannelList.size(); ++i) {
        QString s = m_PlaybackChannelList[i];
        const StreamingJob *j = m_PlaybackChannels[s];

        const SoundFormat &sf          = j->getSoundFormat();
        KURL               url         = j->getURL();
        size_t             buffer_size = j->getBufferSize();

        sf.saveConfig("playback-channel-" + QString::number(i), c);
        c->writeEntry("playback-channel-" + QString::number(i) + "-url", url.url());
        c->writeEntry("playback-channel-" + QString::number(i) + "-buffer-size", buffer_size);
    }

    c->writeEntry("capture-channels", m_CaptureChannelList.size());
    for (unsigned int i = 0; i < m_CaptureChannelList.size(); ++i) {
        QString s = m_CaptureChannelList[i];
        const StreamingJob *j = m_CaptureChannels[s];

        const SoundFormat &sf          = j->getSoundFormat();
        KURL               url         = j->getURL();
        size_t             buffer_size = j->getBufferSize();

        sf.saveConfig("capture-channel-" + QString::number(i), c);
        c->writeEntry("capture-channel-" + QString::number(i) + "-url", url.url());
        c->writeEntry("capture-channel-" + QString::number(i) + "-buffer-size", buffer_size);
    }
}

void StreamingDevice::restoreState (KConfig *c)
{
    c->setGroup(QString("streaming-") + PluginBase::name());
    setSoundStreamClientID(c->readEntry("soundstreamclient-id", getSoundStreamClientID()));

    resetPlaybackStreams(false);
    resetCaptureStreams(false);

    int n = c->readNumEntry("playback-channels", 0);
    for (int i = 0; i < n; ++i) {
        SoundFormat sf;
        sf.restoreConfig("playback-channel-" + QString::number(i), c);
        QString url         = c->readEntry("playback-channel-" + QString::number(i) + "-url", QString::null);
        size_t  buffer_size = c->readNum64Entry("playback-channel-" + QString::number(i) + "-buffer-size", 32*1024);

        if (!url.isNull()) {
            addPlaybackStream(url, sf, buffer_size, i == n-1);
        }
    }

    n = c->readNumEntry("capture-channels", 0);
    for (int i = 0; i < n; ++i) {
        SoundFormat sf;
        sf.restoreConfig("capture-channel-" + QString::number(i), c);
        QString url         = c->readEntry("capture-channel-" + QString::number(i) + "-url", QString::null);
        size_t  buffer_size = c->readNum64Entry("capture-channel-" + QString::number(i) + "-buffer-size", 32*1024);

        if (!url.isNull()) {
            addCaptureStream(url, sf, buffer_size, i == n-1);
        }
    }

    if (!m_CaptureChannelList.size()) {
        addCaptureStream("/dev/video24", SoundFormat(48000, 2, 16, true, BYTE_ORDER, "raw"), 65536);
    }

    emit sigUpdateConfig();
}


ConfigPageInfo  StreamingDevice::createConfigurationPage()
{
    StreamingConfiguration *conf = new StreamingConfiguration(NULL, this);
    QObject::connect(this, SIGNAL(sigUpdateConfig()), conf, SLOT(slotUpdateConfig()));
    return ConfigPageInfo (conf,
                           i18n("Streaming"),
                           i18n("Streaming Device Options"),
                           "kradio_streaming");
}


AboutPageInfo StreamingDevice::createAboutPage()
{
    return AboutPageInfo();
}



bool StreamingDevice::preparePlayback(SoundStreamID id, const QString &channel, bool /*active_mode*/, bool start_immediately)
{
    if (id.isValid() && m_PlaybackChannels.find(channel)) {
        m_AllPlaybackStreams.insert(id, channel);
        if (start_immediately)
            startPlayback(id);
        return true;
    }
    return false;
}


bool StreamingDevice::prepareCapture(SoundStreamID id, const QString &channel)
{
    logDebug("StreamingDevice::prepareCapture");
    if (id.isValid() && m_CaptureChannels.find(channel)) {
        m_AllCaptureStreams.insert(id, channel);
        return true;
    }
    return false;
}

bool StreamingDevice::releasePlayback(SoundStreamID id)
{
    if (id.isValid() && m_AllPlaybackStreams.contains(id)) {
        stopPlayback(id);
        if (!m_EnabledPlaybackStreams.contains(id))
            m_AllPlaybackStreams.remove(id);
        return true;
    }
    return false;
}

bool StreamingDevice::releaseCapture(SoundStreamID id)
{
    logDebug("StreamingDevice::releaseCapture");
    if (id.isValid() && m_AllCaptureStreams.contains(id)) {
        stopCapture(id);
        if (!m_EnabledCaptureStreams.contains(id))
            m_AllCaptureStreams.remove(id);
        return true;
    }
    return false;
}

bool StreamingDevice::supportsPlayback()   const
{
    return m_PlaybackChannels.size() > 0;
}


bool StreamingDevice::supportsCapture() const
{
    return m_CaptureChannels.size() > 0;
}


bool StreamingDevice::startPlayback(SoundStreamID id)
{
    if (id.isValid() && m_AllPlaybackStreams.contains(id)) {
        m_EnabledPlaybackStreams.insert(id, m_AllPlaybackStreams[id]);
        StreamingJob &x = *m_PlaybackChannels.find(m_AllPlaybackStreams[id]);
        x.startPlayback();
        return true;
    } else {
        return false;
    }
}


bool StreamingDevice::pausePlayback(SoundStreamID /*id*/)
{
    //return stopPlayback(id);
    return false;
}


bool StreamingDevice::stopPlayback(SoundStreamID id)
{
    if (id.isValid() && m_EnabledPlaybackStreams.contains(id)) {
        StreamingJob &x = *m_PlaybackChannels.find(m_AllPlaybackStreams[id]);
        if (x.stopPlayback()) {
            m_EnabledPlaybackStreams.remove(id);
        }
        return true;
    } else {
        return false;
    }
}


bool StreamingDevice::isPlaybackRunning(SoundStreamID id, bool &b) const
{
    if (id.isValid() && m_EnabledPlaybackStreams.contains(id)) {
        b = true;
        return true;
    } else {
        return false;
    }
}

bool StreamingDevice::startCaptureWithFormat(SoundStreamID      id,
                                             const SoundFormat &proposed_format,
                                             SoundFormat       &real_format,
                                             bool               force_format)
{
    logDebug("StreamingDevice::startCaptureWithFormat");
    if (id.isValid() && m_AllCaptureStreams.contains(id)) {
        m_EnabledCaptureStreams.insert(id, m_AllCaptureStreams[id]);
        StreamingJob &x = *m_CaptureChannels.find(m_AllCaptureStreams[id]);
        x.startCapture(proposed_format, real_format, force_format);
        return true;
    } else {
        return false;
    }
}


bool StreamingDevice::stopCapture(SoundStreamID id)
{
    if (id.isValid() && m_EnabledCaptureStreams.contains(id)) {
        StreamingJob &x = *m_CaptureChannels.find(m_AllCaptureStreams[id]);
        if (x.stopCapture()) {
            m_EnabledCaptureStreams.remove(id);
        }
        return true;
    } else {
        return false;
    }
}


bool StreamingDevice::isCaptureRunning(SoundStreamID id, bool &b, SoundFormat &sf) const
{
    if (id.isValid() && m_EnabledCaptureStreams.contains(id)) {
        StreamingJob &x = *m_CaptureChannels.find(m_AllCaptureStreams[id]);
        sf = x.getSoundFormat();
        b  = true;
        return true;
    } else {
        return false;
    }
}


bool StreamingDevice::noticeSoundStreamClosed(SoundStreamID id)
{
    bool found = (stopCapture(id)    && releaseCapture(id)) ||
                 (stopPlayback(id)   && releasePlayback(id));
    return found;
}


bool StreamingDevice::noticeSoundStreamRedirected(SoundStreamID oldID, SoundStreamID newID)
{
    bool found = false;
    if (newID != oldID) {
        if (m_AllPlaybackStreams.contains(oldID)) {
            m_AllPlaybackStreams.insert(newID, m_AllPlaybackStreams[oldID]);
            m_AllPlaybackStreams.remove(oldID);
            found = true;
        }
        if (m_EnabledPlaybackStreams.contains(oldID)) {
            m_EnabledPlaybackStreams.insert(newID, m_EnabledPlaybackStreams[oldID]);
            m_EnabledPlaybackStreams.remove(oldID);
            found = true;
        }
        if (m_AllCaptureStreams.contains(oldID)) {
            m_AllCaptureStreams.insert(newID, m_AllCaptureStreams[oldID]);
            m_AllCaptureStreams.remove(oldID);
            found = true;
        }
        if (m_EnabledCaptureStreams.contains(oldID)) {
            m_EnabledCaptureStreams.insert(newID, m_EnabledCaptureStreams[oldID]);
            m_EnabledCaptureStreams.remove(oldID);
            found = true;
        }
    }
    return found;
}


bool StreamingDevice::noticeSoundStreamData(SoundStreamID id,
                                            const SoundFormat &/*format*/,
                                            const char *data, size_t size, size_t &consumed_size,
                                            const SoundMetaData &/*md*/
                                            )
{
    if (id.isValid() && m_EnabledPlaybackStreams.contains(id)) {
        StreamingJob &x = *m_CaptureChannels.find(m_AllCaptureStreams[id]);
        x.playData(data, size, consumed_size);
        return true;
    }
    else {
        return false;
    }
}

bool StreamingDevice::noticeReadyForPlaybackData(SoundStreamID id, size_t free_size)
{
    if (!id.isValid() || !m_AllCaptureStreams.contains(id))
        return false;
    StreamingJob &x = *m_CaptureChannels.find(m_AllCaptureStreams[id]);

    while (x.hasRecordedData() && free_size > 0) {
        const char   *buffer        = NULL;
        size_t        size          = SIZE_T_DONT_CARE;
        size_t        consumed_size = SIZE_T_DONT_CARE;
        SoundMetaData meta_data(0,0,0, QString::null);
        x.lockData(buffer, size, meta_data); // get pointer to data and meta-data content
        if (size > free_size)
            size = free_size;
        notifySoundStreamData(id, x.getSoundFormat(), buffer, size, consumed_size, meta_data);
        if (consumed_size == SIZE_T_DONT_CARE)
            consumed_size = size;
        x.removeData(consumed_size);
        free_size -= consumed_size;
        if (consumed_size < size) {
            logWarning(i18n("StreamingDevice %1::notifySoundStreamData: Playback Clients skipped %2 bytes").arg(name()).arg(size-consumed_size));
            break;
        }
    }
    return true;
}



const QStringList &StreamingDevice::getPlaybackChannels() const
{
    return m_PlaybackChannelList;
}


const QStringList &StreamingDevice::getCaptureChannels() const
{
    return m_CaptureChannelList;
}


QString StreamingDevice::getSoundStreamClientDescription() const
{
    return i18n("Streaming Device %1").arg(PluginBase::name());
}


void   StreamingDevice::logStreamError(const KURL &url, const QString &s)
{
    logError(i18n("Streaming Device %1, %2: %3").arg(name()).arg(url.url()).arg(s));
}

void   StreamingDevice::logStreamWarning(const KURL &url, const QString &s)
{
    logWarning(i18n("Streaming Device %1, %2: %3").arg(name()).arg(url.url()).arg(s));
}


bool StreamingDevice::getPlaybackStreamOptions(const QString &channel, QString &url, SoundFormat &sf, size_t &buffer_size) const
{
    if (m_PlaybackChannels.find(channel)) {
        const StreamingJob *j = m_PlaybackChannels[channel];
        url = j->getURL();
        sf  = j->getSoundFormat();
        buffer_size = j->getBufferSize();
        return true;
    }
    return false;
}


bool StreamingDevice::getCaptureStreamOptions(const QString &channel, QString &url, SoundFormat &sf, size_t &buffer_size) const
{
    if (m_CaptureChannels.find(channel)) {
        const StreamingJob *j = m_CaptureChannels[channel];
        url = j->getURL();
        sf  = j->getSoundFormat();
        buffer_size = j->getBufferSize();
        return true;
    }
    return false;
}

void StreamingDevice::resetPlaybackStreams(bool notification_enabled)
{
    while (m_EnabledPlaybackStreams.begin() != m_EnabledPlaybackStreams.end()) {
        sendStopPlayback(m_EnabledPlaybackStreams.begin().key());
    }
    while (m_AllPlaybackStreams.begin() != m_AllPlaybackStreams.end()) {
        releasePlayback(m_AllPlaybackStreams.begin().key());
    }
    m_PlaybackChannelList.clear();
    m_PlaybackChannels.clear();
    if (notification_enabled) {
        notifyPlaybackChannelsChanged(m_SoundStreamClientID, m_PlaybackChannelList);
    }
}


void StreamingDevice::resetCaptureStreams(bool notification_enabled)
{
    while (m_EnabledCaptureStreams.begin() != m_EnabledCaptureStreams.end()) {
        sendStopCapture(m_EnabledCaptureStreams.begin().key());
    }
    while (m_AllCaptureStreams.begin() != m_AllCaptureStreams.end()) {
        releaseCapture(m_AllCaptureStreams.begin().key());
    }
    m_CaptureChannelList.clear();
    m_CaptureChannels.clear();
    if (notification_enabled) {
        notifyCaptureChannelsChanged(m_SoundStreamClientID, m_CaptureChannelList);
    }
}


void StreamingDevice::addPlaybackStream(const QString &url, const SoundFormat &sf, size_t buffer_size, bool notification_enabled)
{
    StreamingJob *x = new StreamingJob(url, sf, buffer_size);
    connect(x,    SIGNAL(logStreamError(const KURL &, const QString &)),
            this, SLOT  (logStreamError(const KURL &, const QString &)));

    m_PlaybackChannelList.append(url);
    m_PlaybackChannels.insert(url, x);
    if (notification_enabled) {
        notifyPlaybackChannelsChanged(m_SoundStreamClientID, m_PlaybackChannelList);
    }
}


void StreamingDevice::addCaptureStream (const QString &url, const SoundFormat &sf, size_t buffer_size, bool notification_enabled)
{
    StreamingJob *x = new StreamingJob(url, sf, buffer_size);
    connect(x,    SIGNAL(logStreamError(const KURL &, const QString &)),
            this, SLOT  (logStreamError(const KURL &, const QString &)));

    m_CaptureChannelList.append(url);
    m_CaptureChannels.insert(url, x);
    if (notification_enabled) {
        notifyCaptureChannelsChanged(m_SoundStreamClientID, m_CaptureChannelList);
    }
}


