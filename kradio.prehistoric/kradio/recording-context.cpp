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
#include <kdebug.h>

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


int RecordingConfig::sampleSize() const
{
	if (bits <= 8)  return 1;
	if (bits <= 16) return 2;
	if (bits <= 32) return 4;

	// unknown
	return -1;
}


int RecordingConfig::frameSize() const
{
	return sampleSize() * channels;
}


int RecordingConfig::minValue() const
{
    if (!sign) return 0;
	return -(1 << (bits-1));
}


int RecordingConfig::maxValue() const
{
	return (1 << (bits-sign)) - 1;
}


///////////////////////////////////////////////////////////////////////


RecordingContext::RecordingContext()
 : m_state(rsInvalid),
   m_oldState(rsInvalid),
   m_config(),
   m_buffer(NULL),
   m_bufValidElements(0),
   m_bufAvailElements(0),
   m_outputFile(QString::null),
   m_size_low(0),
   m_size_high(0)
{
}


RecordingContext::RecordingContext(const RecordingContext &c)
 : m_state (c.m_state),
   m_oldState (c.m_oldState),
   m_config(c.m_config),
   m_buffer(NULL),
   m_bufValidElements(0),
   m_bufAvailElements(0),
   m_outputFile(c.m_outputFile),
   m_size_low  (c.m_size_low),
   m_size_high (c.m_size_high)
{
	resizeBuffer(c.m_bufAvailElements);
	if (m_buffer && c.m_buffer) {
		m_bufAvailElements = c.m_bufAvailElements;
		for (int i = 0; i < m_bufAvailElements; ++i)
			m_buffer[i] = c.m_buffer[i];
	}
}


RecordingContext::~RecordingContext()
{
	resizeBuffer(0);
}


void RecordingContext::startMonitor(const RecordingConfig &c)
{
	if (m_state != rsMonitor) {
		m_oldState         = m_state;
		m_state            = rsMonitor;
		m_config           = c;

		m_outputFile       = QString::null;
		m_size_low         = 0;
		m_size_high        = 0;
	}
}


void RecordingContext::start(const QString &o, const RecordingConfig &c)
{
	if (m_state != rsRunning) {
		m_oldState         = m_state;
		m_state            = rsRunning;
		m_config           = c;
		m_outputFile       = o;
		m_size_low         = 0;
		m_size_high        = 0;
	}
}


void RecordingContext::stop()
{
	switch (m_state) {
		case rsMonitor:
			m_oldState = m_state;
			m_state    = rsInvalid;
			break;
		case rsRunning:
			m_oldState = m_state;
			m_state    = rsFinished;
			break;
		default:  break; // do not change state
	}
	resizeBuffer (0);
}


void RecordingContext::setError()
{
	m_oldState = m_state;
	m_state    = rsError;
	resizeBuffer (0);
}


void RecordingContext::addInput(char *raw, unsigned int rawSize)
{
	if (m_state == rsRunning || m_state == rsMonitor) {
		if (m_state == rsRunning) {
			m_size_low += rawSize;
			if (m_size_low < rawSize) ++m_size_high;
		}

		int sampleSize = m_config.sampleSize();
		int samples    = rawSize / sampleSize;

		resizeBuffer(samples);

		if (samples > m_bufAvailElements) samples = m_bufAvailElements;

		// convert raw input into   int*
		int delta = m_config.littleEndian ? -1 : 1;
		int start = m_config.littleEndian ? sampleSize-1 : 0,
		    end   = m_config.littleEndian ? -1 : sampleSize;

        for (int i = 0; i < samples; ++i) {
			int k   = start;
			int val = m_config.sign ? (int)(signed char)raw[k] : (int)(unsigned char)raw[k];
			for (k += delta; k != end; k += delta) {
				val <<= 8;
				val |= (unsigned char)raw[k];
			}
			m_buffer[i] = val;
			raw += sampleSize;
        }

        m_bufValidElements = samples;

	}
}


void RecordingContext::resizeBuffer(int nElements)
{
	if (m_bufAvailElements < nElements) {
		if (m_buffer) delete m_buffer;
		m_buffer = new int[nElements];
		m_bufAvailElements = m_buffer ? nElements : 0;
		m_bufValidElements = 0;
	} else if (nElements == 0) {
		if (m_buffer) delete m_buffer;
		m_buffer = NULL;
		m_bufValidElements = 0;
		m_bufAvailElements = 0;
	}
}


double RecordingContext::outputSize() const
{
	return m_size_low + 65536.0 * 65536.0 * (double)m_size_high;
}


double RecordingContext::outputTime() const
{
	return outputSize() / m_config.frameSize() / m_config.rate;
}


