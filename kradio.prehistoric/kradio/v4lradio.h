/***************************************************************************
                          radio.h  -  description
                             -------------------
    begin                : Thu Mar 8 2001
    copyright            : (C) 2001, 2002 by Frank Schwanz, Ernst Martin Witte
    email                : schwanz@fh-brandenburg.de, witte@kawo1.rwth-aachen.de
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

#include "radiobase.h"

class V4LRadio : public RadioBase {

protected:
	
	QString RadioDev;
	QString MixerDev;
	int MixerChannel;
	int radio_fd;
	int mixer_fd;
	struct video_tuner *tuner;
	struct video_audio *audio;
	
//	__u16 balance;
//	__u16 bass ;
//	__u16 treble;

public:
	V4LRadio(QObject *parent, const QString &name,
	         const QString &RadioDev = "", const QString &MixerDev = "", int MixerChannel = 0);
	~V4LRadio();

	virtual void  setRadioDevice(const QString &s);
	virtual void  setMixerDevice(const QString &s, int ch);
	virtual void  setDevices(const QString &r, const QString &m, int ch);

	virtual const QString &radioDevice()  const { return RadioDev; }
	virtual const QString &mixerDevice()  const { return MixerDev; }
	virtual const int      mixerChannel() const { return MixerChannel; }
	
	virtual float signal() const;				
	virtual bool  isStereo() const;

	virtual void  _setFrequency(float freq);
	virtual	float getFrequency() const;
	virtual	float deltaF () const;
	virtual float minFrequency () const;
	virtual float maxFrequency () const;

	virtual void  setVolume(float vol);			
	virtual float getVolume() const;

	virtual bool  isPowerOn () const;
	virtual bool  isMuted() const;

public slots:
	virtual void  PowerOn ();
	virtual void  PowerOff ();
	virtual void  unmute();
	virtual void  mute();

protected:
	void radio_init();
	void radio_done();
};

#endif
