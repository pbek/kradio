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

#include <qthread.h>
#include <qevent.h>
#include <qapplication.h>

#include <kconfig.h>

#include <sndfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include <fcntl.h>


#define  START_EVENT    ((QEvent::Type)(QEvent::User+1))
#define  PROCESS_EVENT  ((QEvent::Type)(QEvent::User+2))
#define  STOP_EVENT     ((QEvent::Type)(QEvent::User+3))


#define BUFFER_SIZE_SAMPLES 4096

class RecordingThread : public QThread
{
public:
	RecordingThread(QObject *parent);
	~RecordingThread();
	void run();

	void start(const QString &outputFile);
	void stop(bool sync);

	void setConfig(const RecordingConfig &c);
	
protected:

	void notifyParent(QEvent::Type MessageID);

	bool openDevice();
	void closeDevice();

	bool openOutput();
	void closeOutput();

	bool internalStart();
	void internalStop();
	
protected:
	int              m_devfd;
	int             *m_buffer;
	SNDFILE         *m_output;

	QObject         *m_parent;
	bool             m_stopFlag;
	RecordingContext m_context;
	RecordingConfig  m_config;
};

///////////////////////////////////////////////////////////////////////

RecordingThread::RecordingThread(QObject *parent)
	: m_devfd(-1),
	  m_buffer(NULL),
	  m_output(NULL),
      m_parent(parent),
	  m_stopFlag(false),
	  m_context(),
	  m_config()
{
}

RecordingThread::~RecordingThread()
{
	stop(true);
}


void RecordingThread::start(const QString &outputFile)
{
	m_context.start(outputFile);
	QThread::start();
}


void RecordingThread::stop(bool sync)
{
	m_stopFlag = true;
	if (sync) wait();
}


bool RecordingThread::openDevice()
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

    if (err) closeDevice();

    // setup the input soundfile pointer

	SF_INFO sinfo;
	m_config.getSoundFileInfo(sinfo, true);

    // setup buffer
        
    m_buffer = new int[BUFFER_SIZE_SAMPLES * m_config.channels];

    return !err && m_buffer;
}


void RecordingThread::closeDevice()
{
	if (m_devfd >= 0) close (m_devfd);
	m_devfd = -1;
	if (m_buffer) delete m_buffer;
}

bool RecordingThread::openOutput()
{
	SF_INFO sinfo;
	m_config.getSoundFileInfo(sinfo, false);
    m_output = sf_open(m_context.outputFile, SFM_WRITE, &sinfo);
    
	m_context.start();
	return m_output;
}

void RecordingThread::closeOutput()
{
	sf_close (m_output);
	m_output = NULL;
	m_context.stop();
}


bool RecordingThread::internalStart()
{
	if (openDevice() && openOutput()) {
		notifyParent(START_EVENT);
		m_stopFlag = false;
		return true;
	} else {
		return false;
	}
}


void RecordingThread::internalStop()
{
	closeDevice();
	closeOutput();
	notifyParent(STOP_EVENT);
}


void RecordingThread::run()
{
	kdDebug() << "recording started" << endl;
	if (!internalStart()) {
		internalStop();
		return;
	}

    int bufferSize = BUFFER_SIZE_SAMPLES;
    bufferSize *= m_config.channels;
    if (m_config.bits > 16) bufferSize *= 4;
    else if (m_config.bits > 8) bufferSize *= 2;
	
	while (!m_stopFlag) {

		int r = read(m_devfd, m_buffer, bufferSize);
		
		if (r > 0 && sf_write_raw(m_output, m_buffer, r) == r) {
			m_context.bufferAdded(r, m_config);
			notifyParent(PROCESS_EVENT);
		} else {
			kdDebug() << "error while recording: " << errno << endl;
			m_context.setError();
			m_stopFlag = true;
		}
	}
	internalStop();
	kdDebug() << "recording stopped" << endl;
}


void RecordingThread::notifyParent(QEvent::Type MessageID)
{
	QApplication::postEvent(m_parent,
       new QCustomEvent(MessageID, new RecordingContext(m_context)));
}


void RecordingThread::setConfig(const RecordingConfig &c)
{
	m_config = c;
}


///////////////////////////////////////////////////////////////////////

Recording::Recording(const QString &name)
	: PluginBase(name)
{
	m_recordingThread = new RecordingThread(this);
}


Recording::~Recording()
{
	if (m_recordingThread) {
		m_recordingThread->stop(true);
		delete m_recordingThread;
		m_recordingThread = NULL;
	}
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
	m_config.saveConfig(c);
}


void Recording::restoreState (KConfig *c)
{
    c->setGroup(QString("recording-") + PluginBase::name());
	m_config.restoreConfig(c);
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
	if (!m_recordingThread)
		return false;
		
	if (!m_recordingThread->running()) {
	    QString ext = ".wav";
	    switch (m_config.outputFormat) {
			case RecordingConfig::outputWAV:  ext = ".wav";  break;
			case RecordingConfig::outputAIFF: ext = ".aiff"; break;
			case RecordingConfig::outputAU:   ext = ".au";   break;
			case RecordingConfig::outputRAW:  ext = ".raw";  break;
			default:                          ext = ".wav";  break;
	    }
		QString output = m_config.directory
	       + "/kradio-recording"
	       + QDateTime::currentDateTime().toString(Qt::ISODate)
	       + ext;

		m_recordingThread->setConfig(m_config);
		m_recordingThread->start(output);
	}
	return true;
}


bool Recording::stopRecording()
{
	if (m_recordingThread)
		m_recordingThread->stop(false);
	return true;
}


bool Recording::setRecordingConfig(const RecordingConfig &c)
{
	m_config = c;
	// do not set recordingThread config, it might be running.
	notifyRecordingConfigChanged(m_config);
	return true;
}


bool Recording::isRecording() const
{
	if (m_recordingThread)
		return m_recordingThread->running();
	else
		return false;
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

// QT/KDE

void Recording::customEvent(QCustomEvent *e)
{
	if (!e) return;
	RecordingContext *c = (RecordingContext *)e->data();
	if (!c) return;
	m_context = *c;
	
	switch(e->type()) {
		case START_EVENT :
			notifyRecordingStarted(); break;
		case PROCESS_EVENT :			
		    notifyRecordingContextChanged(*c); break;
		case STOP_EVENT :
		    notifyRecordingStopped(); break;
		default:
			break;
	}

	delete c;
	e->setData(NULL);
}

