/***************************************************************************
                          v4lradio.h  -  description
                             -------------------
    begin                : Jan 2002
    copyright            : (C) 2002-2005 Ernst Martin Witte, Klas Kalass
    email                : emw-kradio@nocabal.de, klas@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRADIO_V4LRADIO_H
#define KRADIO_V4LRADIO_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QtCore/QTimer>

#include "radiodevice_interfaces.h"
#include "radio_interfaces.h"
#include "pluginbase.h"
#include "frequencyradiostation.h"
#include "frequencyseekhelper.h"
#include "soundstreamclient_interfaces.h"
#include "v4lcfg_interfaces.h"

#include "rds_decoder.h"

struct video_tuner;
struct video_audio;
#ifdef HAVE_V4L2
struct v4l2_tuner;
#endif

class QSocketNotifier;

class V4LRadio : public QObject,
                 public PluginBase,
                 public IRadioDevice,
                 public IRadioClient,
                 public ISeekRadio,
                 public IFrequencyRadio,
                 public ISoundStreamClient,
                 public IV4LCfg
{
Q_OBJECT
public:
    V4LRadio (const QString &instanceID, const QString &name);
    virtual ~V4LRadio ();

    virtual bool connectI (Interface *);
    virtual bool disconnectI (Interface *);

    virtual QString pluginClassName() const { return "V4LRadio"; }

//     virtual const QString &name() const { return PluginBase::name(); }
//     virtual       QString &name()       { return PluginBase::name(); }

    // PluginBase

public:
    virtual void   saveState    (      KConfigGroup &) const;
    virtual void   restoreState (const KConfigGroup &);
    virtual void   startPlugin();

    virtual ConfigPageInfo  createConfigurationPage();
//     virtual AboutPageInfo   createAboutPage();

    // IRadioDevice methods

RECEIVERS:
    virtual bool setPower(bool p);
    virtual bool powerOn();
    virtual bool powerOff();
    virtual bool activateStation(const RadioStation &rs);

ANSWERS:
    virtual bool                   isPowerOn() const;
    virtual bool                   isPowerOff() const;
    virtual const RadioStation  &  getCurrentStation() const;
    virtual const QString       &  getDescription() const;
    virtual SoundStreamID          getCurrentSoundStreamSinkID() const;
    virtual SoundStreamID          getCurrentSoundStreamSourceID() const;

    virtual bool                   getRDSState      () const;
    virtual const QString       &  getRDSRadioText  () const;
    virtual const QString       &  getRDSStationName() const;

    // IRadioClient

RECEIVERS:
    bool noticePowerChanged(bool /*on*/)                         { return false; }
    bool noticeStationChanged (const RadioStation &, int /*idx*/){ return false; }
    bool noticeStationsChanged(const StationList &sl);
    bool noticePresetFileChanged(const QString &/*f*/)           { return false; }

    bool noticeRDSStateChanged      (bool  /*enabled*/)          { return false; }
    bool noticeRDSRadioTextChanged  (const QString &/*s*/)       { return false; }
    bool noticeRDSStationNameChanged(const QString &/*s*/)       { return false; }

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/) { return false; }
    bool noticeCurrentSoundStreamSinkIDChanged(SoundStreamID /*id*/)   { return false; }

    // ISeekRadio

RECEIVERS:
    virtual bool toBeginning();
    virtual bool toEnd();
    virtual bool startSeek (bool up);
    virtual bool startSeekUp();
    virtual bool startSeekDown();
    virtual bool stopSeek();

ANSWERS:
    virtual bool  isSeekRunning() const;
    virtual bool  isSeekUpRunning() const;
    virtual bool  isSeekDownRunning() const;
    virtual float getProgress () const;


    // IFrequencyRadio

RECEIVERS:
    virtual bool setFrequency(float f, const FrequencyRadioStation *s);
    virtual bool setMinFrequency(float mf);
    virtual bool setMaxFrequency(float mf);
    virtual bool setScanStep(float s);

