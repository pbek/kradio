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

struct RecordingConfig
{
	RecordingConfig ();
	RecordingConfig (const QString &dev,
	                 const QString &out,
	                 int c, int b, bool le, bool s, int r);
	RecordingConfig (const QString &dev,
	                 const QString &out,
	                 int c, int ossFormat, int r);
	RecordingConfig (const RecordingConfig &c);

	int      getOSSFormat() const;
	void     setOSSFormat(int);

	void     restoreConfig(KConfig *c);
	void     saveConfig(KConfig *c) const;

	int      channels;
	int      bits;
	bool     littleEndian;
	bool     sign;
	int      rate;
	QString  device;
	QString  output;
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

	void start(const QString &o);
	void stop();
	void setError();
	void bufferAdded(unsigned int size, const RecordingConfig &c);
};


#endif

