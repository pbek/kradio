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


class V4LRadio : public QObject,
	             public PluginBase,
                 public IRadioDevice,
                 public IRadioSound,
                 public ISeekRadio,
                 public IFrequencyRadio
{
Q_OBJECT
public:
	V4LRadio (const QString &name);
	virtual ~V4LRadio ();

	virtual bool connect (Interface *);
	virtual bool disconnect (Interface *);

	virtual const QString &name() const { return PluginBase::name(); }
	virtual       QString &name()       { return PluginBase::name(); }

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


	// IRadioSound

RECEIVERS:
	virtual bool setVolume (float v);
	virtual bool mute (bool mute = true);
	virtual bool unmute (bool unmute = true);
	virtual bool setSignalMinQuality(float q);

ANSWERS:
	virtual float   getVolume() const;
	virtual float   getSignalQuality() const;
	virtual float   getSignalMinQuality() const;
	virtual bool    hasGoodQuality() const;
	virtual bool    isStereo() const;
	virtual bool    isMuted() const;


    // ISeekRadio
	
RECEIVERS:
	virtual bool startSeek (bool up);
	virtual bool startSeekUp();
	virtual bool startSeekDown();
	virtual bool stopSeek();

ANSWERS:
	virtual bool isSeekRunning() const;
	virtual bool isSeekUpRunning() const;
	virtual bool isSeekDownRunning() const;


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

	// PluginBase

public:
	virtual void   saveState (KConfig *) const;
	virtual void   restoreState (KConfig *);

	virtual bool   connect (PluginBase *p)    { return connect ((Interface*)p); }
	virtual bool   disconnect (PluginBase *p) { return disconnect ((Interface*)p); }

protected:
	virtual QFrame *internal_createConfigurationPage(KDialogBase *dlg);
	virtual QFrame *internal_createAboutPage(QWidget *parent);


    // anything else
public:

    const QString &getRadioDevice () const { return m_radioDev; }
    void           setRadioDevice (const QString &s);

    const QString &getMixerDevice () const { return m_mixerDev; }
    void           setMixerDevice (const QString &s, int ch);

    void  setDevices (const QString &r, const QString &m, int ch);
    int   getMixerChannel() const { return m_mixerChannel; }

protected slots:
	void  poll();
	
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
	mutable bool           m_muted;
	mutable float          m_signalQuality;
	mutable bool           m_stereo;
	
	float                  m_minQuality;
	float                  m_minFrequency;
	float                  m_maxFrequency;

	FrequencySeekHelper    m_seekHelper;
	float                  m_scanStep;

	QString                m_radioDev;
	QString                m_mixerDev;
	int                    m_mixerChannel;
	int                    m_radio_fd;
	int                    m_mixer_fd;
	
	mutable struct video_tuner   *m_tuner;
	mutable struct video_audio   *m_audio;

	QTimer                 m_pollTimer;

	mutable struct TunerCache {
		bool  valid;
		float deltaF;
		float minF, maxF;
		TunerCache() { valid = false; }
	}
	                       m_tunercache;


	mutable bool           m_blockReadTuner,
	                       m_blockReadAudio;
	
//	__u16 balance;
//	__u16 bass ;
//	__u16 treble;
};

#endif
