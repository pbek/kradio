/***************************************************************************
                          recording.cpp  -  description
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

#include "recording.h"
#include "recording-context.h"
#include "recording-configuration.h"
#include "radiostation.h"

#include <qsocketnotifier.h>
#include <qevent.h>
#include <qapplication.h>
#include <qregexp.h>

#include <kconfig.h>

#include <sndfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>


///////////////////////////////////////////////////////////////////////

Recording::Recording(const QString &name)
	: QObject(NULL, QString::null),
	  PluginBase(name, "Recording Plugin"),
	  m_devfd(-1),
	  m_buffer(NULL),
	  m_bufferSize(0),
	  m_output(NULL),
	  m_config(),
	  m_context(),
	  m_notifier(NULL)
{
}


Recording::~Recording()
{
	stopRecording();
}


bool Recording::connect(Interface *i)
{
	bool a = IRecording::connect(i);
	bool b = ITimeControlClient::connect(i);
	bool c = IRadioClient::connect(i);
	return a || b || c;
}


bool Recording::disconnect(Interface *i)
{
	bool a = IRecording::disconnect(i);
	bool b = ITimeControlClient::disconnect(i);
	bool c = IRadioClient::disconnect(i);
	return a || b || c;
}

// PluginBase

void Recording::saveState (KConfig *c) const
{
    c->setGroup(QString("recording-") + PluginBase::name());
//    c->writeEntry ("monitoring", (m_context.state() == RecordingContext::rsMonitor));
	m_config.saveConfig(c);
}


void Recording::restoreState (KConfig *c)
{
    c->setGroup(QString("recording-") + PluginBase::name());
	m_config.restoreConfig(c);
//    if (c->readBoolEntry("monitoring", false) && m_context.state() != RecordingContext::rsRunning)
//		startMonitoring();  
}


ConfigPageInfo  Recording::createConfigurationPage()
{
	RecordingConfiguration *c = new RecordingConfiguration(NULL);
	connect(c);
    return ConfigPageInfo(c,
                          "Recording",
                          "Recording",
                          "kradio_record");
}


QWidget        *Recording::createAboutPage()
{
	// FIXME
	return NULL;
}


// IRecording

bool  Recording::startRecording()
{
	bool ok = true;
	if (m_context.state() == RecordingContext::rsRunning)
		return true;

	if (m_context.state() != RecordingContext::rsMonitor)
		ok &= openDevice();

	ok &= openOutput();
	kdDebug() << "recording starting" << endl;
	if (ok) {
		notifyRecordingStarted();
		notifyRecordingContextChanged(m_context);
		sendPowerOn();
		return true;
	} else {
		kdDebug() << "recording stopped with error" << endl;
		m_context.setError();
		stopRecording();
		return false;
	}
}


bool  Recording::startMonitoring()
{
	bool ok = true;
	if (m_context.state() == RecordingContext::rsRunning ||
	    m_context.state() == RecordingContext::rsMonitor) {
		    return true;
	}

	ok &= openDevice();
	kdDebug() << "monitoring starting" << endl;
	if (ok) {
		m_context.startMonitor(m_config);
		notifyMonitoringStarted();
		notifyRecordingContextChanged(m_context);
		return true;
	} else {
		kdDebug() << "monitoring stopped with error" << endl;
		m_context.setError();
		stopMonitoring();
		return false;
	}
}


bool Recording::stopRecording()
{
	RecordingContext::RecordingState oldState = m_context.oldState();
	kdDebug() << "oldstate: " << oldState << endl;
	switch (m_context.state()) {
		case RecordingContext::rsRunning:
		case RecordingContext::rsError:
			closeDevice();
			closeOutput();
			notifyRecordingContextChanged(m_context);
			notifyRecordingStopped();
			kdDebug() << "recording stopped" << endl;
			if (m_context.state() != RecordingContext::rsError &&
			    oldState == RecordingContext::rsMonitor) {
				     startMonitoring();
			}
			break;
		case RecordingContext::rsMonitor:
			stopMonitoring();
			break;
		default:
			break;
	}
	return true;
}


bool Recording::stopMonitoring()
{
	switch (m_context.state()) {
		case RecordingContext::rsMonitor:
		case RecordingContext::rsError:
			closeDevice();
			m_context.stop();
			notifyRecordingContextChanged(m_context);
			notifyMonitoringStopped();
			kdDebug() << "monitoring stopped" << endl;
			break;
		case RecordingContext::rsRunning:
			stopRecording();
			break;
		default:
			break;
	}
	return true;
}


bool Recording::setRecordingConfig(const RecordingConfig &c)
{
	m_config = c;

	notifyRecordingConfigChanged(m_config);
	
	if (m_context.state() == RecordingContext::rsMonitor) {
		stopMonitoring();
		startMonitoring();
	}	
	return true;
}


bool Recording::isRecording() const
{
	return m_context.state() == RecordingContext::rsRunning;
}


bool Recording::isMonitoring() const
{
	return m_context.state() == RecordingContext::rsMonitor;
}


const RecordingConfig  &Recording::getRecordingConfig() const
{
	return m_config;
}


const RecordingContext  &Recording::getRecordingContext() const
{
	return m_context;
}


// ITimeControlClient

bool Recording::noticeAlarm(const Alarm &a)
{
	if (a.alarmType() == Alarm::StartRecording && !isRecording()) {
		startRecording();
	} else if (a.alarmType() == Alarm::StopRecording && isRecording()) {
		stopRecording();
	}
	return true;
}


// IRadioClient

bool Recording::noticePowerChanged(bool on)
{
	if (isRecording() && !on) {
		stopRecording();
	}
	return true;
}


// ...


bool Recording::openDevice()
{
	// opening and seeting up the device file

	m_devfd = open(m_config.device, O_RDONLY);

    bool err = m_devfd < 0;

    int format = m_config.getOSSFormat();
    err |= (ioctl(m_devfd, SNDCTL_DSP_SETFMT, &format) != 0);
    kdDebug() << "err after SETFMT: " << err << endl;

    int channels = m_config.channels;
    err |= (ioctl(m_devfd, SNDCTL_DSP_CHANNELS, &channels) != 0);
    kdDebug() << "err after CHANNELS: " << err << endl;

    int rate = m_config.rate;
    err |= (ioctl(m_devfd, SNDCTL_DSP_SPEED, &rate) != 0);
    kdDebug() << "err after SPEED: " << err << endl;

    // setup buffer

    err |= ioctl (m_devfd, SNDCTL_DSP_GETBLKSIZE, &m_bufferSize);
    
    m_buffer = err ? NULL : new char[m_bufferSize];
    
    // setup Socket Notifier
    m_notifier = new QSocketNotifier(m_devfd, QSocketNotifier::Read, this);
    QObject::connect(m_notifier, SIGNAL(activated(int)),
                     this, SLOT(slotSoundDataAvailable()));

    // setup the input soundfile pointer

    if (err) closeDevice();

    return !err && m_buffer;
}


void Recording::closeDevice()
{
	if (m_notifier) delete m_notifier;
	m_notifier = NULL;

	if (m_devfd >= 0) close (m_devfd);
	m_devfd = -1;

	if (m_buffer) delete m_buffer;
	m_buffer = NULL;
	m_bufferSize = 0;
}


bool Recording::openOutput()
{
	QString ext = ".wav";
	switch (m_config.outputFormat) {
		case RecordingConfig::outputWAV:  ext = ".wav";  break;
		case RecordingConfig::outputAIFF: ext = ".aiff"; break;
		case RecordingConfig::outputAU:   ext = ".au";   break;
		case RecordingConfig::outputRAW:  ext = ".raw";  break;
		default:                          ext = ".wav";  break;
	}
	QString station = queryCurrentStation().name();
	station.replace(QRegExp("[/*?]"), "_");
	QString output = m_config.directory
	    + "/kradio-recording-"
	    + station + "-" +
	    + QDateTime::currentDateTime().toString(Qt::ISODate)
	    + ext;

	kdDebug() << "outputFile: " << output << endl;


	SF_INFO sinfo;
	m_config.getSoundFileInfo(sinfo, false);
    m_output = sf_open(output, SFM_WRITE, &sinfo);

	m_context.start(output, m_config);
	if (!m_output) m_context.setError();
	return m_output;
}


void Recording::closeOutput()
{
	if (m_output) sf_close (m_output);
	m_output = NULL;
	m_context.stop();
}


void Recording::slotSoundDataAvailable()
{
	if (m_context.state() == RecordingContext::rsRunning ||
	    m_context.state() == RecordingContext::rsMonitor) {
	
		int r = read(m_devfd, m_buffer, m_bufferSize);
		bool err = r <= 0;
		if (err)
			kdDebug() << "error: no data to record" << endl;

		if (!err && m_context.state() == RecordingContext::rsRunning) {
			err = sf_write_raw(m_output, m_buffer, r) != r;
			if (err)
				kdDebug() << "error writing output" << endl;
		}
			
		if (!err) {
			m_context.addInput(m_buffer, r);
			notifyRecordingContextChanged(m_context);
		} else {
			kdDebug() << "error while recording: " << errno << endl;
			m_context.setError();
			stopRecording();
		}
	}
}


