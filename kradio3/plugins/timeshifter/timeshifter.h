/***************************************************************************
                          timeshifter.h  -  description
                             -------------------
    begin                : May 16 2005
    copyright            : (C) 2005 Ernst Martin Witte
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

#ifndef KRADIO_TIMESHIFTER_H
#define KRADIO_TIMESHIFTER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../../src/include/plugins.h"
#include "../../src/include/soundstreamclient_interfaces.h"
#include "../../src/include/fileringbuffer.h"


class TimeShifter : public QObject,
                    public PluginBase,
                    public ISoundStreamClient
{
Q_OBJECT
public:
    TimeShifter (const QString &name);
    virtual ~TimeShifter ();

    virtual bool connectI (Interface *);
    virtual bool disconnectI (Interface *);

    virtual QString pluginClassName() const { return "TimeShifter"; }

    virtual const QString &name() const { return PluginBase::name(); }
    virtual       QString &name()       { return PluginBase::name(); }

    // config

    const QString &getPlaybackMixer()        const { return m_PlaybackMixerID; }
    const QString &getPlaybackMixerChannel() const { return m_PlaybackMixerChannel; }
    const QString &getTempFileName()         const { return m_TempFileName; }
    Q_UINT64       getTempFileMaxSize()      const { return m_TempFileMaxSize; }

    void setTempFile(const QString &filename, Q_UINT64 s);
    bool setPlaybackMixer(const QString &soundStreamClientID, const QString &ch);

    // PluginBase

public:
    virtual void   saveState    (KConfig *) const;
    virtual void   restoreState (KConfig *);

    virtual ConfigPageInfo  createConfigurationPage();
    virtual AboutPageInfo   createAboutPage();

protected:

    ISoundStreamClient *searchPlaybackMixer();

    size_t writeMetaDataToBuffer(const SoundMetaData &md, char *buffer,  size_t buffer_size);
    size_t readMetaDataFromBuffer(SoundMetaData &md, const char *buffer, size_t buffer_size);
    void skipPacketInRingBuffer();

    // SoundStreamClient
    void noticeConnectedI (ISoundStreamServer *s, bool pointer_valid);

    bool startCaptureWithFormat(SoundStreamID      id,
                      const SoundFormat &proposed_format,
                      SoundFormat       &real_format,
                      bool               force_format);
    bool stopCapture(SoundStreamID id);
    bool noticeSoundStreamClosed(SoundStreamID id);
    bool startPlayback(SoundStreamID id);
    bool stopPlayback(SoundStreamID id);
    bool pausePlayback(SoundStreamID id);
    bool noticeSoundStreamData(SoundStreamID id, const SoundFormat &sf, const char *data, size_t size, size_t &consumed_size, const SoundMetaData &md);
    bool noticeReadyForPlaybackData(SoundStreamID id, size_t size);

    bool getSoundStreamDescription(SoundStreamID id, QString &descr) const;

    // FIXME: react on redirect request

signals:

    void sigUpdateConfig();

protected:

    QString             m_TempFileName;
    size_t              m_TempFileMaxSize;
    SoundFormat         m_SoundFormat;
    SoundFormat         m_realSoundFormat;

    QString             m_PlaybackMixerID;
    QString             m_PlaybackMixerChannel;

    QString             m_StreamFile;
    bool                m_StreamPaused;
    SoundStreamID       m_OrgStreamID;
    SoundStreamID       m_NewStreamID;
    SoundFormat         m_RealSoundFormat;
    float               m_orgVolume;

    SoundMetaData       m_PlaybackMetaData;
    size_t              m_PlaybackDataLeftInBuffer;

    FileRingBuffer      m_RingBuffer;
};

#endif
