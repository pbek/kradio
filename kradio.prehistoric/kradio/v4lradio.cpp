/***************************************************************************
                          v4lradio.cpp  -  description
                             -------------------
    begin                : Don M�r  8 21:57:17 CET 2001
    copyright            : (C) 2002, 2003 by Ernst Martin Witte
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


#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/videodev.h>
#include <linux/soundcard.h>

#include "v4lradio.h"

/*

TODO:

  prevent loops (read...info -> notify -> readinfo) ??

*/

struct _lrvol { unsigned char l, r; short dummy; };

V4LRadio::V4LRadio()
  : m_volume(0),
    m_muted(true),
    m_signalQuality(0),
    m_stereo(false),
    
	m_minQuality(0.75),
	m_minFrequency(0),
	m_maxFrequency(0),

	m_seekHelper(*this),	
	m_scanStep(0.05),
	
	m_mixerChannel(0),	
	m_radio_fd(0),
	m_mixer_fd(0),
	m_pollTimer(this)
{
	QObject::connect (&m_pollTimer, SIGNAL(timeout()), this, SLOT(poll()));
	m_pollTimer.start(333);
}


V4LRadio::~V4LRadio()
{
	setPower(false);
}


bool V4LRadio::connect (Interface *i)
{
	return IRadioDevice::connect(i) || 
           IRadioSound::connect(i) || 
           ISeekRadio::connect(i) || 
           IFrequencyRadio::connect(i);
}


bool V4LRadio::disconnect (Interface *i)
{
	return IRadioDevice::disconnect(i) ||
           IRadioSound::disconnect(i) ||
           ISeekRadio::disconnect(i) ||
           IFrequencyRadio::disconnect(i);
}


// IRadioDevice methods

bool V4LRadio::setPower (bool on)
{
	return on ? powerOn() : powerOff();
}

bool V4LRadio::powerOn ()
{
	if (isPowerOn())
		return true;

    radio_init();
    unmute();
    notifyPowerChanged(true);

    return true;
}


bool V4LRadio::powerOff ()
{
	if (! isPowerOn())
		return true;

    mute();
    radio_done();
    notifyPowerChanged(false);

    return true;
}


bool V4LRadio::activateStation(const RadioStation &rs)
{
	const FrequencyRadioStation *frs = dynamic_cast<const FrequencyRadioStation*>(&rs);
	if (frs == NULL)
		return false;

	if (setFrequency(frs->frequency())) {
		m_currentStation = *frs;

		if (frs->initialVolume() > 0)
			setVolume(frs->initialVolume());
			
		return true;
	}

	return false;
}



bool V4LRadio::isPowerOn() const
{
	return m_radio_fd != 0;
}


bool V4LRadio::isPowerOff() const
{
	return m_radio_fd <= 0;
}


const RadioStation &V4LRadio::getCurrentStation() const
{
	return m_currentStation;
}



// IRadioSound methods

bool V4LRadio::setVolume (float vol)
{
	if (vol > 1.0) vol = 1.0;
	if (vol < 0) vol = 0.0;

	const int divs = 100;
	vol = round(vol * divs) / float(divs);

  	if (m_volume != vol) {
	    if(m_radio_fd != 0) {
    		_lrvol tmpvol;
			tmpvol.r = tmpvol.l = (unsigned int)(round(vol * divs));
			if (ioctl(m_mixer_fd, MIXER_WRITE(m_mixerChannel), &tmpvol) != 0) {
				kdDebug() << "V4LRadio::setVolume: " << i18n("error setting volume to") << " " << vol;
				return false;
			}
	    }
		m_volume = vol;
		notifyVolumeChanged(m_volume);
	}

	return true;
}


bool V4LRadio::mute (bool mute)
{
	// mute
  	if (!updateAudioInfo(false))
	   return false;

    if (m_muted != mute) {
		if (mute)		m_audio.flags |= VIDEO_AUDIO_MUTE;
		else			m_audio.flags &= ~VIDEO_AUDIO_MUTE;

		return updateAudioInfo(true);
	}
	return false;
}


bool V4LRadio::unmute (bool unmute)
{
	return mute(!unmute);
}


bool V4LRadio::setSignalMinQuality (float mq)
{
	m_minQuality = mq;
	return true;
}


float   V4LRadio::getVolume() const
{
	if (m_radio_fd != 0) {
		_lrvol tmpvol;
		if(ioctl(m_mixer_fd, MIXER_READ(m_mixerChannel), &tmpvol)) {
			kdDebug() << "V4LRadio::getVolume: "
			          << i18n("error reading volume")
			          << endl;
			tmpvol.l = tmpvol.r = 0;
		}
		float v = float(tmpvol.l) / 100.0;

		if (m_volume != v) {
			m_volume = v;
			notifyVolumeChanged(m_volume);
		}
	}
	return m_volume;
}


float   V4LRadio::getSignalMinQuality() const
{
	return m_minQuality;
}


float   V4LRadio::getSignalQuality() const
{
  	readTunerInfo();
    return m_signalQuality;
}


