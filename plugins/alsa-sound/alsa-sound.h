/***************************************************************************
                          alsa-sound.h  -  description
                             -------------------
    begin                : Thu May 26 2005
    copyright            : (C) 2005 by Martin Witte
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

#ifndef _KRADIO_ALSA_SOUND_H
#define _KRADIO_ALSA_SOUND_H

#include "ringbuffer.h"
#include "pluginbase.h"
#include "soundstreamclient_interfaces.h"

#include "alsa-config-mixer-setting.h"

#include <QObject>
#include <QTimer>

#include <alsa/asoundlib.h>


#include "alsa-thread.h"
#include "alsa-sounddevice-metadata.h"

enum DUPLEX_MODE { DUPLEX_UNKNOWN, DUPLEX_FULL, DUPLEX_HALF };

struct SoundStreamConfig
{
    SoundStreamConfig()
        : m_ActiveMode(false),
          m_Channel(),
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


class AlsaCaptureThread;

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
                        public ISoundStreamClient,
                        public ThreadLoggingClient
{
Q_OBJECT

public:
    AlsaSoundDevice (const QString &instanceID, const QString &name);
    virtual ~AlsaSoundDevice ();

    virtual bool   connectI(Interface *i)    override;
    virtual bool   disconnectI(Interface *i) override;


public:

    static QList<AlsaSoundDeviceMetaData> getPCMCaptureDeviceDescriptions();
    static QList<AlsaSoundDeviceMetaData> getPCMPlaybackDeviceDescriptions();
    static QList<AlsaSoundDeviceMetaData> getPCMDeviceDescriptions(const QString &filter);
    static QList<AlsaMixerMetaData>       getCaptureMixerDescriptions();
    static QList<AlsaMixerMetaData>       getPlaybackMixerDescriptions();
    static QList<AlsaMixerMetaData>       getMixerDeviceDescriptions(const QString &filter);
    static QString                        extractMixerNameFromPCMDevice(const QString &devString);

    // PluginBase

public:
    virtual void   saveState    (      KConfigGroup &) const override;
    virtual void   restoreState (const KConfigGroup &)       override;

    virtual QString pluginClassName() const override { return QString::fromLatin1("AlsaSoundDevice"); }

    virtual void setName(const QString &n);

    virtual ConfigPageInfo  createConfigurationPage() override;

    // ISoundStreamClient: direct device access

RECEIVERS:
    void noticeConnectedI (ISoundStreamServer *s, bool pointer_valid) override;
    bool preparePlayback(SoundStreamID id, const QString &channel, bool active_mode, bool start_immediately) override;
    bool prepareCapture (SoundStreamID id, const QString &channel) override;
    bool releasePlayback(SoundStreamID id) override;
    bool releaseCapture (SoundStreamID id) override;

ANSWERS:
    bool supportsPlayback() const override;
    bool supportsCapture()  const override;

    QString getSoundStreamClientDescription() const override;

    // ISoundStreamClient: mixer access

public:
    static
    void getPlaybackMixerChannels(const QString &mixerName,
                                  snd_mixer_t   *mixer_handle,
                                  QStringList   &retval, QMap<QString, AlsaMixerElement> &int2id,
                                  bool           playback_enabled);
    static
    void getCaptureMixerChannels (const QString &mixerName,
                                  snd_mixer_t   *mixer_handle,
                                  QStringList   &vol_list, QMap<QString, AlsaMixerElement> &vol_ch2id,
                                  QStringList   &sw_list,  QMap<QString, AlsaMixerElement> &sw_ch2id,
                                  QStringList   *all_list,
                                  bool           capture_enabled);

ANSWERS:
    const QStringList &getPlaybackChannels() const override;
    const QStringList &getCaptureChannels () const override;

RECEIVERS:
    bool setPlaybackVolume    (SoundStreamID id, float volume)        override;
    bool setCaptureVolume     (SoundStreamID id, float volume)        override;
    bool getPlaybackVolume    (SoundStreamID id, float &volume) const override;
    bool getCaptureVolume     (SoundStreamID id, float &volume) const override;

    bool muteSink             (SoundStreamID id, bool mute)     override;
    bool unmuteSink           (SoundStreamID id, bool unmute)   override;
    bool isSinkMuted          (SoundStreamID id, bool &m) const override;
    bool muteSourcePlayback   (SoundStreamID id, bool mute)     override;
    bool unmuteSourcePlayback (SoundStreamID id, bool unmute)   override;
    bool isSourcePlaybackMuted(SoundStreamID id, bool &m) const override;


    // ISoundStreamClient: generic broadcasts

RECEIVERS:
    bool startPlayback    (SoundStreamID id) override;
    bool pausePlayback    (SoundStreamID id) override;
    bool resumePlayback   (SoundStreamID id) override;
    bool stopPlayback     (SoundStreamID id) override;
    bool isPlaybackRunning(SoundStreamID id, bool &b) const override;

    bool startCaptureWithFormat(SoundStreamID      id,
                      const SoundFormat &proposed_format,
                      SoundFormat       &real_format,
                      bool               force_format) override;
    bool stopCapture     (SoundStreamID id) override;
    bool isCaptureRunning(SoundStreamID id, bool &b, SoundFormat &sf) const override;

    bool noticeSoundStreamClosed          (SoundStreamID id) override;
    bool noticeSoundStreamSinkRedirected  (SoundStreamID oldID, SoundStreamID newID) override;
    bool noticeSoundStreamSourceRedirected(SoundStreamID oldID, SoundStreamID newID) override;

    bool noticeSoundStreamData(SoundStreamID id,
                               const SoundFormat &,
                               const char *data, size_t size, size_t &consumed_size,
                               const SoundMetaData &md
                              ) override;


    // Config Access

    size_t         getPlaybackBufferSize()  const { return m_PlaybackBufferSize;  }
    size_t         getPlaybackChunkSize()   const { return m_PlaybackChunkSize;   }
    size_t         getCaptureBufferSize()   const { return m_CaptureBufferSize;   }
    size_t         getCaptureChunkSize()    const { return m_CaptureChunkSize;    }
    bool           isNonBlockingPlayback()  const { return m_nonBlockingPlayback; }
    bool           isNonBlockingCapture()   const { return m_nonBlockingCapture;  }
    bool           isPlaybackEnabled()      const { return m_EnablePlayback; }
    bool           isCaptureEnabled()       const { return m_EnableCapture;  }
    const QString &getPlaybackDeviceName()  const { return m_PlaybackDeviceName; }
    const QString &getPlaybackMixerName()   const { return m_PlaybackMixerName; }
    const QString &getCaptureDeviceName()   const { return m_CaptureDeviceName; }
    const QString &getCaptureMixerName()    const { return m_CaptureMixerName; }
    const QMap<QString, AlsaConfigMixerSetting> &
                   getCaptureMixerSettings() const { return m_CaptureMixerSettings; }
    bool           getSoftPlaybackVolume(double &correction_factor) const { correction_factor = m_SoftPlaybackVolumeCorrectionFactor; return m_SoftPlaybackVolumeEnabled; }
    double         getSoftPlaybackVolumeValue()                     const { return m_SoftPlaybackVolumeMuted ? 0 : m_SoftPlaybackVolume; }
    bool           getCaptureFormatOverride(SoundFormat &sf);

    void           setBufferSizes(size_t playback_size, size_t playback_chunk_size, size_t capture_size, size_t capture_chunk_size);
    void           setNonBlockingFlags(bool playback_flag, bool capture_flag);
    void           enablePlayback(bool on);
    void           enableCapture(bool on);
    void           setPlaybackDevice(const QString &deviceName, bool force = false);
    void           setPlaybackMixer (const QString &mixerName,  bool force = false);
    void           setCaptureDevice (const QString &deviceName, bool force = false);
    void           setCaptureMixer  (const QString &mixerName,  bool force = false);
    void           setCaptureMixerSettings(const QMap<QString, AlsaConfigMixerSetting> &map);
    void           setSoftPlaybackVolume(bool enable, double correction_factor);
    void           setCaptureFormatOverride(bool override_enabled, const SoundFormat &sf);


    unsigned       getWorkaroundPlaybackSleepTime() const { return m_workaroundSleepPlaybackMilliSeconds; }
    unsigned       getWorkaroundCaptureSleepTime()  const { return m_workaroundSleepCaptureMilliSeconds;  }
    void           setWorkaroundSleepTime(unsigned play, unsigned cap) { m_workaroundSleepPlaybackMilliSeconds = play; m_workaroundSleepCaptureMilliSeconds = cap; }

// pure virtual members of ThreadLoggingClient
protected:
    IErrorLogClient    *getErrorLogClient() override;

protected slots:

    void   slotPollPlayback();
    void   slotPollCapture();

signals:

    void   sigUpdateConfig();

protected:
    INLINE_IMPL_DEF_noticeConnectedI(IErrorLogClient);

    bool   openAlsaDevice(snd_pcm_t *&alsa_handle, SoundFormat &format, const QByteArray &pcm_name_ba, snd_pcm_stream_t stream, int flags, unsigned &latency, size_t buffer_size, size_t chunk_size);

    bool   openPlaybackDevice (const SoundFormat &format, bool reopen = false);
    bool   openCaptureDevice  (const SoundFormat &format, bool reopen = false);
    bool   closePlaybackDevice(bool force = false);
    bool   closeCaptureDevice (bool force = false);

           bool   openPlaybackMixerDevice (bool reopen = false);
           bool   openCaptureMixerDevice  (bool reopen = false);
    static bool   openMixerDevice(snd_mixer_t *&mixer_handle, const QString &mixerName, bool reopen, QTimer *timer, int timer_latency);
           bool   closeCaptureMixerDevice (bool force = false);
           bool   closePlaybackMixerDevice(bool force = false);
    static bool   closeMixerDevice(snd_mixer_t *&mixer_handle, const QString &mixerName, SoundStreamID id, snd_pcm_t *pcm_handle, bool force, QTimer *timer);

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
    QString         m_PlaybackDeviceName;
    QString         m_PlaybackMixerName;
    QString         m_CaptureDeviceName;
    QString         m_CaptureMixerName;

    unsigned        m_PlaybackLatency;
    unsigned        m_CaptureLatency;

    unsigned        m_workaroundSleepPlaybackMilliSeconds;
    unsigned        m_workaroundSleepCaptureMilliSeconds;

    QStringList     m_PlaybackChannels,
                    m_CaptureChannels,
                    m_CaptureChannelsSwitch;

    QMap<QString, AlsaMixerElement> m_PlaybackChannels2ID,
                                    m_CaptureChannels2ID,
                                    m_CaptureChannelsSwitch2ID;

    QMap<SoundStreamID, SoundStreamConfig>
                    m_PlaybackStreams,
                    m_CaptureStreams;

    QList<SoundStreamID>
                    m_PassivePlaybackStreams;
    SoundStreamID   m_PlaybackStreamID,
                    m_CaptureStreamID;

    bool            m_nonBlockingPlayback;
    bool            m_nonBlockingCapture;

    size_t          m_PlaybackChunkSize;
    size_t          m_PlaybackBufferSize;
    size_t          m_CaptureChunkSize;
    size_t          m_CaptureBufferSize;
    RingBuffer      m_PlaybackBuffer,
                    m_CaptureBuffer;
    int             m_PlaybackBufferWaitForMinFill; // in percent

    unsigned        m_CaptureRequestCounter;
    quint64         m_CapturePos;
    time_t          m_CaptureStartTime;

    bool            m_EnablePlayback,
                    m_EnableCapture;

    QTimer          m_PlaybackPollingTimer;
    QTimer          m_CapturePollingTimer;

//     AlsaCaptureThread  *m_captureThread;

    QMap<QString, AlsaConfigMixerSetting> m_CaptureMixerSettings;


    float           m_SoftPlaybackVolumeCorrectionFactor;
    bool            m_SoftPlaybackVolumeEnabled;
    float           m_SoftPlaybackVolume;
    bool            m_SoftPlaybackVolumeMuted;

    bool            m_CaptureFormatOverrideEnable;
    SoundFormat     m_CaptureFormatOverride;


public:
    char  *getPlaybackData(size_t &buffersize, size_t &maxAvailableData);
    void   freePlaybackData(size_t bytes);
    void   lockPlaybackBufferTransaction();
    void   unlockPlaybackBufferTransaction();
    void   setWaitForMinPlaybackBufferFill(int percent);
    size_t getPlaybackBufferMinFill() const;

    char  *getFreeCaptureBuffer(size_t &bufsize);
    void   removeFreeCaptureBufferSpace(size_t bytesRead);
    void   lockCaptureBufferTransaction();
    void   unlockCaptureBufferTransaction();

protected:

    void  checkThreadErrorsAndWarning();

protected:

    bool            m_use_threads;

    AlsaThread     *m_playbackThread;
    AlsaThread     *m_captureThread;

    QString         m_i18nLogPrefixPlayback;
    QString         m_i18nLogPrefixCapture;
};



#endif
