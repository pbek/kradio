/***************************************************************************
                          alsa-sound.h  -  description
                             -------------------
    begin                : Thu May 26 2005
    copyright            : (C) 2005 by Martin Witte
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

#ifndef _KRADIO_ALSA_SOUND_H
#define _KRADIO_ALSA_SOUND_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../../src/libkradio/ringbuffer.h"
#include "../../src/libkradio/plugins.h"
#include "../../src/interfaces/soundstreamclient_interfaces.h"

#include "alsa-config-mixer-setting.h"

#include <qobject.h>
#include <qtimer.h>
#include <alsa/asoundlib.h>

enum DUPLEX_MODE { DUPLEX_UNKNOWN, DUPLEX_FULL, DUPLEX_HALF };


struct SoundStreamConfig
{
    SoundStreamConfig()
        : m_ActiveMode(false),
          m_Channel(QString::null),
          m_Volume(-1),
          m_Muted(false)
       {}

    SoundStreamConfig(const QString &_channel, bool active_mode = true)
        : m_ActiveMode(active_mode),
          m_Channel(_channel),
          m_Volume(-1),
          m_Muted(false)
       {}

    SoundStreamConfig(const SoundStreamConfig &c)
        : m_ActiveMode(c.m_ActiveMode),
          m_Channel(c.m_Channel),
          m_Volume(c.m_Volume),
          m_Muted(c.m_Muted)
       {}

    bool           m_ActiveMode;
    QString        m_Channel;
    float          m_Volume;
    bool           m_Muted;
};


class AlsaMixerElement
{
public:
    AlsaMixerElement()                           { snd_mixer_selem_id_malloc(&m_ID); }
    AlsaMixerElement(snd_mixer_selem_id_t *id)   { snd_mixer_selem_id_malloc(&m_ID); snd_mixer_selem_id_copy(m_ID, id)    ; }
    AlsaMixerElement(const  AlsaMixerElement &x) { snd_mixer_selem_id_malloc(&m_ID); snd_mixer_selem_id_copy(m_ID, x.m_ID); }
    ~AlsaMixerElement() { snd_mixer_selem_id_free (m_ID); }

     operator snd_mixer_selem_id_t *&() { return m_ID; }

     AlsaMixerElement &operator = (const AlsaMixerElement &x) { snd_mixer_selem_id_copy(m_ID, x.m_ID); return *this; }

protected:
    snd_mixer_selem_id_t *m_ID;
};


class AlsaSoundDevice : public QObject,
                        public PluginBase,
                        public ISoundStreamClient
{
Q_OBJECT

public:
    AlsaSoundDevice (const QString &name);
    virtual ~AlsaSoundDevice ();

    virtual bool   connectI(Interface *i);
    virtual bool   disconnectI(Interface *i);

    // PluginBase

public:
    virtual void   saveState (KConfig *) const;
    virtual void   restoreState (KConfig *);

    virtual QString pluginClassName() const { return "AlsaSoundDevice"; }

    virtual const QString &name() const { return PluginBase::name(); }
    virtual       QString &name()       { return PluginBase::name(); }

    virtual ConfigPageInfo  createConfigurationPage();
    virtual AboutPageInfo   createAboutPage();

    // ISoundStreamClient: direct device access

RECEIVERS:
    void noticeConnectedI (ISoundStreamServer *s, bool pointer_valid);
    bool preparePlayback(SoundStreamID id, const QString &channel, bool active_mode);
    bool prepareCapture(SoundStreamID id, const QString &channel);
    bool releasePlayback(SoundStreamID id);
    bool releaseCapture(SoundStreamID id);

ANSWERS:
    bool supportsPlayback() const;
    bool supportsCapture()  const;

    QString getSoundStreamClientDescription() const;

    // ISoundStreamClient: mixer access

public:
    static
    void getPlaybackMixerChannels(int card, snd_mixer_t *mixer_handle,
                                  QStringList &retval, QMap<QString, AlsaMixerElement> &int2id);
    static
    void getCaptureMixerChannels (int card, snd_mixer_t *mixer_handle,
                                  QStringList &vol_list, QMap<QString, AlsaMixerElement> &vol_ch2id,
                                  QStringList &sw_list,  QMap<QString, AlsaMixerElement> &sw_ch2id,
                                  QStringList *all_list = NULL);

ANSWERS:
    const QStringList &getPlaybackChannels() const;
    const QStringList &getCaptureChannels() const;

RECEIVERS:
    bool setPlaybackVolume(SoundStreamID id, float volume);
    bool setCaptureVolume(SoundStreamID id,  float volume);
    bool getPlaybackVolume(SoundStreamID id, float &volume) const;
    bool getCaptureVolume(SoundStreamID id,  float &volume) const;

    bool mute (SoundStreamID id, bool mute);
    bool unmute (SoundStreamID id, bool unmute);
    bool isMuted(SoundStreamID id, bool &m) const;


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
    bool isCaptureRunning(SoundStreamID id, bool &b, SoundFormat &sf) const;

    bool noticeSoundStreamClosed(SoundStreamID id);
    bool noticeSoundStreamRedirected(SoundStreamID oldID, SoundStreamID newID);

    bool noticeSoundStreamData(SoundStreamID id,
                               const SoundFormat &,
                               const char *data, size_t size,
                               const SoundMetaData &md
                              );


    // Config Access

    int            getHWBufferSize()    const { return m_HWBufferSize; }
    int            getBufferSize()      const { return m_BufferSize; }
    bool           isPlaybackEnabled()  const { return m_EnablePlayback; }
    bool           isCaptureEnabled()   const { return m_EnableCapture;  }
    int            getPlaybackCard()    const { return m_PlaybackCard; }
    int            getPlaybackDevice()  const { return m_PlaybackDevice; }
    int            getCaptureCard()     const { return m_CaptureCard; }
    int            getCaptureDevice()   const { return m_CaptureDevice; }
    const QMap<QString, AlsaConfigMixerSetting> &
                   getCaptureMixerSettings() const { return m_CaptureMixerSettings; }

    void           setHWBufferSize(int s);
    void           setBufferSize(int s);
    void           enablePlayback(bool on);
    void           enableCapture(bool on);
    void           setPlaybackDevice(int card, int device);
    void           setCaptureDevice(int card, int device);
    void           setCaptureMixerSettings(const QMap<QString, AlsaConfigMixerSetting> &map);

protected slots:

    void   slotPollPlayback();
    void   slotPollCapture();

signals:

    void   sigUpdateConfig();

protected:
    bool   openAlsaDevice(snd_pcm_t *&alsa_handle, SoundFormat &format, const char *pcm_name, snd_pcm_stream_t stream, unsigned &latency);

    bool   openPlaybackDevice (const SoundFormat &format, bool reopen = false);
    bool   openCaptureDevice  (const SoundFormat &format, bool reopen = false);
    bool   closePlaybackDevice(bool force = false);
    bool   closeCaptureDevice (bool force = false);

    bool   openPlaybackMixerDevice (bool reopen = false);
    bool   openCaptureMixerDevice  (bool reopen = false);
    static bool   openMixerDevice(snd_mixer_t *&mixer_handle, int card, bool reopen, QTimer *timer, int timer_latency);
    bool   closeCaptureMixerDevice (bool force = false);
    bool   closePlaybackMixerDevice(bool force = false);
    static bool   closeMixerDevice(snd_mixer_t *&mixer_handle, int card, SoundStreamID id, snd_pcm_t *pcm_handle, bool force, QTimer *timer);

    void   checkMixerVolume(SoundStreamID id);
    float  readPlaybackMixerVolume(const QString &channel, bool &muted) const;
    float  readCaptureMixerVolume(const QString &channel) const;
    bool   writePlaybackMixerVolume(const QString &channel, float &vol, bool muted);
    bool   writeCaptureMixerVolume(const QString &channel, float &vol);
    bool   writeCaptureMixerSwitch(const QString &channel, bool capture);

    void   selectCaptureChannel (const QString &channel);

    /* ALSA HANDLES */
    snd_pcm_t      *m_hPlayback;
    snd_pcm_t      *m_hCapture;
    snd_mixer_t    *m_hPlaybackMixer;
    snd_mixer_t    *m_hCaptureMixer;

    SoundFormat     m_PlaybackFormat;
    SoundFormat     m_CaptureFormat;
    int             m_PlaybackCard;
    int             m_PlaybackDevice;
    int             m_CaptureCard;
    int             m_CaptureDevice;

    unsigned        m_PlaybackLatency;
    unsigned        m_CaptureLatency;

    QStringList     m_PlaybackChannels,
                    m_CaptureChannels,
                    m_CaptureChannelsSwitch;

    QMap<QString, AlsaMixerElement> m_PlaybackChannels2ID,
                                    m_CaptureChannels2ID,
                                    m_CaptureChannelsSwitch2ID;

    QMap<SoundStreamID, SoundStreamConfig>
                    m_PlaybackStreams,
                    m_CaptureStreams;

    QValueList<SoundStreamID>
                    m_PassivePlaybackStreams;
    SoundStreamID   m_PlaybackStreamID,
                    m_CaptureStreamID;

    size_t          m_HWBufferSize;
    size_t          m_BufferSize;
    RingBuffer      m_PlaybackBuffer,
                    m_CaptureBuffer;

    unsigned        m_CaptureRequestCounter;
    Q_UINT64        m_CapturePos;
    time_t          m_CaptureStartTime;

    size_t          m_PlaybackSkipCount,
                    m_CaptureSkipCount;

    bool            m_EnablePlayback,
                    m_EnableCapture;

    QTimer          m_PlaybackPollingTimer;
    QTimer          m_CapturePollingTimer;

    QMap<QString, AlsaConfigMixerSetting> m_CaptureMixerSettings;

};



#endif