bool   V4LRadio::hasGoodQuality() const
{
	return getSignalQuality() >= m_minQuality;
}


bool    V4LRadio::isStereo() const
{
	updateAudioInfo(false);
	return m_stereo;
}


bool    V4LRadio::isMuted() const
{
  	updateAudioInfo(false);
    return m_muted;
}



// ISeekRadio

bool V4LRadio::startSeekUp()
{
	return startSeek(true);
}

bool V4LRadio::startSeekDown()
{
	return startSeek(false);
}

bool V4LRadio::startSeek(bool up)
{
	m_seekHelper.start(up ? SeekHelper::up : SeekHelper::down);
	return true;
}

bool V4LRadio::stopSeek()
{
	m_seekHelper.stop();
	return true;
}

bool V4LRadio::isSeekRunning() const
{
	return m_seekHelper.isRunning();
}


bool V4LRadio::isSeekUpRunning() const
{
	return m_seekHelper.isRunningUp();
}


bool V4LRadio::isSeekDownRunning() const
{
	return m_seekHelper.isRunningDown();
}



// IFrequencyRadio

bool V4LRadio::setFrequency(float freq)
{
	stopSeek();
	
	if (m_radio_fd > 0) {
		if (m_currentStation.frequency() == freq) {
			return true;
		}
		
  		bool oldMute = isMuted();
  		if (!oldMute) mute();

		float         df = deltaF();
  		unsigned long lfreq = (unsigned long) round(freq / df);

	  	if (freq > getMaxFrequency() || freq < getMinFrequency()) {
	  		kdDebug() << "V4LRadio::setFrequency: "
	                  << i18n("invalid frequency")
	                  << freq << endl;
	    	return false;
	    }

  		if (ioctl(m_radio_fd, VIDIOCSFREQ, &lfreq)) {
  			kdDebug() << "V4LRadio::setFrequency: "
                      << i18n("error setting frequency to")
                      << freq << endl;
            // unmute the old radio with the old radio station
            if (!oldMute) unmute();
            return false;
        }
        // unmute this radio device, because we now have the current
        // radio station

        m_currentStation.setFrequency(freq);
  		if (!oldMute) unmute();

        notifyFrequencyChanged(freq, &m_currentStation);
        notifyStationChanged(m_currentStation);
    
		return true;
	} else {
		return false;
	}	
}


bool V4LRadio::setMinFrequency (float minF)
{
	float oldm = getMinFrequency();
	m_minFrequency = minF;

	float newm = getMinFrequency();
	if (oldm != newm)
	    notifyMinMaxFrequencyChanged(newm, getMaxFrequency());
	    
	return true;
}


bool V4LRadio::setMaxFrequency (float maxF)
{
	float oldm = getMaxFrequency();
	m_maxFrequency = maxF;

	float newm = getMaxFrequency();
	if (oldm != newm)
	    notifyMinMaxFrequencyChanged(getMinFrequency(), newm);

	return true;
}


bool V4LRadio::setScanStep(float s)
{
	float old = m_scanStep;
	m_scanStep = s;

	if (old != s) notifyScanStepChanged(m_scanStep);
	return true;
}


float V4LRadio::getFrequency()     const
{
	return m_currentStation.frequency();
}


float V4LRadio::getMinFrequency()  const
{
	return m_minFrequency ? m_minFrequency : getMinDeviceFrequency();
}


float V4LRadio::getMaxFrequency()  const
{
	return m_maxFrequency ? m_maxFrequency : getMaxDeviceFrequency();
}


float V4LRadio::getMinDeviceFrequency() const
{
	if (!m_tunercache.valid)
		readTunerInfo();

	return m_tunercache.minF;
}


float V4LRadio::getMaxDeviceFrequency() const
{
	if (!m_tunercache.valid)
		readTunerInfo();

	return m_tunercache.maxF;
}


float V4LRadio::getScanStep()      const
{
	return m_scanStep;
}



////////////////////////////////////////
// anything else

void V4LRadio::radio_init()
{
	stopSeek();
	
	m_mixer_fd = open(m_mixerDev, O_RDONLY);
	if (m_mixer_fd <= 0) {
		radio_done();
		
		kdDebug() << "V4LRadio::radio_init: "
		          << i18n("cannot open mixer device")
		          << " " << m_mixerDev << endl;
		return;
	}
	
	m_radio_fd = open(m_radioDev, O_RDONLY);
  	if (m_radio_fd <= 0) {
		radio_done();

		kdDebug() << "V4LRadio::radio_init: "
		          << i18n("cannot open radio device") << " "
		          << m_radioDev << endl;
    	return;
	}

	readTunerInfo();
    updateAudioInfo(false);

  	// restore frequency
  	setFrequency(getFrequency());

  	// read volume level from mixer
  	setVolume (getVolume());
}


void V4LRadio::radio_done()
{
	stopSeek();
	
	if (m_radio_fd > 0) close (m_radio_fd);
	if (m_mixer_fd > 0) close (m_mixer_fd);

	m_radio_fd = m_mixer_fd = 0;
}


