/***************************************************************************
                          recordingcontext.cpp  -  description
                             -------------------
    begin                : Mi Aug 27 2003
    copyright            : (C) 2003 by Martin Witte
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

#include "recording-context.h"

#include <sys/soundcard.h>
#include <kconfig.h>

RecordingConfig::RecordingConfig ()
  : channels(2),
    bits(16),
    littleEndian(true),
    sign(true),
    rate(44100),
    device("/dev/dsp"),
    output("/dev/null")
{
}

RecordingConfig::RecordingConfig (const QString &dev,
                                  const QString &o,
                                  int c, int b, bool le, bool s, int r)
  : channels(c),
    bits(b),
    littleEndian(le),
    sign(s),
    rate(r),
    device(dev),
    output(o)
{
}


RecordingConfig::RecordingConfig (const QString &dev,
	                              const QString &o,
	                              int c, int ossFormat, int r)
  : channels(c),
    rate(r),
    device(dev),
    output(o)
{
	setOSSFormat(ossFormat);
}


RecordingConfig::RecordingConfig (const RecordingConfig &c)
  : channels(c.channels),
    bits(c.bits),
    littleEndian(c.littleEndian),
    sign(c.sign),
    rate(c.rate),
    device(c.device),
    output(c.output)
{
}


int RecordingConfig::getOSSFormat() const
{
	if (bits == 16) {
		switch (2*sign + littleEndian) {
			case 0: return AFMT_U16_BE;
			case 1: return AFMT_U16_LE;
			case 2: return AFMT_S16_BE;
			case 3: return AFMT_S16_LE;
		}
	}
	if (bits == 8) {
		switch (sign) {
			case 0: return AFMT_U8;
			case 1: return AFMT_S8;
		}
	}
	return 0;
}


void RecordingConfig::setOSSFormat(int f)
{
	int t = 7;
	switch (f) {
		case AFMT_U8:     t = 0; break;
		case AFMT_S8:     t = 2; break;
		case AFMT_U16_BE: t = 4; break;
		case AFMT_U16_LE: t = 5; break;
		case AFMT_S16_BE: t = 6; break;
		case AFMT_S16_LE: t = 7; break;
		default:          t = 7; break;
	}
	littleEndian = (t & 1) != 0;
	sign         = (t & 2) != 0;
	bits         = (t & 4) ? 16 : 8;
}


void  RecordingConfig::restoreConfig(KConfig *c)
{
	bits = c->readNumEntry("bits", 16);
	sign = c->readBoolEntry("sign", true);
	channels = c->readNumEntry("channels", 2);
	rate = c->readNumEntry("rate", 44100);
	littleEndian = c->readBoolEntry("littleEndian", true);
	device       = c->readEntry("device", "/dev/dsp");
	output       = c->readEntry("outputFile", "/dev/null");
}


void  RecordingConfig::saveConfig(KConfig *c) const
{
	c->writeEntry("bits",         bits);
	c->writeEntry("sign",         sign);
	c->writeEntry("channels",     channels);
	c->writeEntry("rate",         rate);
	c->writeEntry("littleEndian", littleEndian);
	c->writeEntry("device",       device);
	c->writeEntry("outputFile",   output);
}

	
///////////////////////////////////////////////////////////////////////

RecordingContext::RecordingContext()
 : valid(false),
   running(false),
   error(false),
   outputFile(QString::null),
   size_low(0),
   size_high(0),
   seconds(0),
   subSecondSamples(0)
{
}


RecordingContext::RecordingContext(const RecordingContext &c)
 : valid(c.valid),
   running(c.running),
   error(c.error),
   outputFile(c.outputFile),
   size_low(c.size_low),
   size_high(c.size_high),
   seconds(c.seconds),
   subSecondSamples(c.subSecondSamples)
{
}


void RecordingContext::start(const QString &o)
{
	running          = true;
	valid            = true;
	error            = false;
	outputFile       = o;
	size_low         = 0;
    size_high        = 0;
    seconds          = 0;
    subSecondSamples = 0;
}


void RecordingContext::stop()
{
	running          = false;
}


void RecordingContext::setError()
{
	error            = true;
	stop();
}


void RecordingContext::bufferAdded(unsigned int size, const RecordingConfig &c)
{
	if (running) {
		int sampleSize = 1;
		if (c.bits >  8) sampleSize = 2;
		if (c.bits > 16) sampleSize = 4;
		sampleSize *= c.channels;

		size_low += size;
		if (size_low < size) ++size_high;
		subSecondSamples += size / sampleSize;
		seconds += subSecondSamples / c.rate;
		subSecondSamples %= c.rate;
	}
}


