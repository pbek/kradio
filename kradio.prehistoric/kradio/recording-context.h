/***************************************************************************
                          recording-context.h  -  description
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


#ifndef KRADIO_RECORDING_CONTEXT_H
#define KRADIO_RECORDING_CONTEXT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <qstring.h>
#include <unistd.h>

class KConfig;
struct SF_INFO;

struct RecordingConfig
{
public:
	enum OutputFormat { outputWAV, outputAIFF, outputAU, outputRAW };

public:
	RecordingConfig ();
	RecordingConfig (const QString &dev,
	                 const QString &directory,
	                 OutputFormat of,
	                 int c, int b, bool le, bool s, int r);
	RecordingConfig (const QString &dev,
	                 const QString &directory,
	                 OutputFormat of,
	                 int c, int ossFormat, int r);
	RecordingConfig (const RecordingConfig &c);

	int      getOSSFormat();
	void     setOSSFormat(int);

	void     restoreConfig(KConfig *c);
	void     saveConfig(KConfig *c) const;

	void     getSoundFileInfo(SF_INFO &info, bool input);

	void     checkFormatSettings();
	
	int      sampleSize() const;      // size of a single sample
	int      frameSize() const;       // sampleSize * channels
	int      minValue() const;
	int      maxValue() const;

public:
	int          channels;
	int          bits;
	bool         littleEndian;
	bool         sign;
	int          rate;
	QString      device;
	QString      directory;
	OutputFormat outputFormat;
};

 

class RecordingContext
{
public:
	enum RecordingState { rsInvalid, rsRunning, rsFinished, rsError, rsMonitor };
	
protected:
	RecordingState   m_state;
	RecordingConfig  m_config;

	int             *m_buffer;
	int              m_bufValidElements; // # of valid Elements in Buffer
	int              m_bufAvailElements; // real BufferSize in ints
	
	QString          m_outputFile;
	size_t           m_size_low,
	                 m_size_high;

public:
	RecordingContext();
	RecordingContext(const RecordingContext &c);
	~RecordingContext();

	void                    startMonitor(const RecordingConfig &c);
	void                    start(const QString &o, const RecordingConfig &c);
	void                    stop();
	void                    setError();
	
    RecordingState          state() const           { return m_state;  }
    const RecordingConfig & config() const          { return m_config; }
    const int             * buffer() const          { return m_buffer; }
    const int               samplesInBuffer() const { return m_bufValidElements; }
    const int               framesInBuffer() const  { return m_bufValidElements / m_config.channels; }

    const QString         & outputFile() const      { return m_outputFile; }
    double                  outputSize() const;
    double                  outputTime() const;
	
	void addInput(char *rawBuffer, unsigned int rawSize);
	
protected:
	void resizeBuffer(int elements);
};


#endif

