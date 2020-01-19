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

    virtual bool   connectI   (Interface *i) override;
    virtual bool   disconnectI(Interface *i) override;

    bool getPlaybackStreamOptions(const QString &channel, QUrl &url, SoundFormat &sf, size_t &buffer_size) const;
    bool getCaptureStreamOptions (const QString &channel, QUrl &url, SoundFormat &sf, size_t &buffer_size) const;

    void resetPlaybackStreams(bool notification_enabled = true);
    void resetCaptureStreams(bool notification_enabled = true);
    void addPlaybackStream(const QUrl &url, const SoundFormat &sf, size_t buffer_size, bool notification_enabled = true);
    void addCaptureStream (const QUrl &url, const SoundFormat &sf, size_t buffer_size, bool notification_enabled = true);

    // PluginBase

public:
    virtual void   saveState   (      KConfigGroup &) const override;
    virtual void   restoreState(const KConfigGroup &)       override;

    virtual QString pluginClassName() const override { return QString::fromLatin1("StreamingDevice"); }

    virtual void setName(const QString &n);

    virtual ConfigPageInfo  createConfigurationPage() override;

    // ISoundStreamClient: direct device access

RECEIVERS:
    void noticeConnectedI (ISoundStreamServer *s, bool pointer_valid) override;
    INLINE_IMPL_DEF_noticeConnectedI (IErrorLogClient);

    bool preparePlayback(SoundStreamID id, const QString &channel, bool active_mode, bool start_immediately) override;
    bool prepareCapture (SoundStreamID id, const QString &channel) override;
    bool releasePlayback(SoundStreamID id) override;
    bool releaseCapture (SoundStreamID id) override;

ANSWERS:
    bool supportsPlayback() const override;
    bool supportsCapture()  const override;

    QString getSoundStreamClientDescription() const override;

    // ISoundStreamClient: mixer access

protected:

ANSWERS:
    const QStringList &getPlaybackChannels() const override;
    const QStringList &getCaptureChannels () const override;

    // ISoundStreamClient: generic broadcasts

RECEIVERS:
    bool startPlayback (SoundStreamID id) override;
    bool pausePlayback (SoundStreamID id) override;
    bool resumePlayback(SoundStreamID id) override;
    bool stopPlayback  (SoundStreamID id) override;
    bool isPlaybackRunning(SoundStreamID id, bool &b) const override;

    bool startCaptureWithFormat(SoundStreamID      id,
                                const SoundFormat &proposed_format,
                                SoundFormat       &real_format,
                                bool               force_format) override;
    bool stopCapture     (SoundStreamID id) override;
    bool isCaptureRunning(SoundStreamID id, bool &b, SoundFormat &sf) const override;

    bool noticeSoundStreamClosed          (SoundStreamID id)                         override;
    bool noticeSoundStreamSourceRedirected(SoundStreamID oldID, SoundStreamID newID) override;
    bool noticeSoundStreamSinkRedirected  (SoundStreamID oldID, SoundStreamID newID) override;

    bool noticeReadyForPlaybackData(SoundStreamID id, size_t size) override;

    bool noticeSoundStreamData(SoundStreamID id,
                               const SoundFormat &,
                               const char *data, size_t size, size_t &consumed_size,
                               const SoundMetaData &md
                              ) override;

public slots:

    void   logStreamError  (const QUrl &url, const QString &s);
    void   logStreamWarning(const QUrl &url, const QString &s);
    void   logStreamInfo   (const QUrl &url, const QString &s);
    void   logStreamDebug  (const QUrl &url, const QString &s);

signals:

    void   sigUpdateConfig();

protected:

    QStringList     m_PlaybackChannelStringList,
                    m_CaptureChannelStringList;

    QList<QUrl>     m_PlaybackChannelList,
                    m_CaptureChannelList;

    QMap<QUrl, StreamingJob*>
                    m_PlaybackChannelJobs,
                    m_CaptureChannelJobs;

    QMap<SoundStreamID, QString>
                    m_AllPlaybackStreams,
                    m_AllCaptureStreams,
                    m_EnabledPlaybackStreams,
                    m_EnabledCaptureStreams;
};



#endif
