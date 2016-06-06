/***************************************************************************
                          streaming.cpp  -  description
                             -------------------
    begin                : Sun Sept 3 2006
    copyright            : (C) 2006 by Martin Witte
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

#include "streaming.h"

#include <klocalizedstring.h>
#include <kaboutdata.h>
#include <kurl.h>

#include "streaming-job.h"
#include "streaming-configuration.h"

///////////////////////////////////////////////////////////////////////
//// plugin library functions

static KAboutData aboutData()
{
    KAboutData about("StreamingDevice",
                     PROJECT_NAME,
                     KLocalizedString(),
                     KRADIO_VERSION,
                     ki18nc("@title", "Streaming Support"),
                     KAboutData::License_GPL,
                     KLocalizedString(),
                     KLocalizedString(),
                     "http://sourceforge.net/projects/kradio",
                     "emw-kradio@nocabal.de");
    return about;
}

KRADIO_EXPORT_PLUGIN(StreamingDevice, aboutData())

/////////////////////////////////////////////////////////////////////////////




StreamingDevice::StreamingDevice(const QString &instanceID, const QString &name)
    : PluginBase(instanceID, name, i18n("KRadio Streaming Plugin"))
{
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
        s->register4_notifyReadyForPlaybackData(this);
    }
}

// PluginBase

void StreamingDevice::saveState (KConfigGroup &c) const
{
    PluginBase::saveState(c);

    c.writeEntry("soundstreamclient-id", m_SoundStreamClientID);

    c.writeEntry("playback-channels", m_PlaybackChannelList.size());
    for (int i = 0; i < m_PlaybackChannelList.size(); ++i) {
        KUrl                url =  m_PlaybackChannelList[i];
        const StreamingJob *j   = *m_PlaybackChannelJobs.find(url);
        const QString num       = QString::number(i);

        const SoundFormat &sf          = j->getSoundFormat();
        url                            = j->getURL();
        size_t             buffer_size = j->getBufferSize();

        sf.saveConfig("playback-channel-" + num, c);
        c.writeEntry ("playback-channel-" + num + "-url", url);
        c.writeEntry ("playback-channel-" + num + "-buffer-size", (quint64)buffer_size);
    }

    c.writeEntry("capture-channels", m_CaptureChannelList.size());
    for (int i = 0; i < m_CaptureChannelList.size(); ++i) {
        KUrl                url =  m_CaptureChannelList[i];
        const StreamingJob *j   = *m_CaptureChannelJobs.find(url);
        const QString num       = QString::number(i);

        const SoundFormat &sf          = j->getSoundFormat();
        url                            = j->getURL();
        size_t             buffer_size = j->getBufferSize();

        sf.saveConfig("capture-channel-" + num, c);
        c.writeEntry ("capture-channel-" + num + "-url", url);
        c.writeEntry ("capture-channel-" + num + "-buffer-size", (quint64)buffer_size);
    }
}

void StreamingDevice::restoreState (const KConfigGroup &c)
{
    PluginBase::restoreState(c);

    resetPlaybackStreams(false);
    resetCaptureStreams(false);

    int n = c.readEntry("playback-channels", 0);
    for (int i = 0; i < n; ++i) {
        SoundFormat sf;
        const QString num = QString::number(i);
        sf.restoreConfig("playback-channel-" + num, c);
        KUrl    url         = c.readEntry("playback-channel-" + num + "-url", KUrl());
        size_t  buffer_size = c.readEntry("playback-channel-" + num + "-buffer-size", (quint64)32*1024);

        if (url.isValid()) {
            addPlaybackStream(url, sf, buffer_size, i == n-1);
        }
    }

    n = c.readEntry("capture-channels", 0);
    for (int i = 0; i < n; ++i) {
        SoundFormat sf;
        const QString num = QString::number(i);
        sf.restoreConfig("capture-channel-" + num, c);
        KUrl    url         = c.readEntry("capture-channel-" + num + "-url", KUrl());
        size_t  buffer_size = c.readEntry("capture-channel-" + num + "-buffer-size", (quint64)32*1024);

        if (url.isValid()) {
            addCaptureStream(url, sf, buffer_size, i == n-1);
        }
    }

    if (!m_CaptureChannelList.size()) {
        addCaptureStream(KUrl("/dev/video24"), SoundFormat(48000, 2, 16, true, BYTE_ORDER, "raw"), 32768);
        addCaptureStream(KUrl("/dev/video32"), SoundFormat(48000, 2, 16, true, BYTE_ORDER, "raw"), 32768);
        addCaptureStream(KUrl("/dev/urandom"), SoundFormat(48000, 2, 16, true, BYTE_ORDER, "raw"), 32768);
    }

    // must be last. noticeSoundClientConnected will be called by this.
    setSoundStreamClientID(c.readEntry("soundstreamclient-id", getSoundStreamClientID()));
    emit sigUpdateConfig();
}


ConfigPageInfo  StreamingDevice::createConfigurationPage()
{
    StreamingConfiguration *conf = new StreamingConfiguration(NULL, this);
    QObject::connect(this, SIGNAL(sigUpdateConfig()), conf, SLOT(slotUpdateConfig()));
    return ConfigPageInfo (conf,
                           i18n("Streaming"),
                           i18n("Streaming Device Options"),
                           "kradio_streaming"
                          );
}


bool StreamingDevice::preparePlayback(SoundStreamID id, const QString &channel, bool /*active_mode*/, bool start_immediately)
{
    if (id.isValid() && m_PlaybackChannelJobs.contains(channel)) {
        m_AllPlaybackStreams.insert(id, channel);
        if (start_immediately)
            startPlayback(id);
        return true;
    }
    return false;
}