float V4LRadio::deltaF() const
{
	if (!m_tunercache.valid)
		readTunerInfo();

	return m_tunercache.deltaF;
}











/*
int V4LRadio::set_bass(__u16 lbass)
{
  bass = lbass;
  if(radio_fd == 0)
    return RADIO_DEV_NOT_OPEN;
//  audio->bass = bass;
//  audio->volume = 65535;
//  audio->treble = treble;
//	audio->balance = balance;
//  ioctl(radio_fd, VIDIOCSAUDIO, audio);
  return RADIO_OK;
}

int V4LRadio::set_treble(__u16 ltreble)
{
  treble = ltreble;
  if(radio_fd == 0)
    return RADIO_DEV_NOT_OPEN;
//  audio->treble = treble;
//  audio->volume = 65535;
//  audio->bass = bass;
//	audio->balance = balance;
//  ioctl(radio_fd, VIDIOCSAUDIO, audio);

  return RADIO_OK;
}


*/



void  V4LRadio::setRadioDevice(const QString &s)
{
	if (m_radioDev != s) {
		bool p = isPowerOn();
		powerOff();
		m_radioDev = s;
		setPower(p);
	}
}


void  V4LRadio::setMixerDevice(const QString &s, int ch)
{
	if (m_mixerDev != s || m_mixerChannel != ch) {
		bool p = isPowerOn();
		powerOff();
		m_mixerDev = s;
		m_mixerChannel = ch;
		setPower(p);
	}
}


void  V4LRadio::setDevices(const QString &r, const QString &m, int ch)
{
	if (m_radioDev != r || m_mixerDev != m || m_mixerChannel != ch) {
		bool p = isPowerOn();
		powerOff();
		m_radioDev = r;
		m_mixerDev = m;
		m_mixerChannel = ch;
		setPower(p);
	}
}


bool V4LRadio::readTunerInfo() const
{
	float oldminf = m_tunercache.minF;
	float oldmaxf = m_tunercache.maxF;

	if (!m_tunercache.valid) {
		m_tunercache.minF   = 87;
		m_tunercache.maxF   = 109;
		m_tunercache.deltaF = 1.0/16.0;
		m_tunercache.valid  = true;
	}

    if (m_radio_fd > 0) {
		if (ioctl(m_radio_fd, VIDIOCGTUNER, &m_tuner) == 0) {

			if ((m_tuner.flags & VIDEO_TUNER_LOW) != 0)
				m_tunercache.deltaF = 1.0 / 16000.0;

			m_tunercache.minF = float(m_tuner.rangelow)  * m_tunercache.deltaF;
			m_tunercache.maxF = float(m_tuner.rangehigh) * m_tunercache.deltaF;

			m_tunercache.valid = true;

		} else {
			kdDebug() << "V4LRadio::readTunerInfo: "
			          << i18n("cannot get tuner info")
			          << endl;
			return false;
		}
	} else {
		m_tuner.signal = 0;
	}

	if (oldminf != m_tunercache.minF || oldmaxf != m_tunercache.maxF)
		notifyDeviceMinMaxFrequencyChanged(m_tunercache.minF, m_tunercache.maxF);

	if (  ! m_minFrequency && (oldminf != m_tunercache.minF)
	   || ! m_maxFrequency && (oldmaxf != m_tunercache.maxF))
		notifyMinMaxFrequencyChanged(getMinFrequency(), getMaxFrequency());
		
	// extract information on current state
	float oldq = m_signalQuality;
	m_signalQuality = float(m_tuner.signal) / 32767.0;

	if (m_signalQuality != oldq)
		notifySignalQualityChanged(m_signalQuality);
	if ( (m_signalQuality >= m_minQuality) != (oldq >= m_minQuality))
		notifySignalQualityChanged(m_signalQuality > m_minQuality);		

	return true;
}


bool V4LRadio::updateAudioInfo(bool write) const
{
	if (m_radio_fd > 0) {
		if (ioctl(m_radio_fd, write ? VIDIOCSAUDIO : VIDIOCGAUDIO, &m_audio) != 0) {
			kdDebug() << "V4LRadio::updateAudioInfo: "
			          << i18n("error updating radio audio info") << " ("
			          << i18n(write ? "write" : "read") << ")"
			          << endl;
			return false;
		}
	} else {
		m_audio.mode  &= ~VIDEO_SOUND_STEREO;
		m_audio.flags &= ~VIDEO_AUDIO_MUTE;
	}

	bool oldStereo = m_stereo;
	bool oldMute   = m_muted;

	m_stereo = (m_audio.mode  & VIDEO_SOUND_STEREO) != 0;
	m_muted  = (m_audio.flags & VIDEO_AUDIO_MUTE) != 0;

	if (oldStereo != m_stereo)
		notifyStereoChanged(m_stereo);
	if (oldMute != m_muted)
		notifyMuted(m_muted);
	
	return (m_radio_fd > 0);
}




void V4LRadio::poll()
{
	readTunerInfo();
	updateAudioInfo(false);
	getVolume();
}