ANSWERS:
    virtual float getFrequency()           const;
    virtual float getMinFrequency()        const;
    virtual float getMinDeviceFrequency()  const;
    virtual float getMaxFrequency()        const;
    virtual float getMaxDeviceFrequency()  const;
    virtual float getScanStep()            const;


    // ISoundStreamClient: mixer functions


RECEIVERS:
    void noticeConnectedI (ISoundStreamServer *s, bool pointer_valid);
    void noticeConnectedSoundClient(ISoundStreamClient::thisInterface *i, bool pointer_valid);

    bool noticePlaybackChannelsChanged(const QString & /*client_id*/, const QStringList &/*channels*/);
    bool noticeCaptureChannelsChanged (const QString & /*client_id*/, const QStringList &/*channels*/);

    bool setTreble          (SoundStreamID, float v);
    bool setBass            (SoundStreamID, float v);
    bool setBalance         (SoundStreamID, float v);
    bool muteSource         (SoundStreamID, bool mute = true);
    bool unmuteSource       (SoundStreamID, bool unmute = true);
    bool setSignalMinQuality(SoundStreamID, float q);
    bool setStereoMode      (SoundStreamID, StationStereoMode m);

    bool getTreble          (SoundStreamID, float &v) const;
    bool getBass            (SoundStreamID, float &v) const;
    bool getBalance         (SoundStreamID, float &b) const;
    bool getSignalQuality   (SoundStreamID, float &q) const;
    bool getSignalMinQuality(SoundStreamID, float &q) const;
    bool hasGoodQuality     (SoundStreamID, bool &) const;
    bool isStereo           (SoundStreamID, bool &s) const;
    bool isSourceMuted      (SoundStreamID, bool &m) const;

    // ISoundStreamClient: generic stream handling (broadcasts)

RECEIVERS:

    bool getSoundStreamDescription  (SoundStreamID id, QString &descr) const;
    bool getSoundStreamRadioStation (SoundStreamID id, const RadioStation *&rs) const;
    bool enumerateSourceSoundStreams(QMap<QString, SoundStreamID> &list) const;

    bool noticeSoundStreamSinkRedirected  (SoundStreamID oldID, SoundStreamID newID);
    bool noticeSoundStreamSourceRedirected(SoundStreamID oldID, SoundStreamID newID);

//     bool stopCapture(SoundStreamID id); // if active playback also call stopPlayback

    // if the radio is powered off, we will handle the volume by changing m_defaultPlaybackVolume
    bool       setPlaybackVolume(SoundStreamID id, float volume);
    bool       getPlaybackVolume(SoundStreamID id, float &volume) const;


    // IV4LCfg
RECEIVERS:
    bool  setRadioDevice  (const QString &s);
    bool  setPlaybackMixer(const QString &soundStreamClientID, const QString &ch, bool force = false);
    bool  setCaptureMixer (const QString &soundStreamClientID, const QString &ch, bool force = false);
    bool  setDeviceVolume (float v);
    bool  setActivePlayback(bool a, bool muteCaptureChannelPlayback);
    bool  setMuteOnPowerOff(bool a);
    bool  setForceRDSEnabled(bool a);
    bool  setVolumeZeroOnPowerOff(bool a);
    bool  setV4LVersionOverride(V4LVersion vo);

ANSWERS:
    const QString &getRadioDevice         () const { return m_radioDev; }
    const QString &getPlaybackMixerID     () const { return m_PlaybackMixerID; }
    const QString &getCaptureMixerID      () const { return m_CaptureMixerID; }
    const QString &getPlaybackMixerChannel() const { return m_PlaybackMixerChannel; }
    const QString &getCaptureMixerChannel () const { return m_CaptureMixerChannel; }
    float          getDeviceVolume        () const;
    V4LCaps        getCapabilities(const QString &dev = QString::null) const;

    bool           getActivePlayback(bool & muteCaptureChannelPlayback) const { muteCaptureChannelPlayback = m_ActivePlaybackMuteCaptureChannelPlayback; return m_ActivePlayback; }
    bool           getMuteOnPowerOff()                                  const { return m_MuteOnPowerOff; }
    bool           getForceRDSEnabled()                                 const { return m_RDSForceEnabled; }
    bool           getVolumeZeroOnPowerOff()                            const { return m_VolumeZeroOnPowerOff; }
    V4LVersion     getV4LVersionOverride()                              const { return m_V4L_version_override; }

    // anything else


