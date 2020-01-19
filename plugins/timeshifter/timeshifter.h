/***************************************************************************
                          timeshifter.h  -  description
                             -------------------
    begin                : May 16 2005
    copyright            : (C) 2005 Ernst Martin Witte
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

#ifndef KRADIO_TIMESHIFTER_H
#define KRADIO_TIMESHIFTER_H

#include "pluginbase.h"
#include "soundstreamclient_interfaces.h"
#include "fileringbuffer.h"

class KConfigGroup;

class TimeShifter : public QObject,
                    public PluginBase,
                    public ISoundStreamClient
{
Q_OBJECT
public:
    TimeShifter (const QString &instanceID, const QString &name);
    virtual ~TimeShifter ();

    virtual bool connectI    (Interface *) override;
    virtual bool disconnectI (Interface *) override;

    virtual QString pluginClassName() const override { return QString::fromLatin1("TimeShifter"); }

    // config

    const QString &getPlaybackMixer()        const { return m_PlaybackMixerID; }
    const QString &getPlaybackMixerChannel() const { return m_PlaybackMixerChannel; }
    const QString &getTempFileName()         const { return m_TempFileName; }
    quint64        getTempFileMaxSize()      const { return m_TempFileMaxSize; }

    void setTempFile(const QString &filename, quint64  s);
    bool setPlaybackMixer(QString soundStreamClientID, QString ch, bool force);

    // PluginBase

public:
    virtual void   saveState    (      KConfigGroup &) const override;
    virtual void   restoreState (const KConfigGroup &)       override;

    virtual ConfigPageInfo  createConfigurationPage() override;

protected:

    ISoundStreamClient *searchPlaybackMixer();

    size_t writeMetaDataToBuffer(const SoundMetaData &md, char *buffer,  size_t buffer_size);
    size_t writeCurrentStreamPropertiesToBuffer(char *buffer,  size_t buffer_size);
    size_t readMetaDataFromBuffer(SoundMetaData &md, const char *buffer, size_t buffer_size);
    size_t readCurrentStreamPropertiesFromBuffer(const char *buffer,  size_t buffer_size);
    void skipPacketInRingBuffer();

    // SoundStreamClient
    void noticeConnectedI          (ISoundStreamServer *s, bool pointer_valid) override;

    bool startCaptureWithFormat    (SoundStreamID      id,
                                    const SoundFormat &proposed_format,
                                    SoundFormat       &real_format,
                                    bool               force_format) override;
    bool stopCapture               (SoundStreamID id) override;
    bool noticeSoundStreamClosed   (SoundStreamID id) override;
    bool startPlayback             (SoundStreamID id) override;
    bool stopPlayback              (SoundStreamID id) override;
    bool pausePlayback             (SoundStreamID id) override;
    bool resumePlayback            (SoundStreamID id) override;
    bool noticeSoundStreamData     (SoundStreamID id, const SoundFormat &sf, const char *data, size_t size, size_t &consumed_size, const SoundMetaData &md) override;
    bool noticeReadyForPlaybackData(SoundStreamID id, size_t size) override;

    bool getSoundStreamDescription(SoundStreamID id, QString &descr) const override;
    bool getSoundStreamRadioStation(SoundStreamID id, const RadioStation *&rs) const override;
    bool isPlaybackPaused(SoundStreamID id, bool &b) const override;


    bool getSignalQuality   (SoundStreamID, float &q) const override;
    bool hasGoodQuality     (SoundStreamID, bool &)   const override;
    bool isStereo           (SoundStreamID, bool &s)  const override;


    // ISoundStreamClient: mixer functions
RECEIVERS:
    void noticeConnectedSoundClient       (ISoundStreamClient::thisInterface *i, bool pointer_valid)   override;
    bool noticePlaybackChannelsChanged    (const QString & client_id, const QStringList &/*channels*/) override;
    bool noticeSoundStreamSinkRedirected  (SoundStreamID oldID, SoundStreamID newID) override;
    bool noticeSoundStreamSourceRedirected(SoundStreamID oldID, SoundStreamID newID) override;

signals:

    void sigUpdateConfig();


    

protected:

    INLINE_IMPL_DEF_noticeConnectedI(IErrorLogClient);

protected:

    QString             m_TempFileName;
    size_t              m_TempFileMaxSize;
    SoundFormat         m_SoundFormat;
    SoundFormat         m_realSoundFormat;

    QString             m_PlaybackMixerID;
    QString             m_PlaybackMixerChannel;

    QString             m_StreamFile;
    bool                m_StreamPaused;

    SoundStreamID       m_InputStreamID;         // I'm now the new sink for that stream
    SoundStreamID       m_OutputStreamSourceID;  // I'm the source of that stream
    SoundStreamID       m_OutputStreamSinkID;    // Just that I know, where my stream ends

    SoundFormat         m_RealSoundFormat;
    float               m_orgVolume;

    SoundMetaData       m_PlaybackMetaData;
    size_t              m_PlaybackDataLeftInBuffer;

    FileRingBuffer      m_RingBuffer;

    float               m_currentQuality;
    bool                m_currentGoodQuality;
    bool                m_currentStereo;

    QString             m_SoundStreamDescription;
};

#endif
