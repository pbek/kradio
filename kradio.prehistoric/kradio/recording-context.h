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

	int          channels;
	int          bits;
	bool         littleEndian;
	bool         sign;
	int          rate;
	QString      device;
	QString      directory;
	OutputFormat outputFormat;
};

 

struct RecordingContext
{
	bool          valid;
	bool          running;
	bool          error;
	QString       outputFile;
	size_t        size_low, size_high;
	unsigned int  seconds;
	unsigned int  subSecondSamples;

	RecordingContext();
	RecordingContext(const RecordingContext &c);

	void start()                      { start(outputFile); }
	void start(const QString &o);
	void stop();
	void setError();
	void bufferAdded(unsigned int samples, const RecordingConfig &c);
};


#endif