protected:

    INLINE_IMPL_DEF_noticeConnectedI(IErrorLogClient);
    INLINE_IMPL_DEF_noticeConnectedI(IRadioDevice);
    INLINE_IMPL_DEF_noticeConnectedI(IRadioClient);
    INLINE_IMPL_DEF_noticeConnectedI(ISeekRadio);
    INLINE_IMPL_DEF_noticeConnectedI(IFrequencyRadio);
    INLINE_IMPL_DEF_noticeConnectedI(IV4LCfg);

protected slots:
    void    poll();
    void    slotRDSData(int socket);

protected:
    V4LCaps readV4LCaps(const QString &device) const;
    void    radio_init();
    void    radio_done();

    bool    readTunerInfo() const;
    bool    updateAudioInfo(bool write) const;
    bool    readAudioInfo() const { return updateAudioInfo(false); }
    bool    writeAudioInfo() const { return updateAudioInfo(true); }

    void    searchMixers(ISoundStreamClient **playback_mixer, ISoundStreamClient **capture_mixer);

    const FrequencyRadioStation *findMatchingStation(const StationList &sl) const;


    void    processRDSData      (unsigned char buf[3]);
    void    updateRDSState      (bool enabled);
    void    updateRDSStationName(const QString &s);
    void    updateRDSRadioText  (const QString &s);

protected:

    FrequencyRadioStation  m_currentStation;
    mutable float          m_treble;
    mutable float          m_bass;
    mutable float          m_balance;
    mutable float          m_deviceVolume;
    mutable bool           m_muted;
    mutable float          m_signalQuality;
    mutable bool           m_stereo;
    StationStereoMode      m_stereoMode;

    float                  m_minQuality;
    float                  m_minFrequency;
    float                  m_maxFrequency;
    mutable float          m_lastMinDevFrequency;
    mutable float          m_lastMaxDevFrequency;

    float                  m_defaultPlaybackVolume;

    FrequencySeekHelper   *m_seekHelper;
    float                  m_scanStep;

    mutable V4LCaps        m_caps;
    QString                m_radioDev;
    int                    m_radio_fd;

    mutable bool           m_useOldV4L2Calls;


    mutable struct video_audio   *m_audio;
    mutable struct video_tuner   *m_tuner;
#ifdef HAVE_V4L2
    mutable struct v4l2_tuner    *m_tuner2;
#endif

    QTimer                        m_pollTimer;

    struct TunerCache {
        bool  valid;
        float deltaF;
        float minF, maxF;
        TunerCache() { valid = false; deltaF = minF = maxF = 0; }
    };
    mutable struct TunerCache     m_tunercache;


    mutable bool                  m_blockReadTuner,
                                  m_blockReadAudio;

    SoundStreamID                 m_SoundStreamSinkID;
    SoundStreamID                 m_SoundStreamSourceID;
    QString                       m_PlaybackMixerID;
    QString                       m_CaptureMixerID;
    QString                       m_PlaybackMixerChannel;
    QString                       m_CaptureMixerChannel;

    bool                          m_ActivePlayback;
    bool                          m_ActivePlaybackMuteCaptureChannelPlayback;
    bool                          m_MuteOnPowerOff;
    bool                          m_VolumeZeroOnPowerOff;

    bool                          m_restorePowerOn;

    V4LVersion                    m_V4L_version_override;

    QSocketNotifier              *m_RDS_notify;
    mutable bool                  m_RDS_visible;
    QString                       m_RDS_StationName;
    QString                       m_RDS_RadioText;
    RDSDecoder                    m_RDS_decoder;
    int                           m_RDS_errorRate_subsample_counter;
    bool                          m_RDSForceEnabled;

protected slots:
    void slotEmulateRDS();

protected:
    QTimer                        m_RDS_emulate_timer;
    unsigned int                  m_RDS_emulate_pos;
};

#endif
