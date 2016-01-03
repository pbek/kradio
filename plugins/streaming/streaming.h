/***************************************************************************
                          streaming.h  -  description
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

#ifndef _KRADIO_STREAMING_H
#define _KRADIO_STREAMING_H

#include "pluginbase.h"
#include "soundformat.h"
#include "soundstreamclient_interfaces.h"

#include <QObject>
#include <QMap>
#include <QStringList>

class StreamingJob;

class StreamingDevice : public QObject,
                        public PluginBase,
                        public ISoundStreamClient
{
Q_OBJECT

public:
    StreamingDevice (const QString &instanceID, const QString &name);
    virtual ~StreamingDevice ();

    virtual bool   connectI(Interface *i);
    virtual bool   disconnectI(Interface *i);

    bool getPlaybackStreamOptions(const QString &channel, KUrl &url, SoundFormat &sf, size_t &buffer_size) const;
    bool getCaptureStreamOptions (const QString &channel, KUrl &url, SoundFormat &sf, size_t &buffer_size) const;

    void resetPlaybackStreams(bool notification_enabled = true);
    void resetCaptureStreams(bool notification_enabled = true);
    void addPlaybackStream(const KUrl &url, const SoundFormat &sf, size_t buffer_size, bool notification_enabled = true);
    void addCaptureStream (const KUrl &url, const SoundFormat &sf, size_t buffer_size, bool notification_enabled = true);

    // PluginBase

public:
    virtual void   saveState   (      KConfigGroup &) const;
    virtual void   restoreState(const KConfigGroup &);

    virtual QString pluginClassName() const { return QString::fromLatin1("StreamingDevice"); }

//     virtual const QString &name() const { return PluginBase::name(); }
//     virtual       QString &name()       { return PluginBase::name(); }
    virtual void setName(const QString &n);

    virtual ConfigPageInfo  createConfigurationPage();

    // ISoundStreamClient: direct device access

RECEIVERS:
    void noticeConnectedI (ISoundStreamServer *s, bool pointer_valid);
    INLINE_IMPL_DEF_noticeConnectedI (IErrorLogClient);

    bool preparePlayback(SoundStreamID id, const QString &channel, bool active_mode, bool start_immediately);
    bool prepareCapture(SoundStreamID id,  const QString &channel);
    bool releasePlayback(SoundStreamID id);
    bool releaseCapture(SoundStreamID id);

ANSWERS:
    bool supportsPlayback() const;
    bool supportsCapture()  const;

    QString getSoundStreamClientDescription() const;

    // ISoundStreamClient: mixer access

protected:

ANSWERS:
    const QStringList &getPlaybackChannels() const;
    const QStringList &getCaptureChannels() const;

    // ISoundStreamClient: generic broadcasts

RECEIVERS:
    bool startPlayback(SoundStreamID id);
    bool pausePlayback(SoundStreamID id);
    bool resumePlayback(SoundStreamID id);
    bool stopPlayback(SoundStreamID id);
    bool isPlaybackRunning(SoundStreamID id, bool &b) const;

    bool startCaptureWithFormat(SoundStreamID      id,
                                const SoundFormat &proposed_format,
                                SoundFormat       &real_format,
                                bool               force_format);
    bool stopCapture(SoundStreamID id);
    bool isCaptureRunning(SoundStreamID id, bool &b, SoundFormat &sf) const;

    bool noticeSoundStreamClosed(SoundStreamID id);
    bool noticeSoundStreamSourceRedirected(SoundStreamID oldID, SoundStreamID newID);
    bool noticeSoundStreamSinkRedirected(SoundStreamID oldID, SoundStreamID newID);

    bool noticeReadyForPlaybackData(SoundStreamID id, size_t size);

    bool noticeSoundStreamData(SoundStreamID id,
                               const SoundFormat &,
                               const char *data, size_t size, size_t &consumed_size,
                               const SoundMetaData &md
                              );

public slots:

    void   logStreamError  (const KUrl &url, const QString &s);
    void   logStreamWarning(const KUrl &url, const QString &s);
    void   logStreamInfo   (const KUrl &url, const QString &s);
    void   logStreamDebug  (const KUrl &url, const QString &s);

signals:

    void   sigUpdateConfig();

protected:

    QStringList     m_PlaybackChannelStringList,
                    m_CaptureChannelStringList;

    QList<KUrl>     m_PlaybackChannelList,
                    m_CaptureChannelList;

    QMap<KUrl, StreamingJob*>
                    m_PlaybackChannelJobs,
                    m_CaptureChannelJobs;

    QMap<SoundStreamID, QString>
                    m_AllPlaybackStreams,
                    m_AllCaptureStreams,
                    m_EnabledPlaybackStreams,
                    m_EnabledCaptureStreams;
};



#endif