bool StreamingDevice::prepareCapture(SoundStreamID id, const QString &channel)
{
//     logDebug(QString("StreamingDevice::prepareCapture: SoundStream: %1, channel: %2").arg(id.getID()).arg(channel));
    if (id.isValid() && m_CaptureChannelJobs.contains(channel)) {
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
//     logDebug("StreamingDevice::releaseCapture");
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
    return m_PlaybackChannelJobs.size() > 0;
}


bool StreamingDevice::supportsCapture() const
{
    return m_CaptureChannelJobs.size() > 0;
}


bool StreamingDevice::startPlayback(SoundStreamID id)
{
    if (id.isValid() && m_AllPlaybackStreams.contains(id)) {
        m_EnabledPlaybackStreams.insert(id, m_AllPlaybackStreams[id]);
        StreamingJob *x = *m_PlaybackChannelJobs.find(m_AllPlaybackStreams[id]);
        x->startPlayback();
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


bool StreamingDevice::resumePlayback(SoundStreamID /*id*/)
{
    return false;
}


bool StreamingDevice::stopPlayback(SoundStreamID id)
{
    if (id.isValid() && m_EnabledPlaybackStreams.contains(id)) {
        StreamingJob *x = *m_PlaybackChannelJobs.find(m_AllPlaybackStreams[id]);
        if (x->stopPlayback()) {
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
//     logDebug(QString("StreamingDevice::startCaptureWithFormat: SoundStream: %1").arg(id.getID()));
    if (id.isValid() && m_AllCaptureStreams.contains(id)) {
        m_EnabledCaptureStreams.insert(id, m_AllCaptureStreams[id]);
        StreamingJob *x = *m_CaptureChannelJobs.find(m_AllCaptureStreams[id]);
        x->startCapture(proposed_format, real_format, force_format);
        return true;
    } else {
        return false;
    }
}


bool StreamingDevice::stopCapture(SoundStreamID id)
{
    if (id.isValid() && m_EnabledCaptureStreams.contains(id)) {
        StreamingJob *x = *m_CaptureChannelJobs.find(m_AllCaptureStreams[id]);
        if (x->stopCapture()) {
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
        StreamingJob *x = *m_CaptureChannelJobs.find(m_AllCaptureStreams[id]);
        sf = x->getSoundFormat();
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


bool StreamingDevice::noticeSoundStreamSourceRedirected(SoundStreamID oldID, SoundStreamID newID)
{
    bool found = false;
    if (newID != oldID) {
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


bool StreamingDevice::noticeSoundStreamSinkRedirected(SoundStreamID oldID, SoundStreamID newID)
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
        StreamingJob *x = *m_CaptureChannelJobs.find(m_AllCaptureStreams[id]);
        x->playData(data, size, consumed_size);
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
    StreamingJob *x = *m_CaptureChannelJobs.find(m_AllCaptureStreams[id]);

    while (x->hasRecordedData() && free_size > 0) {
        const char   *buffer        = NULL;
        size_t        size          = SIZE_T_DONT_CARE;
        size_t        consumed_size = SIZE_T_DONT_CARE;
        SoundMetaData meta_data(0,0,0, i18n("internal stream, not stored (%1)", m_AllCaptureStreams[id]));
        x->lockData(buffer, size, meta_data); // get pointer to data and meta-data content
        if (size > free_size)
            size = free_size;
        notifySoundStreamData(id, x->getSoundFormat(), buffer, size, consumed_size, meta_data);
        if (consumed_size == SIZE_T_DONT_CARE)
            consumed_size = size;
        x->removeData(consumed_size);
        free_size -= consumed_size;
        if (consumed_size < size) {
            logWarning(i18n("StreamingDevice %1::notifySoundStreamData: Playback Clients skipped %2 bytes", PluginBase::name(), size-consumed_size));
            break;
        }
    }
    return true;
}



const QStringList &StreamingDevice::getPlaybackChannels() const
{
    return m_PlaybackChannelStringList;
}


const QStringList &StreamingDevice::getCaptureChannels() const
{
    return m_CaptureChannelStringList;
}


QString StreamingDevice::getSoundStreamClientDescription() const
{
    return i18n("Streaming Device %1", PluginBase::name());
}


void   StreamingDevice::logStreamError(const KUrl &url, const QString &s)
{
    logError(i18n("Streaming Device %1, %2: %3", PluginBase::name(), url.pathOrUrl(), s));
}

void   StreamingDevice::logStreamWarning(const KUrl &url, const QString &s)
{
    logWarning(i18n("Streaming Device %1, %2: %3", PluginBase::name(), url.pathOrUrl(), s));
}

void   StreamingDevice::logStreamInfo(const KUrl &url, const QString &s)
{
    logInfo(i18n("Streaming Device %1, %2: %3", PluginBase::name(), url.pathOrUrl(), s));
}

void   StreamingDevice::logStreamDebug(const KUrl &url, const QString &s)
{
    logDebug(i18n("Streaming Device %1, %2: %3", PluginBase::name(), url.pathOrUrl(), s));
}


bool StreamingDevice::getPlaybackStreamOptions(const QString &channel, KUrl &url, SoundFormat &sf, size_t &buffer_size) const
{
    if (m_PlaybackChannelJobs.contains(channel)) {
        const StreamingJob *j = *m_PlaybackChannelJobs.find(channel);
        url = j->getURL();
        sf  = j->getSoundFormat();
        buffer_size = j->getBufferSize();
        return true;
    }
    return false;
}


bool StreamingDevice::getCaptureStreamOptions(const QString &channel, KUrl &url, SoundFormat &sf, size_t &buffer_size) const
{
    if (m_CaptureChannelJobs.contains(channel)) {
        const StreamingJob *j = *m_CaptureChannelJobs.find(channel);
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

    m_PlaybackChannelList      .clear();
    m_PlaybackChannelStringList.clear();

    qDeleteAll(m_PlaybackChannelJobs);
    m_PlaybackChannelJobs.clear();

    if (notification_enabled) {
        notifyPlaybackChannelsChanged(m_SoundStreamClientID, m_PlaybackChannelStringList);
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

    m_CaptureChannelList      .clear();
    m_CaptureChannelStringList.clear();

    qDeleteAll(m_CaptureChannelJobs);
    m_CaptureChannelJobs.clear();

    if (notification_enabled) {
        notifyCaptureChannelsChanged(m_SoundStreamClientID, m_CaptureChannelStringList);
    }
}


void StreamingDevice::addPlaybackStream(const KUrl &url, const SoundFormat &sf, size_t buffer_size, bool notification_enabled)
{
    StreamingJob *x = new StreamingJob(url, sf, buffer_size);
    connect(x,    SIGNAL(logStreamError  (const KUrl &, const QString &)),
            this, SLOT  (logStreamError  (const KUrl &, const QString &)));
    connect(x,    SIGNAL(logStreamWarning(const KUrl &, const QString &)),
            this, SLOT  (logStreamWarning(const KUrl &, const QString &)));
    connect(x,    SIGNAL(logStreamInfo   (const KUrl &, const QString &)),
            this, SLOT  (logStreamInfo   (const KUrl &, const QString &)));
    connect(x,    SIGNAL(logStreamDebug  (const KUrl &, const QString &)),
            this, SLOT  (logStreamDebug  (const KUrl &, const QString &)));

    m_PlaybackChannelList      .append(url);
    m_PlaybackChannelStringList.append(url.pathOrUrl());
    m_PlaybackChannelJobs      .insert(url, x);
    if (notification_enabled) {
        notifyPlaybackChannelsChanged(m_SoundStreamClientID, m_PlaybackChannelStringList);
    }
}


void StreamingDevice::addCaptureStream (const KUrl &url, const SoundFormat &sf, size_t buffer_size, bool notification_enabled)
{
    StreamingJob *x = new StreamingJob(url, sf, buffer_size);
    connect(x,    SIGNAL(logStreamError  (const KUrl &, const QString &)),
            this, SLOT  (logStreamError  (const KUrl &, const QString &)));
    connect(x,    SIGNAL(logStreamWarning(const KUrl &, const QString &)),
            this, SLOT  (logStreamWarning(const KUrl &, const QString &)));
    connect(x,    SIGNAL(logStreamInfo   (const KUrl &, const QString &)),
            this, SLOT  (logStreamInfo   (const KUrl &, const QString &)));
    connect(x,    SIGNAL(logStreamDebug  (const KUrl &, const QString &)),
            this, SLOT  (logStreamDebug  (const KUrl &, const QString &)));

    m_CaptureChannelList      .append(url);
    m_CaptureChannelStringList.append(url.pathOrUrl());
    m_CaptureChannelJobs      .insert(url, x);
    if (notification_enabled) {
        notifyCaptureChannelsChanged(m_SoundStreamClientID, m_CaptureChannelStringList);
    }
}


void StreamingDevice::setName(const QString &n)
{
    PluginBase::setName(n);
    notifyPlaybackChannelsChanged(m_SoundStreamClientID, m_PlaybackChannelStringList);
    notifyCaptureChannelsChanged (m_SoundStreamClientID, m_CaptureChannelStringList);
}



#include "streaming.moc"
