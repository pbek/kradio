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

#include <QTimer>

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

    virtual bool connectI    (Interface *) override;
    virtual bool disconnectI (Interface *) override;

    virtual QString   pluginClassName() const override { return QString::fromLatin1("V4LRadio"); }

    QList<DeviceInfo> getDeviceProposals(const QString &devdir = QString::fromLatin1("/dev/")) const override;

    // PluginBase

public:
    virtual void   saveState    (      KConfigGroup &) const override;
    virtual void   restoreState (const KConfigGroup &)       override;
    virtual void   startPlugin  ()                           override;

    virtual ConfigPageInfo  createConfigurationPage()        override;

    // IRadioDevice methods


RECEIVERS:
    virtual bool setPower(bool p) override;
    virtual bool powerOn () override;
    virtual bool powerOff() override;
    virtual bool activateStation(const RadioStation &rs) override;

ANSWERS:
    virtual bool                   isPowerOn () const override;
    virtual bool                   isPowerOff() const override;
    virtual const RadioStation  &  getCurrentStation() const override;
    virtual const QString       &  getDescription   () const override;
    virtual SoundStreamID          getCurrentSoundStreamSinkID  () const override;
    virtual SoundStreamID          getCurrentSoundStreamSourceID() const override;

    virtual bool                   getRDSState      () const override;
    virtual const QString       &  getRDSRadioText  () const override;
    virtual const QString       &  getRDSStationName() const override;

    // IRadioClient

RECEIVERS:
    bool noticePowerChanged(bool /*on*/)                          override { return false; }
    bool noticeStationChanged (const RadioStation &, int /*idx*/) override { return false; }
    bool noticeStationsChanged(const StationList &sl)             override;
    bool noticePresetFileChanged(const QUrl &/*f*/)               override { return false; }

    bool noticeRDSStateChanged      (bool  /*enabled*/)           override { return false; }
    bool noticeRDSRadioTextChanged  (const QString &/*s*/)        override { return false; }
    bool noticeRDSStationNameChanged(const QString &/*s*/)        override { return false; }

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/) override { return false; }
    bool noticeCurrentSoundStreamSinkIDChanged(SoundStreamID /*id*/)   override { return false; }

    // ISeekRadio

RECEIVERS:
    virtual bool toBeginning  () override;
    virtual bool toEnd        () override;
    virtual bool startSeek    (bool up) override;
    virtual bool startSeekUp  () override;
    virtual bool startSeekDown() override;
    virtual bool stopSeek     () override;

ANSWERS:
    virtual bool  isSeekRunning    () const override;
    virtual bool  isSeekUpRunning  () const override;
    virtual bool  isSeekDownRunning() const override;
    virtual float getProgress      () const override;


    // IFrequencyRadio

RECEIVERS:
    virtual bool setFrequency   (float f, const FrequencyRadioStation *s) override;
    virtual bool setMinFrequency(float mf) override;
    virtual bool setMaxFrequency(float mf) override;
    virtual bool setScanStep    (float s)  override;

ANSWERS:
    virtual float getFrequency()           const override;
    virtual float getMinFrequency()        const override;
    virtual float getMinDeviceFrequency()  const override;
    virtual float getMaxFrequency()        const override;
    virtual float getMaxDeviceFrequency()  const override;
    virtual float getScanStep()            const override;


    // ISoundStreamClient: mixer functions


RECEIVERS:
    void noticeConnectedI (ISoundStreamServer *s, bool pointer_valid) override;
    void noticeConnectedSoundClient(ISoundStreamClient::thisInterface *i, bool pointer_valid) override;

    bool noticePlaybackChannelsChanged(const QString & /*client_id*/, const QStringList &/*channels*/) override;
    bool noticeCaptureChannelsChanged (const QString & /*client_id*/, const QStringList &/*channels*/) override;

    bool setTreble          (SoundStreamID, float v) override;
    bool setBass            (SoundStreamID, float v) override;
    bool setBalance         (SoundStreamID, float v) override;
    bool muteSource         (SoundStreamID, bool  mute   = true) override;
    bool unmuteSource       (SoundStreamID, bool  unmute = true) override;
    bool setSignalMinQuality(SoundStreamID, float q) override;
    bool setStereoMode      (SoundStreamID, StationStereoMode m) override;

    bool getTreble          (SoundStreamID, float &v) const override;
    bool getBass            (SoundStreamID, float &v) const override;
    bool getBalance         (SoundStreamID, float &b) const override;
    bool getSignalQuality   (SoundStreamID, float &q) const override;
    bool getSignalMinQuality(SoundStreamID, float &q) const override;
    bool hasGoodQuality     (SoundStreamID, bool  &)  const override;
    bool isStereo           (SoundStreamID, bool  &s) const override;
    bool isSourceMuted      (SoundStreamID, bool  &m) const override;

    // ISoundStreamClient: generic stream handling (broadcasts)

