/***************************************************************************
                          radio.cpp  -  description
                             -------------------
    begin                : Don Mär  8 21:57:17 CET 2001
    copyright            : (C) 2001, 2002 by Frank Schwanz, Ernst Martin Witte
    email                : schwanz@fh-brandenburg.de, witte@kawo1.rwth-aachen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/videodev.h>
#include <linux/soundcard.h>
#include <stdio.h>
#include <string.h>

#include "v4lradio.h"
#include "radio.h"

struct _lrvol { unsigned char l, r; short dummy; };

V4LRadio::V4LRadio (Radio *mainRadio)
    : FrequencyRadio(mainRadio)
{
	m_radio_fd = 0;
	m_mixer_fd = 0;
	m_tuner = 0;
	m_audio = 0;
}


V4LRadio::~V4LRadio()
{
	setPower(false);
}


void V4LRadio::radio_init()
{
	m_tunercache.m_valid = false;

	m_mixer_fd = open(m_MixerDev, O_RDONLY);
	if (m_mixer_fd == -1) {
		m_mixer_fd = 0;
		fprintf (stderr, "%s\n", (const char*)i18n("cannot open mixer"));
		return;
	}
	m_radio_fd = open(m_RadioDev, O_RDONLY);
  	if (m_radio_fd == -1) {
		fprintf (stderr, "%s\n", (const char*)i18n("cannot open radio device"));
		close (m_mixer_fd);
		m_radio_fd = m_mixer_fd = 0;
    	return;
	}
	m_tuner = new struct video_tuner;
	m_audio = new struct video_audio;
	if((m_audio == NULL) || (m_tuner == NULL)) {
		close(m_radio_fd);
		close(m_mixer_fd);
		m_radio_fd = m_mixer_fd = 0;
		m_tuner = 0; m_audio = 0;
		return;
	}

	m_tuner->tuner = 0;

	readTunerInfo();

  	// restore frequency
  	setFrequency(frequency());

  	// read volume level from mixer
  	setVolume (volume());

//	balance= 0x8000;
//	bass = 56000; /* can someone suggest a good default value */
//	treble = 56000; /* what is a good default value? */
}


void V4LRadio::radio_done()
{
	m_tunercache.m_valid = false;

	if (m_radio_fd == 0)
		return;
	close (m_radio_fd);
	close (m_mixer_fd);
	m_radio_fd = 0;
	m_mixer_fd = 0;
	if (m_tuner)
  		delete m_tuner;
	if (m_audio)
		delete m_audio;
}


bool V4LRadio::power () const
{
	return m_radio_fd != 0;
}


void V4LRadio::setPower (bool on)
{
	if (power() == on)
		return;

        if (on) {
            radio_init();
            setMute(false);
        } else {
            setMute(true);
            radio_done();
        }
}

float V4LRadio::deltaF() const
{
	if (!m_tunercache.m_valid)
		readTunerInfo();

	return m_tunercache.m_deltaF;
}


float V4LRadio::minPossibleFrequency() const
{
	if (!m_tunercache.m_valid)
		readTunerInfo();

	return m_tunercache.m_minF;
}


float V4LRadio::maxPossibleFrequency() const
{
	if (!m_tunercache.m_valid)
		readTunerInfo();

	return m_tunercache.m_maxF;
}


bool V4LRadio::setFrequencyImpl(float freq)
{
  	if (m_radio_fd != 0) {
  		bool oldMute = mainRadio()->muted();
  		if (!oldMute) mainRadio()->slotMute();

		float         df = deltaF();
  		unsigned long lfreq = (unsigned long) round(freq / df);
//		float         r_freq = lfreq * df;

	  	if (freq > maxFrequency() || freq < minFrequency()) {
	  		fprintf (stderr, "V4LRadio::setFrequency: %s %.2g\n", (const char*)i18n("invalid frequency"), freq);
	    	return false;
	    }

  		if (ioctl(m_radio_fd, VIDIOCSFREQ, &lfreq)) {
  			fprintf (stderr, "V4LRadio::setFrequency: %s %.2g\n", (const char*)i18n("error setting frequency to"), freq);
                        // unmute the old radio with the old radio station
                        if (!oldMute) mainRadio()->slotUnmute();
                        return false;
                }
                // unmute this radio device, because we now have the current
                // radio station
  		if (!oldMute) setMute(false);
	}

	return FrequencyRadio::setFrequencyImpl(freq);
}


float V4LRadio::signal() const
{
  	if (m_radio_fd > 0 && readTunerInfo())
		return float(m_tuner->signal) / 32767.0;
    else
		return 0;
}


bool V4LRadio::isStereo() const
{
	if (m_radio_fd > 0 && readAudioInfo())
		return (m_audio->mode & VIDEO_SOUND_STEREO);
	else
		return false;
}


