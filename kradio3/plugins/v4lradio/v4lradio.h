/***************************************************************************
                          v4lradio.h  -  description
                             -------------------
    begin                : Jan 2002
    copyright            : (C) 2002-2005 Ernst Martin Witte, Klas Kalass
    email                : witte@kawo1.rwth-aachen.de, klas@kde.org
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

#include <qtimer.h>

#include "../../src/include/radiodevice_interfaces.h"
#include "../../src/include/plugins.h"
#include "../../src/include/frequencyradiostation.h"
#include "../../src/include/frequencyseekhelper.h"
#include "../../src/include/soundstreamclient_interfaces.h"
#include "v4lcfg_interfaces.h"


struct video_tuner;
struct video_audio;
#ifdef HAVE_V4L2
struct v4l2_tuner;
#endif

class V4LRadio : public QObject,
                 public PluginBase,
                 public IRadioDevice,
//                 public IRadioSound,
                 public ISeekRadio,
                 public IFrequencyRadio,
                 public ISoundStreamClient,
                 public IV4LCfg
{
Q_OBJECT
public:
    V4LRadio (const QString &name);
    virtual ~V4LRadio ();

    virtual bool connectI (Interface *);
    virtual bool disconnectI (Interface *);

    virtual QString pluginClassName() const { return "V4LRadio"; }

    virtual const QString &name() const { return PluginBase::name(); }
    virtual       QString &name()       { return PluginBase::name(); }

    // PluginBase

public:
    virtual void   saveState (KConfig *) const;
    virtual void   restoreState (KConfig *);
    virtual void   startPlugin();

    virtual ConfigPageInfo  createConfigurationPage();
    virtual AboutPageInfo   createAboutPage();

    // IRadioDevice methods

RECEIVERS:
    virtual bool setPower(bool p);
    virtual bool powerOn();
    virtual bool powerOff();
    virtual bool activateStation(const RadioStation &rs);

ANSWERS:
    virtual bool                   isPowerOn() const;
    virtual bool                   isPowerOff() const;
    virtual SoundStreamID          getSoundStreamID() const;
    virtual const RadioStation  &  getCurrentStation() const;
    virtual const QString       &  getDescription() const;
    virtual SoundStreamID          getCurrentSoundStreamID() const;


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
    virtual bool setFrequency(float f);
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

    bool setTreble  (SoundStreamID, float v);
    bool setBass    (SoundStreamID, float v);
    bool setBalance (SoundStreamID, float v);
    bool mute (SoundStreamID, bool mute = true);
    bool unmute (SoundStreamID, bool unmute = true);
    bool setSignalMinQuality(SoundStreamID, float q);
    bool setStereo(SoundStreamID, bool s);

    bool getTreble(SoundStreamID, float &v) const;
    bool getBass  (SoundStreamID, float &v) const;
    bool getBalance (SoundStreamID, float &b) const;
    bool getSignalQuality(SoundStreamID, float &q) const;
    bool getSignalMinQuality(SoundStreamID, float &q) const;
    bool hasGoodQuality(SoundStreamID, bool &) const;
    bool isStereo(SoundStreamID, bool &s) const;
    bool isMuted(SoundStreamID, bool &m) const;

    // ISoundStreamClient: generic stream handling (broadcasts)

RECEIVERS:

    bool getSoundStreamDescription(SoundStreamID id, QString &descr) const;
    bool getSoundStreamRadioStation(SoundStreamID id, const RadioStation *&rs) const;
    bool enumerateSoundStreams(QMap<QString, SoundStreamID> &list) const;

//     bool stopCapture(SoundStreamID id); // if active playback also call stopPlayback


    // IV4LCfg
RECEIVERS:
    bool  setRadioDevice  (const QString &s);
    bool  setPlaybackMixer(const QString &soundStreamClientID, const QString &ch);
    bool  setCaptureMixer (const QString &soundStreamClientID, const QString &ch);
    bool  setDeviceVolume (float v);
    bool  setActivePlayback(bool a);
    bool  setMuteOnPowerOff(bool a);
    bool  setVolumeZeroOnPowerOff(bool a);

    // if the radio is powered off, we will handle the volume by changing m_defaultPlaybackVolume
    bool setPlaybackVolume(SoundStreamID id, float volume);
    bool getPlaybackVolume(SoundStreamID id, float &volume) const;

ANSWERS:
    const QString &getRadioDevice         () const { return m_radioDev; }
    const QString &getPlaybackMixerID     () const { return m_PlaybackMixerID; }
    const QString &getCaptureMixerID      () const { return m_CaptureMixerID; }
    const QString &getPlaybackMixerChannel() const { return m_PlaybackMixerChannel; }
    const QString &getCaptureMixerChannel () const { return m_CaptureMixerChannel; }
    float          getDeviceVolume        () const;
    V4LCaps        getCapabilities(QString dev = QString::null) const;

    bool           getActivePlayback()       const { return m_ActivePlayback; }
    bool           getMuteOnPowerOff()       const { return m_MuteOnPowerOff; }
    bool           getVolumeZeroOnPowerOff() const { return m_VolumeZeroOnPowerOff; }

    // anything else

protected slots:
    void  poll();

protected:
    V4LCaps readV4LCaps(const QString &device) const;
    void    radio_init();
    void    radio_done();

    bool    readTunerInfo() const;
    bool    updateAudioInfo(bool write) const;
    bool    readAudioInfo() const { return updateAudioInfo(false); }
    bool    writeAudioInfo() const { return updateAudioInfo(true); }

    void    searchMixers(ISoundStreamClient **playback_mixer, ISoundStreamClient **capture_mixer);

protected:

    FrequencyRadioStation  m_currentStation;
    mutable float          m_treble;
    mutable float          m_bass;
    mutable float          m_balance;
    mutable float          m_deviceVolume;
    mutable bool           m_muted;
    mutable float          m_signalQuality;
    mutable bool           m_stereo;

    float                  m_minQuality;
    float                  m_minFrequency;
    float                  m_maxFrequency;
    mutable float          m_lastMinDevFrequency;
    mutable float          m_lastMaxDevFrequency;

    float                  m_defaultPlaybackVolume;

    FrequencySeekHelper   *m_seekHelper;
    float                  m_scanStep;

    V4LCaps                m_caps;
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

    SoundStreamID                 m_SoundStreamID;
    QString                       m_PlaybackMixerID;
    QString                       m_CaptureMixerID;
    QString                       m_PlaybackMixerChannel;
    QString                       m_CaptureMixerChannel;

    bool                          m_ActivePlayback;
    bool                          m_MuteOnPowerOff;
    bool                          m_VolumeZeroOnPowerOff;

    bool                          m_restorePowerOn;
};

#endif
