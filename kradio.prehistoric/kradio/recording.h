/***************************************************************************
                          recording.h  -  description
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

#ifndef KRADIO_RECORDING_H
#define KRADIO_RECORDING_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <qobject.h>
#include <qstring.h>

#include "plugins.h"
#include "recording-interfaces.h"
#include "recording-context.h"

class RecordingThread;

class Recording : public QObject,
                  public PluginBase,
                  public IRecording
{
Q_OBJECT
public:
    Recording(const QString &name);
    ~Recording();

	virtual bool   connect(Interface *i);
	virtual bool   disconnect(Interface *i);

	// PluginBase

public:
	virtual void   saveState (KConfig *) const;
	virtual void   restoreState (KConfig *);

	virtual ConfigPageInfo  createConfigurationPage();
	virtual QWidget        *createAboutPage();


    // IRecording
RECEIVERS:
   	bool  startRecording();
	bool  stopRecording();
    bool  setRecordingDirectory(const QString &);
    
ANSWERS:
	bool            isRecording() const;
	const QString  &getRecordingDirectory() const;


protected:

	void customEvent(QCustomEvent *e);	

protected:
	QString          m_recordingDirectory;
	RecordingThread *m_recordingThread;
	RecordingConfig  m_config;
};





#endif
