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


#include <sndfile.h>

#include <qobject.h>
#include <qstring.h>

#include "plugins.h"
#include "recording-interfaces.h"
#include "recording-context.h"
#include "timecontrol_interfaces.h"
#include "radio_interfaces.h"

class RadioStation;
class StationList;
class QSocketNotifier;

class Recording : public QObject,
                  public PluginBase,
                  public IRecording,
                  public ITimeControlClient,
                  public IRadioClient
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
    bool  setRecordingConfig(const RecordingConfig &);
    
ANSWERS:
	bool                    isRecording() const;
	const RecordingConfig  &getRecordingConfig() const;
	const RecordingContext &getRecordingContext() const;

// ITimeControlClient

	bool noticeAlarmsChanged(const AlarmVector &)         { return false; }
	bool noticeAlarm(const Alarm &);
	bool noticeNextAlarmChanged(const Alarm *)            { return false; }
	bool noticeCountdownStarted(const QDateTime &/*end*/) { return false; }
	bool noticeCountdownStopped()                         { return false; }
	bool noticeCountdownZero()                            { return false; }
	bool noticeCountdownSecondsChanged(int /*n*/)         { return false; }

// IRadioClient

	bool noticePowerChanged(bool on);
	bool noticeStationChanged (const RadioStation &, int /*idx*/){ return false; }
	bool noticeStationsChanged(const StationList &)              { return false; }



protected slots:

	void slotSoundDataAvailable();

protected:

	bool openDevice();
	void closeDevice();

	bool openOutput();
	void closeOutput();

protected:
	int               m_devfd;
	char             *m_buffer;
	int               m_bufferSize;
	SNDFILE          *m_output;

	RecordingConfig   m_config;
	RecordingContext  m_context;
	QSocketNotifier  *m_notifier;
};





#endif
