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


struct _lrvol { unsigned char l, r; short dummy; };

V4LRadio::V4LRadio (QObject *_parent, const QString &_name,
                    const QString &_RadioDev, const QString &_MixerDev, int _MixerChannel)
 : RadioBase (_parent, _name)
{
	RadioDev = _RadioDev;
	MixerDev = _MixerDev;
	MixerChannel = _MixerChannel;
	radio_fd = 0;
	mixer_fd = 0;
	tuner = 0;
	audio = 0;
}


V4LRadio::~V4LRadio()
{
	PowerOff();
}


void V4LRadio::radio_init()
{
	tunercache.valid = false;

	mixer_fd = open(MixerDev, O_RDONLY);
	if (mixer_fd == -1) {
		mixer_fd = 0;
		fprintf (stderr, "%s\n", (const char*)i18n("cannot open mixer"));
		return;
	}
	radio_fd = open(RadioDev, O_RDONLY);
  	if (radio_fd == -1) {
		fprintf (stderr, "%s\n", (const char*)i18n("cannot open radio device"));
		close (mixer_fd);
		radio_fd = mixer_fd = 0;
    	return;
	}
	tuner = new struct video_tuner;
	audio = new struct video_audio;
	if((audio == NULL) || (tuner == NULL)) {
		close(radio_fd);
		close(mixer_fd);
		radio_fd = mixer_fd = 0;
		tuner = 0; audio = 0;
		return;
	}

	tuner->tuner = 0;

	readTunerInfo();

  	// restore frequency
  	setFrequency(RadioBase::getFrequency());

  	// read volume level from mixer
  	setVolume (getVolume());

//	balance= 0x8000;
//	bass = 56000; /* can someone suggest a good default value */
//	treble = 56000; /* what is a good default value? */
}


void V4LRadio::radio_done()
{
	tunercache.valid = false;

	if (radio_fd == 0)
		return;
	close (radio_fd);
	close (mixer_fd);
	radio_fd = 0;
	mixer_fd = 0;
	if (tuner)
  		delete tuner;
	if (audio)
		delete audio;
}


bool V4LRadio::isPowerOn () const
{
	return radio_fd != 0;
}


void V4LRadio::PowerOn ()
{
	if (isPowerOn())
		return;
	radio_init();
	unmute();
	RadioBase::PowerOn();
}


void V4LRadio::PowerOff () {
	if (!isPowerOn())
		return;
	mute ();
	radio_done();
	RadioBase::PowerOff();
}


float V4LRadio::getFrequency() const
{
/*  	if (radio_fd != 0) {
		unsigned long fq;
		if (ioctl (radio_fd, VIDIOCGFREQ, &fq)) {
			fprintf (stderr, "V4LRadio::getFrequency: %s\n", (const char*)i18n("error reading frequency"));
			return 0;
		}

    	float f = float(fq) * deltaF();
	    if (f != RadioBase::getFrequency())
    		const_cast<V4LRadio*>(this)->RadioBase::_setFrequency(f);
    }
*/
    return RadioBase::getFrequency();
}


float V4LRadio::deltaF() const
{
	if (!tunercache.valid)
		readTunerInfo();

	return tunercache.deltaF;
}


float V4LRadio::minPossibleFrequency() const
{
	if (!tunercache.valid)
		readTunerInfo();

	return tunercache.minF;
}


float V4LRadio::maxPossibleFrequency() const
{
	if (!tunercache.valid)
		readTunerInfo();

	return tunercache.maxF;
}


void V4LRadio::_setFrequency(float freq)
{
  	if (radio_fd != 0) {
  		bool oldMute = isMuted();
  		if (!oldMute) mute();

		float         df = deltaF();
  		unsigned long lfreq = (unsigned long) round(freq / df);
//		float         r_freq = lfreq * df;

	  	if (freq > maxFrequency() || freq < minFrequency()) {
	  		fprintf (stderr, "V4LRadio::setFrequency: %s %.2g\n", (const char*)i18n("invalid frequency"), freq);
	    	return;
	    }

  		if (ioctl(radio_fd, VIDIOCSFREQ, &lfreq))
  			fprintf (stderr, "V4LRadio::setFrequency: %s %.2g\n", (const char*)i18n("error setting frequency to"), freq);

  		if (!oldMute) unmute();
	}

	RadioBase::_setFrequency(freq);
}


float V4LRadio::signal() const
{
  	if (radio_fd > 0 && readTunerInfo())
		return float(tuner->signal) / 32767.0;
    else
		return 0;
}