RECEIVERS:

    bool getSoundStreamDescription  (SoundStreamID id, QString &descr)          const override;
    bool getSoundStreamRadioStation (SoundStreamID id, const RadioStation *&rs) const override;
    bool enumerateSourceSoundStreams(QMap<QString, SoundStreamID> &list)        const override;

    bool noticeSoundStreamClosed          (SoundStreamID id)                         override;
    bool noticeSoundStreamSinkRedirected  (SoundStreamID oldID, SoundStreamID newID) override;
    bool noticeSoundStreamSourceRedirected(SoundStreamID oldID, SoundStreamID newID) override;

//     bool stopCapture(SoundStreamID id); // if active playback also call stopPlayback

    // if the radio is powered off, we will handle the volume by changing m_defaultPlaybackVolume
    bool       setPlaybackVolume(SoundStreamID id, float volume)        override;
    bool       getPlaybackVolume(SoundStreamID id, float &volume) const override;


    // IV4LCfg
RECEIVERS:
    bool  setRadioDevice  (const QString &s) override;
    bool  setPlaybackMixer(QString soundStreamClientID, QString ch, bool force = false) override;
    bool  setCaptureMixer (QString soundStreamClientID, QString ch, bool force = false) override;
    bool  setDeviceVolume (float v) override;
    bool  setActivePlayback (bool a, bool muteCaptureChannelPlayback) override;
    bool  setMuteOnPowerOff (bool a) override;
    bool  setForceRDSEnabled(bool a) override;
    bool  setDeviceProbeAtStartup(bool a) override;
    bool  setVolumeZeroOnPowerOff(bool a) override;
    bool  setV4LVersionOverride(V4LVersion vo) override;

ANSWERS:
    const QString &getRadioDevice         () const override { return m_radioDev; }
    const QString &getPlaybackMixerID     () const override { return m_PlaybackMixerID; }
    const QString &getCaptureMixerID      () const override { return m_CaptureMixerID; }
    const QString &getPlaybackMixerChannel() const override { return m_PlaybackMixerChannel; }
    const QString &getCaptureMixerChannel () const override { return m_CaptureMixerChannel; }
    float          getDeviceVolume        () const override;
    V4LCaps        getCapabilities      (const QString &dev) const override;

    bool           getActivePlayback(bool & muteCaptureChannelPlayback) const override { muteCaptureChannelPlayback = m_ActivePlaybackMuteCaptureChannelPlayback; return m_ActivePlayback; }
    bool           getMuteOnPowerOff()                                  const override { return m_MuteOnPowerOff; }
    bool           getForceRDSEnabled()                                 const override { return m_RDSForceEnabled; }
    bool           getDeviceProbeAtStartup()                            const override { return m_deviceProbeAtStartup; }
    bool           getVolumeZeroOnPowerOff()                            const override { return m_VolumeZeroOnPowerOff; }
    V4LVersion     getV4LVersionOverride()                              const override { return m_V4L_version_override; }

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
    bool                          m_deviceProbeAtStartup;

    V4LVersion                    m_V4L_version_override;
    bool                          m_V4L_version_override_by_kernel_once;

    QSocketNotifier              *m_RDS_notify;
    mutable bool                  m_RDS_visible;
    QString                       m_RDS_StationName;
    QString                       m_RDS_RadioText;
    RDSDecoder                    m_RDS_decoder;
    int                           m_RDS_errorRate_subsample_counter;
    bool                          m_RDSForceEnabled;


    mutable bool                  m_tunerInfoReported;

protected slots:
    void slotEmulateRDS();

protected:
    QTimer                        m_RDS_emulate_timer;
    unsigned int                  m_RDS_emulate_pos;
};

#endif
