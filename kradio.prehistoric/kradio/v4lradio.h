/***************************************************************************
                          v4lradio.h  -  description
                             -------------------
    begin                : Jan 2002
    copyright            : (C) 2002, 2003 Ernst Martin Witte, Klas Kalass
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

#include "radiodevice_interfaces.h"
#include "plugins.h"
#include "frequencyradiostation.h"
#include "frequencyseekhelper.h"
#include "v4lcfg_interfaces.h"


struct video_tuner;
struct video_audio;
struct v4l2_tuner;


class V4LRadio : public QObject,
	             public PluginBase,
                 public IRadioDevice,
                 public IRadioSound,
                 public ISeekRadio,
                 public IFrequencyRadio,
                 public IV4LCfg
{
Q_OBJECT
public:
	V4LRadio (const QString &name);
	virtual ~V4LRadio ();

	virtual bool connect (Interface *);
	virtual bool disconnect (Interface *);

	virtual const QString &name() const { return PluginBase::name(); }
	virtual       QString &name()       { return PluginBase::name(); }

	// PluginBase

public:
	virtual void   saveState (KConfig *) const;
	virtual void   restoreState (KConfig *);

	virtual ConfigPageInfo  createConfigurationPage();
	virtual QWidget        *createAboutPage();

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


	// IRadioSound

RECEIVERS:
	virtual bool setVolume  (float v);
	virtual bool setTreble  (float v);
	virtual bool setBass    (float v);
	virtual bool setBalance (float v);
	virtual bool mute (bool mute = true);
	virtual bool unmute (bool unmute = true);
	virtual bool setSignalMinQuality(float q);
	virtual bool setStereo(bool s);

ANSWERS:
	virtual float   getVolume() const;
	virtual float   getTreble() const;
	virtual float   getBass  () const;
	virtual float   getBalance () const;
	virtual float   getSignalQuality() const;
	virtual float   getSignalMinQuality() const;
	virtual bool    hasGoodQuality() const;
	virtual bool    isStereo() const;
	virtual bool    isMuted() const;


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


	// IV4LCfg
RECEIVERS:
    bool  setRadioDevice (const QString &s);
    bool  setMixerDevice (const QString &s, int ch);
    bool  setDevices (const QString &r, const QString &m, int ch);
	bool  setDeviceVolume(float v);

ANSWERS:
    const QString &getRadioDevice () const { return m_radioDev; }
    const QString &getMixerDevice () const { return m_mixerDev; }
    int            getMixerChannel() const { return m_mixerChannel; }
	float          getDeviceVolume() const;
	const V4LCaps &getCapabilities() const { return m_caps; }

    // anything else

protected slots:
	void  poll();
	


public:
	static V4LCaps readV4LCaps(const QString &device);

protected:
	void  radio_init();
	void  radio_done();

	bool  readTunerInfo() const;
	bool  updateAudioInfo(bool write) const;
	bool  readAudioInfo() const { return updateAudioInfo(false); }
	bool  writeAudioInfo() const { return updateAudioInfo(true); }

protected:

	FrequencyRadioStation  m_currentStation;
	mutable float          m_volume;
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

	FrequencySeekHelper    m_seekHelper;
	float                  m_scanStep;

	V4LCaps                m_caps;
	QString                m_radioDev;
	QString                m_mixerDev;
	int                    m_mixerChannel;
	int                    m_radio_fd;
	int                    m_mixer_fd;
	

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
		TunerCache() { valid = false; }
	};
	mutable struct TunerCache     m_tunercache;


	mutable bool                  m_blockReadTuner,
	                              m_blockReadAudio;
	
};

#endif
