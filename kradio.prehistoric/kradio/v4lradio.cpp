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
#define __USE_ISOC99 1
#include <math.h>
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
	mixer_fd = open(MixerDev, O_RDONLY);
	if (mixer_fd == -1) {
		mixer_fd = 0;
		fprintf (stderr, "cannot open mixer\n");
		return;
	}
	radio_fd = open(RadioDev, O_RDONLY);
  	if (radio_fd == -1) {
		fprintf (stderr, "cannot open radio device\n");
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
  	if (radio_fd != 0) {
		unsigned long fq;
		if (ioctl (radio_fd, VIDIOCGFREQ, &fq)) {
			fprintf (stderr, "V4LRadio::getFrequency: error reading frequency\n");
			return 0;
		}
		if (ioctl (radio_fd, VIDIOCGTUNER, tuner)) {
	  		fprintf (stderr, "V4LRadio::getFrequency: cannot get tuner info\n");
	  		return 0;
	  	}
	
		float div = 16;
		if ((tuner->flags & VIDEO_TUNER_LOW) != 0)
	    	div = 16000;
	    	
    	float f = float(fq) / div;
	    if (f != RadioBase::getFrequency())
    		const_cast<V4LRadio*>(this)->RadioBase::_setFrequency(f);
    }

    return RadioBase::getFrequency();
}


float V4LRadio::deltaF() const
{
	float div = 16;
  	if (radio_fd != 0) {  	
	  	if (ioctl(radio_fd, VIDIOCGTUNER, tuner) != 0)
	  		fprintf (stderr, "V4LRadio::deltaF: cannot get tuner info\n");
	  	else if ((tuner->flags & VIDEO_TUNER_LOW) != 0)
	    	div = 16000;
	}
	return 1 / div;
}


float V4LRadio::minFrequency() const
{
	if (rangeOverride)
		return fMinOverride;
  	if (radio_fd != 0) {  	
	  	if (ioctl(radio_fd, VIDIOCGTUNER, tuner) != 0)
	  		fprintf (stderr, "V4LRadio::deltaF: cannot get tuner info\n");
	  	else
	    	return float(tuner->rangelow) * deltaF();
	}
	return 87.0;
}


float V4LRadio::maxFrequency() const
{
	if (rangeOverride)
		return fMaxOverride;
  	if (radio_fd != 0) {  	
	  	if (ioctl(radio_fd, VIDIOCGTUNER, tuner) != 0)
	  		fprintf (stderr, "V4LRadio::deltaF: cannot get tuner info\n");
	  	else
	    	return float(tuner->rangehigh) * deltaF();
	}
	return 108.0;
}


		


void V4LRadio::_setFrequency(float freq)
{
  	if (radio_fd != 0) {
  		bool oldMute = isMuted();
  		if (!oldMute) mute();
  		
	  	if (ioctl(radio_fd, VIDIOCGTUNER, tuner) != 0) {
	  		fprintf (stderr, "V4LRadio::setFrequency: cannot get tuner info\n");
			return;
		}
		
		float div = 16;
	  	if ((tuner->flags & VIDEO_TUNER_LOW) != 0)
	    	div = 16000;
	    	
	    freq = round(freq * div) / div;
    	
  		unsigned long lfreq = (unsigned long) round(freq * div);
  	
	  	if ((lfreq > tuner->rangehigh) || (lfreq < tuner->rangelow)) {
	  		fprintf (stderr, "V4LRadio::setFrequency: invalid frequency %.2g\n", freq);
	    	return;
	    }
    	
  		if (ioctl(radio_fd, VIDIOCSFREQ, &lfreq))
  			fprintf (stderr, "V4LRadio::setFrequency: error setting frequency to %.2g\n", freq);
  		
  		if (!oldMute) unmute();
	}
	
	RadioBase::_setFrequency(freq);
}


float V4LRadio::signal() const
{
  	if (radio_fd == 0)
    	return 0;
    	
  	if (ioctl(radio_fd, VIDIOCGTUNER, tuner)) {
		fprintf (stderr, "V4LRadio::signal: error reading signal quality\n");
		return 0;
  	}
  	return float(tuner->signal) / 32767.0;
}


bool V4LRadio::isStereo() const
{
	if (radio_fd == 0)
    	return false;
    	
	if (ioctl(radio_fd, VIDIOCGAUDIO, audio)) {
		fprintf (stderr, "V4LRadio::isStereo: error reading stereo attributes\n");
		return false;
	}
	return (audio->mode & VIDEO_SOUND_STEREO);
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
			fprintf (stderr, "V4LRadio::setVolume: error setting volume to %.2f\n", vol);
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
			fprintf (stderr, "V4LRadio::getVolume: error reading volume\n");
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
	  	if (ioctl(radio_fd, VIDIOCGAUDIO, audio)) {
	  		fprintf (stderr, "V4LRadio::mute: error reading v4l audio info\n");
	  		return;
	  	}
	  	audio->flags |= VIDEO_AUDIO_MUTE;
	  	if (ioctl(radio_fd, VIDIOCSAUDIO, audio)) {
	  		fprintf (stderr, "V4LRadio::mute: error muting v4l audio\n");
	  		return;
	  	}
  	}
}


void V4LRadio::unmute()
{
	if(radio_fd != 0) {
	    // unmute
		if (ioctl(radio_fd, VIDIOCGAUDIO, audio)) {
	  		fprintf (stderr, "V4LRadio::unmute: error reading v4l audio info\n");
	  		return;
	  	}
		audio->flags &= ~VIDEO_AUDIO_MUTE;
		if (ioctl(radio_fd, VIDIOCSAUDIO, audio)) {
	  		fprintf (stderr, "V4LRadio::unmute: error unmuting v4l audio\n");
	  		return;
	  	}
	}
}


bool V4LRadio::isMuted() const
{
	if (radio_fd == 0)
		return true;
    // unmute
	if (ioctl(radio_fd, VIDIOCGAUDIO, audio)) {
  		fprintf (stderr, "V4LRadio::isMuted: error reading v4l audio info\n");
  		return true;
  	}
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