bool V4LRadio::isStereo() const
{
	if (radio_fd > 0 && readAudioInfo())
		return (audio->mode & VIDEO_SOUND_STEREO);
	else
		return false;
}


void V4LRadio::setVolume(float vol)
{
	if (vol > 1.0) vol = 1.0;
	if (vol < 0) vol = 0.0;

	const int divs = 100;
	vol = round(vol * divs) / float(divs);

  	if (radio_fd != 0) {

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
		if (ioctl(mixer_fd, MIXER_WRITE(MixerChannel), &tmpvol)) {
			fprintf (stderr, "V4LRadio::setVolume: %s %.2f\n", (const char*)i18n("error setting volume to"), vol);
			return;
		}
	}
	RadioBase::setVolume (vol);
}


float V4LRadio::getVolume() const
{
	if (radio_fd != 0) {
		_lrvol tmpvol;
		if(ioctl(mixer_fd, MIXER_READ(MixerChannel), &tmpvol)) {
			fprintf (stderr, "V4LRadio::getVolume: %s\n", (const char*)i18n("error reading volume"));
			tmpvol.l = tmpvol.r = 0;
		}
		float v = float(tmpvol.l) / 100.0;

		if (RadioBase::getVolume() != v)
			const_cast<V4LRadio*>(this)->RadioBase::setVolume(v);
	}
	return RadioBase::getVolume();
}


void V4LRadio::mute()
{
	if (radio_fd != 0) {
		// mute
	  	if (!readAudioInfo())
	  		return;
	  	audio->flags |= VIDEO_AUDIO_MUTE;

	  	if (!writeAudioInfo())
	  		return;
  	}
}


void V4LRadio::unmute()
{
	if(radio_fd != 0) {
	    // unmute
	  	if (!readAudioInfo())
	  		return;
		audio->flags &= ~VIDEO_AUDIO_MUTE;
	  	if (!writeAudioInfo())
	  		return;
	}
}


bool V4LRadio::isMuted() const
{
	if (radio_fd == 0)
		return true;
    // unmute
  	if (!readAudioInfo())
  		return true;
  	return (audio->flags &= VIDEO_AUDIO_MUTE) != 0;
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
	if (RadioDev != s) {
		bool p = isPowerOn();
		PowerOff();
		RadioDev = s;
		if (p) PowerOn();
	}
}

void  V4LRadio::setMixerDevice(const QString &s, int ch)
{
	if (MixerDev != s || MixerChannel != ch) {
		bool p = isPowerOn();
		PowerOff();
		MixerDev = s;
		MixerChannel = ch;
		if (p) PowerOn();
	}
}

void  V4LRadio::setDevices(const QString &r, const QString &m, int ch)
{
	if (RadioDev != r || MixerDev != m || MixerChannel != ch) {
		bool p = isPowerOn();
		PowerOff();
		RadioDev = r;
		MixerDev = m;
		MixerChannel = ch;
		if (p) PowerOn();
	}
}


bool V4LRadio::readTunerInfo() const
{
	tunercache.minF   = 87;
	tunercache.maxF   = 109;
	tunercache.deltaF = 1.0/16.0;
	tunercache.valid  = false;

    if (radio_fd > 0) {
		if (tuner && ioctl(radio_fd, VIDIOCGTUNER, tuner) == 0) {

			if ((tuner->flags & VIDEO_TUNER_LOW) != 0)
				tunercache.deltaF = 1.0 / 16000.0;

			tunercache.minF = float(tuner->rangelow)  * tunercache.deltaF;
			tunercache.maxF = float(tuner->rangehigh) * tunercache.deltaF;

			tunercache.valid = true;

			return true;
		} else {
			fprintf (stderr, "V4LRadio::readTunerInfo: %s\n", (const char*)i18n("cannot get tuner info"));

		}
	}
	return false;
}


bool V4LRadio::readAudioInfo() const
{
	if (radio_fd > 0) {
		if (audio && ioctl(radio_fd, VIDIOCGAUDIO, audio) == 0) {
			return true;
		} else {
			fprintf (stderr, "V4LRadio::readAudioInfo: %s\n", (const char*)i18n("error reading radio audio info"));
		}
	}
	return false;
}


bool V4LRadio::writeAudioInfo()
{
  	if (radio_fd > 0) {
		if (audio && ioctl(radio_fd, VIDIOCSAUDIO, audio) == 0) {
			return true;
		} else {
			fprintf (stderr, "V4LRadio::writeAudioInfo: %s\n", (const char*)i18n("error writing radio audio info"));
		}
    }
	return false;
}
