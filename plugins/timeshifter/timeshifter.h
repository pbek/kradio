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

    virtual bool connectI (Interface *);
    virtual bool disconnectI (Interface *);

    virtual QString pluginClassName() const { return "TimeShifter"; }

//     virtual const QString &name() const { return PluginBase::name(); }
//     virtual       QString &name()       { return PluginBase::name(); }

    // config

    const QString &getPlaybackMixer()        const { return m_PlaybackMixerID; }
    const QString &getPlaybackMixerChannel() const { return m_PlaybackMixerChannel; }
    const QString &getTempFileName()         const { return m_TempFileName; }
    quint64        getTempFileMaxSize()      const { return m_TempFileMaxSize; }

    void setTempFile(const QString &filename, quint64  s);
    bool setPlaybackMixer(QString soundStreamClientID, QString ch, bool force);

    // PluginBase

public:
    virtual void   saveState    (      KConfigGroup &) const;
    virtual void   restoreState (const KConfigGroup &);

    virtual ConfigPageInfo  createConfigurationPage();

protected:

    ISoundStreamClient *searchPlaybackMixer();

    size_t writeMetaDataToBuffer(const SoundMetaData &md, char *buffer,  size_t buffer_size);
    size_t writeCurrentStreamPropertiesToBuffer(char *buffer,  size_t buffer_size);
    size_t readMetaDataFromBuffer(SoundMetaData &md, const char *buffer, size_t buffer_size);
    size_t readCurrentStreamPropertiesFromBuffer(const char *buffer,  size_t buffer_size);
    void skipPacketInRingBuffer();

    // SoundStreamClient
    void noticeConnectedI          (ISoundStreamServer *s, bool pointer_valid);

    bool startCaptureWithFormat    (SoundStreamID      id,
                                    const SoundFormat &proposed_format,
                                    SoundFormat       &real_format,
                                    bool               force_format);
    bool stopCapture               (SoundStreamID id);
    bool noticeSoundStreamClosed   (SoundStreamID id);
    bool startPlayback             (SoundStreamID id);
    bool stopPlayback              (SoundStreamID id);
    bool pausePlayback             (SoundStreamID id);
    bool resumePlayback            (SoundStreamID id);
    bool noticeSoundStreamData     (SoundStreamID id, const SoundFormat &sf, const char *data, size_t size, size_t &consumed_size, const SoundMetaData &md);
    bool noticeReadyForPlaybackData(SoundStreamID id, size_t size);

    bool getSoundStreamDescription(SoundStreamID id, QString &descr) const;
    bool getSoundStreamRadioStation(SoundStreamID id, const RadioStation *&rs) const;
    bool isPlaybackPaused(SoundStreamID id, bool &b) const;


    bool getSignalQuality   (SoundStreamID, float &q) const;
    bool hasGoodQuality     (SoundStreamID, bool &)   const;
    bool isStereo           (SoundStreamID, bool &s)  const;


    // ISoundStreamClient: mixer functions
RECEIVERS:
    void noticeConnectedSoundClient       (ISoundStreamClient::thisInterface *i, bool pointer_valid);
    bool noticePlaybackChannelsChanged    (const QString & client_id, const QStringList &/*channels*/);
    bool noticeSoundStreamSinkRedirected  (SoundStreamID oldID, SoundStreamID newID);
    bool noticeSoundStreamSourceRedirected(SoundStreamID oldID, SoundStreamID newID);

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
