/***************************************************************************
                          oss-sound.h  -  description
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

#ifndef _KRADIO_OSS_SOUND_H
#define _KRADIO_OSS_SOUND_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../../src/libkradio/ringbuffer.h"
#include "../../src/libkradio/plugins.h"
#include "../../src/interfaces/soundstreamclient_interfaces.h"

#include <qobject.h>
#include <qtimer.h>

enum DUPLEX_MODE { DUPLEX_UNKNOWN, DUPLEX_FULL, DUPLEX_HALF };


struct SoundStreamConfig
{
    SoundStreamConfig()
        : m_ActiveMode(false),
          m_Channel(-1),
          m_Volume(-1)
       {}

    SoundStreamConfig(int _channel, bool active_mode = true)
        : m_ActiveMode(active_mode),
          m_Channel(_channel),
          m_Volume(-1)
       {}

    SoundStreamConfig(const SoundStreamConfig &c)
        : m_ActiveMode(c.m_ActiveMode),
          m_Channel(c.m_Channel),
          m_Volume(c.m_Volume)
       {}

    bool           m_ActiveMode;
    int            m_Channel;
    float          m_Volume;
};


class OSSSoundDevice : public QObject,
                       public PluginBase,
                       public ISoundStreamClient
{
Q_OBJECT

public:
    OSSSoundDevice (const QString &name);
    ~OSSSoundDevice ();

    virtual bool   connectI(Interface *i);
    virtual bool   disconnectI(Interface *i);

    // PluginBase

public:
    virtual void   saveState (KConfig *) const;
    virtual void   restoreState (KConfig *);

    virtual QString pluginClassName() const { return "OSSSoundDevice"; }

    virtual ConfigPageInfo  createConfigurationPage();
    virtual AboutPageInfo   createAboutPage();

    // ISoundStreamClient: direct device access

RECEIVERS:
    bool preparePlayback(SoundStreamID id, int channel, bool active_mode);
    bool prepareCapture(SoundStreamID id, int channel);

ANSWERS:
    bool supportsPlayback() const;
    bool supportsCapture()  const;

    QString getSoundStreamClientDescription() const;

    // ISoundStreamClient: mixer access

protected:
    void getMixerChannels(int query_playback_or_rec_mask, QMap<int, QString> &retval) const;

ANSWERS:
    const QMap<int, QString> &getPlaybackChannels() const;
    const QMap<int, QString> &getCaptureChannels() const;

RECEIVERS:
    bool setPlaybackVolume(SoundStreamID id, float volume);
    bool setCaptureVolume(SoundStreamID id,  float volume);
    bool getPlaybackVolume(SoundStreamID id, float &volume) const;
    bool getCaptureVolume(SoundStreamID id,  float &volume) const;

    // ISoundStreamClient: generic broadcasts

RECEIVERS:
    bool startPlayback(SoundStreamID id);
    bool pausePlayback(SoundStreamID id);
    bool stopPlayback(SoundStreamID id);
    bool isPlaybackRunning(SoundStreamID id, bool &b) const;

    bool startCaptureWithFormat(SoundStreamID      id,
                      const SoundFormat &proposed_format,
                      SoundFormat       &real_format,
                      bool               force_format);
    bool stopCapture(SoundStreamID id);
    bool isCaptureRunning(SoundStreamID id, bool &b) const;

    bool noticeSoundStreamClosed(SoundStreamID id);
    bool noticeSoundStreamRedirected(SoundStreamID oldID, SoundStreamID newID);

    bool noticeSoundStreamData(SoundStreamID id,
                               const SoundFormat &,
                               const char *data, unsigned size,
                               const SoundMetaData &md
                              );


    // Config Access

    int            getBufferSize()      const { return m_BufferSize; }
    bool           isPlaybackEnabled()  const { return m_EnablePlayback; }
    bool           isCaptureEnabled()   const { return m_EnableCapture;  }
    const QString &getDSPDeviceName()   const { return m_DSPDeviceName; }
    const QString &getMixerDeviceName() const { return m_MixerDeviceName; }

    void           setBufferSize(int s);
    void           enablePlayback(bool on);
    void           enableCapture(bool on);
    void           setDSPDeviceName(const QString &s);
    void           setMixerDeviceName(const QString &dev_name);

    // own functions

    static int getOSSFormat(const SoundFormat &f);

protected slots:

    void   slotPoll();

signals:

    void   sigUpdateConfig();

protected:

    bool   openDSPDevice(const SoundFormat &format, bool reopen = false);
    bool   closeDSPDevice(bool force = false);

    bool   openMixerDevice(bool reopen = false);
    bool   closeMixerDevice(bool force = false);

    void   checkMixerVolume(SoundStreamID id);
    float  readMixerVolume(int channel) const;
    float  writeMixerVolume(int channel, float vol);

    void   selectCaptureChannel (int channel);

    QString         m_DSPDeviceName,
                    m_MixerDeviceName;
    int             m_DSP_fd,
                    m_Mixer_fd;
    DUPLEX_MODE     m_DuplexMode;
    SoundFormat     m_DSPFormat;

    QMap<int, QString>   m_PlaybackChannels,
                         m_CaptureChannels;

    QMap<SoundStreamID, SoundStreamConfig>
                    m_PlaybackStreams,
                    m_CaptureStreams;

    QValueList<SoundStreamID>
                    m_PassivePlaybackStreams;
    SoundStreamID   m_PlaybackStreamID,
                    m_CaptureStreamID;

    unsigned        m_BufferSize;
    RingBuffer      m_PlaybackBuffer,
                    m_CaptureBuffer;

    unsigned        m_CaptureRequestCounter;
    Q_UINT64        m_CapturePos;
    time_t          m_CaptureStartTime;


    unsigned        m_PlaybackSkipCount,
                    m_CaptureSkipCount;

    bool            m_EnablePlayback,
                    m_EnableCapture;

    QTimer          m_PollingTimer;
};



#endif
