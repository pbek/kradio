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


#define BUFFER_SIZE 4096

class RecordingThread : public QThread
{
public:
	RecordingThread(QObject *parent, const RecordingConfig &c);
	~RecordingThread();
	void run();

	void stop(bool sync);

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
	char            *m_buffer;
	SNDFILE         *m_output;

	QObject         *m_parent;
	bool             m_stopFlag;
	RecordingContext m_context;
	RecordingConfig  m_config;
};

///////////////////////////////////////////////////////////////////////

RecordingThread::RecordingThread(QObject *parent, const RecordingConfig &c)
	: m_devfd(-1),
	  m_buffer(NULL),
	  m_output(NULL),
      m_parent(parent),
	  m_stopFlag(false),
	  m_config(c)
{
}

RecordingThread::~RecordingThread()
{
	stop(true);
}


void RecordingThread::stop(bool sync)
{
	m_stopFlag = true;
	if (sync) wait();
}


bool RecordingThread::openDevice()
{
	m_devfd = open(m_config.device, O_RDONLY);

    bool err = m_devfd < 0;

    int format = m_config.getOSSFormat();
    err |= (ioctl(m_devfd, SNDCTL_DSP_SETFMT, &format) != 0);

    int channels = m_config.channels;
    err |= (ioctl(m_devfd, SNDCTL_DSP_CHANNELS, &channels) != 0);

    int rate = m_config.rate;
    err |= (ioctl(m_devfd, SNDCTL_DSP_SPEED, &rate) != 0);

    if (err) closeDevice();

    m_buffer = new char[BUFFER_SIZE];
    
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
	sinfo.samplerate = m_config.rate;
	sinfo.channels   = m_config.channels;
	sinfo.format     = SF_FORMAT_WAV;

	if (m_config.bits == 8 && m_config.sign)
		sinfo.format |= SF_FORMAT_PCM_S8;
	if (m_config.bits == 8 && !m_config.sign)
		sinfo.format |= SF_FORMAT_PCM_U8;
	if (m_config.bits == 16)
		sinfo.format |= SF_FORMAT_PCM_16;

	sinfo.format |= m_config.littleEndian ? SF_ENDIAN_LITTLE : SF_ENDIAN_BIG;

	m_context.start(m_config.output);

    m_output = sf_open(m_config.output, SFM_WRITE, &sinfo);
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
	if (!internalStart()) {
		internalStop();
		return;
	}	
	while (!m_stopFlag) {
		int r = read(m_devfd, m_buffer, BUFFER_SIZE);
		if (r > 0 && sf_write_raw(m_output, m_buffer, r) == r) {
			m_context.bufferAdded(r, m_config);
			notifyParent(PROCESS_EVENT);
		} else {
			m_context.setError();
			m_stopFlag = 1;
		}
	}
	internalStop();
}


void RecordingThread::notifyParent(QEvent::Type MessageID)
{
	QApplication::postEvent(m_parent,
       new QCustomEvent(MessageID, new RecordingContext(m_context)));
}


///////////////////////////////////////////////////////////////////////

Recording::Recording(const QString &name)
	: PluginBase(name),
	  m_recordingDirectory(".")
{
	m_recordingThread = new RecordingThread(this, m_config);
}


Recording::~Recording()
{
	if (m_recordingThread) {
		m_recordingThread->stop(true);
		delete m_recordingThread;
	}
}


bool Recording::connect(Interface *i)
{
	bool a = IRecording::connect(i);
	return a;
}

bool Recording::disconnect(Interface *i)
{
	bool a = IRecording::disconnect(i);
	return a;
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
	// FIXME
	return ConfigPageInfo();
}


QWidget        *Recording::createAboutPage()
{
	// FIXME
	return NULL;
}


// IRecording

bool  Recording::startRecording()
{
	if (!m_recordingThread->running()) {

		m_config.output = m_recordingDirectory
	       + "/kradio-recording"
	       + QDateTime::currentDateTime().toString(Qt::ISODate)
	       + ".wav";
	
		m_recordingThread->start();
	}
	return true;
}


bool Recording::stopRecording()
{
	m_recordingThread->stop(false);
	return true;
}


bool Recording::setRecordingDirectory(const QString &s)
{
	if (s != m_recordingDirectory) {
		m_recordingDirectory = s;
		notifyRecordingDirectoryChanged(s);
	}
	return true;
}


bool Recording::isRecording() const
{
	return m_recordingThread->running();
}


const QString  &Recording::getRecordingDirectory() const
{
	return m_recordingDirectory;
}


void Recording::customEvent(QCustomEvent *e)
{
	if (!e) return;
	RecordingContext *c = (RecordingContext *)e->data();
	if (!c) return;
	
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

