/***************************************************************************
                          radio.h  -  description
                             -------------------
    begin                : Thu Mar 8 2001
    copyright            : (C) 2001, 2002 by Frank Schwanz, Ernst Martin Witte
                               2003 by Klas Kalass
    email                : schwanz@fh-brandenburg.de, witte@kawo1.rwth-aachen.de,
                           klas@kde.org
    based on             : libradio
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*
 *  libradio, a simple fm tuner control library
 *  Copyright (C) 1999, 2000  Dan Johnson <dj51d@progworks.net>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 *  MA 02111-1307, USA
 */

#ifndef V4LRADIO_H
#define V4LRADIO_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "utils.h"

#include "frequencyradio.h"

// forward declarations
class Radio;

class V4LRadio : public FrequencyRadio {

public:
//	V4LRadio(QObject *parent, const QString &name,
//	         const QString &RadioDev = "", const QString &MixerDev = "", int MixerChannel = 0);
    V4LRadio(Radio *mainRadio);
	~V4LRadio();

    virtual const QString &radioDevice()  const { return RadioDev; }
    virtual void  setRadioDevice(const QString &s);

    virtual const QString &mixerDevice()  const { return MixerDev; }
    virtual void  setMixerDevice(const QString &s, int ch);

    virtual void  setDevices(const QString &r, const QString &m, int ch);

    virtual int mixerChannel() const { return MixerChannel; }

    virtual float signal() const;
    virtual bool  isStereo() const;


    virtual	float deltaF () const;

    virtual	float currentFrequency() const;
    virtual void  setCurrentFrequency(float freq);

    virtual float volume() const;
    virtual void setVolume(float volume);

    virtual bool power() const;
    virtual void setPower(bool on);

    virtual bool muted() const;
    virtual void setMute(bool mute);

    // this function is only supposed to be called by a FrequencyRadioStation
    virtual bool activateStation(FrequencyRadioStation *station);

protected:
	void radio_init();
	void radio_done();

	bool readTunerInfo() const;
	bool readAudioInfo() const;
	bool writeAudioInfo();

    virtual float minPossibleFrequency() const;
    virtual float maxPossibleFrequency() const;
    virtual void setFrequencyImpl(float freq);

protected:
	struct TunerCache {
		bool  valid;
		float deltaF;
		float minF, maxF;

		TunerCache() { valid = false; }
	};


	QString RadioDev;
	QString MixerDev;
	int MixerChannel;
	int radio_fd;
	int mixer_fd;
	struct video_tuner *tuner;
	struct video_audio *audio;


	mutable TunerCache  tunercache;
//	__u16 balance;
//	__u16 bass ;
//	__u16 treble;

};

#endif
