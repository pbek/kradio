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

#include <sndfile.h>
#include <sys/soundcard.h>
#include <kconfig.h>

RecordingConfig::RecordingConfig ()
  : channels(2),
    bits(16),
    littleEndian(true),
    sign(true),
    rate(44100),
    device("/dev/dsp"),
    directory("/tmp"),
    outputFormat(outputWAV)
{
	checkFormatSettings();
}

RecordingConfig::RecordingConfig (const QString &dev,
                                  const QString &directory,
                                  OutputFormat of,
                                  int c, int b, bool le, bool s, int r)
  : channels(c),
    bits(b),
    littleEndian(le),
    sign(s),
    rate(r),
    device(dev),
    directory(directory),
    outputFormat(of)
{
	checkFormatSettings();
}


RecordingConfig::RecordingConfig (const QString &dev,
                                  const QString &directory,
                                  OutputFormat of,
	                              int c, int ossFormat, int r)
  : channels(c),
    rate(r),
    device(dev),
    directory(directory),
    outputFormat(of)
{
	setOSSFormat(ossFormat);
	checkFormatSettings();
}


RecordingConfig::RecordingConfig (const RecordingConfig &c)
  : channels(c.channels),
    bits(c.bits),
    littleEndian(c.littleEndian),
    sign(c.sign),
    rate(c.rate),
    device(c.device),
    directory(c.directory),
    outputFormat(c.outputFormat)
{
	checkFormatSettings();
}


int RecordingConfig::getOSSFormat()
{
	checkFormatSettings();
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

	checkFormatSettings();
}


void  RecordingConfig::restoreConfig(KConfig *c)
{
	bits = c->readNumEntry("bits", 16);
	sign = c->readBoolEntry("sign", true);
	channels = c->readNumEntry("channels", 2);
	rate = c->readNumEntry("rate", 44100);
	littleEndian = c->readBoolEntry("littleEndian", true);
	device       = c->readEntry("device", "/dev/dsp");
	directory    = c->readEntry("directory", "/tmp");

	QString of = c->readEntry("outputFileFormat", ".wav");
	if (of == ".wav")
		outputFormat = outputWAV;
	else if (of == ".aiff")
		outputFormat = outputAIFF;
	else if (of == ".au")
		outputFormat = outputAU;
	else if (of == ".raw")
		outputFormat = outputRAW;
	else
		outputFormat = outputWAV;

	checkFormatSettings();
}


void  RecordingConfig::saveConfig(KConfig *c) const
{
	c->writeEntry("bits",         bits);
	c->writeEntry("sign",         sign);
	c->writeEntry("channels",     channels);
	c->writeEntry("rate",         rate);
	c->writeEntry("littleEndian", littleEndian);
	c->writeEntry("device",       device);
	c->writeEntry("directory",    directory);

	switch(outputFormat) {
		case outputWAV:  c->writeEntry("outputFormat", ".wav");
		case outputAIFF: c->writeEntry("outputFormat", ".aiff");
		case outputAU:   c->writeEntry("outputFormat", ".au");
		case outputRAW:  c->writeEntry("outputFormat", ".raw");
		default:         c->writeEntry("outputFormat", ".wav");
	}
}


void RecordingConfig::getSoundFileInfo(SF_INFO &sinfo, bool input)
{
	checkFormatSettings();

	sinfo.samplerate = rate;
	sinfo.channels   = channels;
	sinfo.format     = 0;
	sinfo.seekable   = !input;

	// U8 only supported for RAW and WAV
	if (bits == 8) {
		sinfo.format |= SF_FORMAT_PCM_S8;
		if (!sign && (outputFormat == outputRAW || outputFormat != outputWAV))
			sinfo.format |= SF_FORMAT_PCM_U8;
	}
	if (bits == 16)
		sinfo.format |= SF_FORMAT_PCM_16;

	if (littleEndian)
		sinfo.format |= SF_ENDIAN_LITTLE;
	else
		sinfo.format |= SF_ENDIAN_BIG;

	if (input) {
		sinfo.format |= SF_FORMAT_RAW;
	} else {
		switch (outputFormat) {
			case outputWAV:  sinfo.format |= SF_FORMAT_WAV;  break;
			case outputAIFF: sinfo.format |= SF_FORMAT_AIFF; break;
			case outputAU:   sinfo.format |= SF_FORMAT_AU;   break;
			case outputRAW:  sinfo.format |= SF_FORMAT_RAW;  break;
			default:         sinfo.format |= SF_FORMAT_WAV; break;
		}
	}
}

void RecordingConfig::checkFormatSettings()
{
	// libsndfile only supports signed 16 bit samples
	if (bits == 16)
		sign = true;

	// correct Endianess and Signs for specific formats
	switch (outputFormat) {
		case outputWAV:  littleEndian = true; break;
		case outputAIFF: littleEndian = false; sign = true; break;
		case outputAU:   littleEndian = false; sign = true; break;
		case outputRAW:  break;
		default:         break;
	}
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


void RecordingContext::bufferAdded(unsigned int sizeDelta, const RecordingConfig &c)
{
	if (running) {
		int sampleSize = 1;
		if (c.bits >  8) sampleSize = 2;
		if (c.bits > 16) sampleSize = 4;
		sampleSize *= c.channels;

		size_low += sizeDelta;
		if (size_low < sizeDelta) ++size_high;

		unsigned int samples = sizeDelta / sizeDelta;
		subSecondSamples += samples;
		seconds += subSecondSamples / c.rate;
		subSecondSamples %= c.rate;
	}
}