void V4LRadio::setVolume(float vol)
{
	if (vol > 1.0) vol = 1.0;
	if (vol < 0) vol = 0.0;

	const int divs = 100;
	vol = round(vol * divs) / float(divs);

  	if (m_radio_fd != 0) {

/*		audio->volume  = 0xFFFF;
		audio->bass    = 0xFFFF;
		audio->treble  = 0xFFFF;
		audio->balance = 0x8000;
  		audio->flags &= ~VIDEO_AUDIO_MUTE;
  		ioctl(radio_fd, VIDIOCGAUDIO, audio);
		fprintf (stderr, "%i\n", ioctl(radio_fd, VIDIOCSAUDIO, audio));
*/
		_lrvol tmpvol;
		tmpvol.r = tmpvol.l = (unsigned int)(round(vol * divs));
		if (ioctl(m_mixer_fd, MIXER_WRITE(m_MixerChannel), &tmpvol)) {
			fprintf (stderr, "V4LRadio::setVolume: %s %.2f\n", (const char*)i18n("error setting volume to"), vol);
			return;
		}
	}
	FrequencyRadio::setVolume (vol);
}


float V4LRadio::volume() const
{
	if (m_radio_fd != 0) {
		_lrvol tmpvol;
		if(ioctl(m_mixer_fd, MIXER_READ(m_MixerChannel), &tmpvol)) {
			fprintf (stderr, "V4LRadio::volume: %s\n", (const char*)i18n("error reading volume"));
			tmpvol.l = tmpvol.r = 0;
		}
		float v = float(tmpvol.l) / 100.0;

		if (FrequencyRadio::volume() != v)
			const_cast<V4LRadio*>(this)->FrequencyRadio::setVolume(v);
	}
	return FrequencyRadio::volume();
}


void V4LRadio::setMute(bool mute)
{
	if (m_radio_fd != 0) {
		// mute
	  	if (!readAudioInfo())
	  		return;
                if (mute)
                    m_audio->flags |= VIDEO_AUDIO_MUTE;
                else
                    m_audio->flags &= ~VIDEO_AUDIO_MUTE;

	  	if (!writeAudioInfo())
	  		return;
  	}
}

bool V4LRadio::muted() const
{
	if (m_radio_fd == 0)
		return true;

  	if (!readAudioInfo())
  		return true;
  	return (m_audio->flags &= VIDEO_AUDIO_MUTE) != 0;
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
	if (m_RadioDev != s) {
		bool p = power();
		setPower(false);
		m_RadioDev = s;
		setPower(p);
	}
}

void  V4LRadio::setMixerDevice(const QString &s, int ch)
{
	if (m_MixerDev != s || m_MixerChannel != ch) {
		bool p = power();
		setPower(false);
		m_MixerDev = s;
		m_MixerChannel = ch;
		setPower(p);
	}
}

void  V4LRadio::setDevices(const QString &r, const QString &m, int ch)
{
	if (m_RadioDev != r || m_MixerDev != m || m_MixerChannel != ch) {
		bool p = power();
		setPower(false);
		m_RadioDev = r;
		m_MixerDev = m;
		m_MixerChannel = ch;
		setPower(p);
	}
}


bool V4LRadio::readTunerInfo() const
{
	m_tunercache.m_minF   = 87;
	m_tunercache.m_maxF   = 109;
	m_tunercache.m_deltaF = 1.0/16.0;
	m_tunercache.m_valid  = false;

    if (m_radio_fd > 0) {
		if (m_tuner && ioctl(m_radio_fd, VIDIOCGTUNER, m_tuner) == 0) {

			if ((m_tuner->flags & VIDEO_TUNER_LOW) != 0)
				m_tunercache.m_deltaF = 1.0 / 16000.0;

			m_tunercache.m_minF = float(m_tuner->rangelow)  * m_tunercache.m_deltaF;
			m_tunercache.m_maxF = float(m_tuner->rangehigh) * m_tunercache.m_deltaF;

			m_tunercache.m_valid = true;

			return true;
		} else {
			fprintf (stderr, "V4LRadio::readTunerInfo: %s\n", i18n("cannot get tuner info").ascii());

		}
	}
	return false;
}


bool V4LRadio::readAudioInfo() const
{
	if (m_radio_fd > 0) {
		if (m_audio && ioctl(m_radio_fd, VIDIOCGAUDIO, m_audio) == 0) {
			return true;
		} else {
			fprintf (stderr, "V4LRadio::readAudioInfo: %s\n", i18n("error reading radio audio info").ascii());
		}
	}
	return false;
}


bool V4LRadio::writeAudioInfo()
{
  	if (m_radio_fd > 0) {
		if (m_audio && ioctl(m_radio_fd, VIDIOCSAUDIO, m_audio) == 0) {
			return true;
		} else {
			fprintf (stderr, "V4LRadio::writeAudioInfo: %s\n", i18n("error writing radio audio info").ascii());
		}
    }
	return false;
}
