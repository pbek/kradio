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
#include "errorlog-interfaces.h"

#include <qsocketnotifier.h>
#include <qevent.h>
#include <qapplication.h>
#include <qregexp.h>

#include <kconfig.h>
#include <kdeversion.h>

#include <sndfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>

#include <kaboutdata.h>
#include "aboutwidget.h"


QEvent::Type EncodingTerminated = (QEvent::Type)(QEvent::User+1);
QEvent::Type EncodingStep       = (QEvent::Type)(QEvent::User+2);

///////////////////////////////////////////////////////////////////////

Recording::Recording(const QString &name)
    : QObject(NULL, QString::null),
      PluginBase(name, i18n("Recording Plugin")),
      m_devfd(-1),
      m_buffer(NULL),
      m_bufferBlockSize(0),
      m_config(),
      m_context(),
      m_notifier(NULL),
      m_encodingThread(NULL),
      m_skipCount(0)
{

    QObject::connect(&m_pollTimer, SIGNAL(timeout()), this, SLOT(slotSoundDataAvailable()));

}


Recording::~Recording()
{
    stopRecording();
}


bool Recording::connectI(Interface *i)
{
    bool a = IRecording::connectI(i);
    bool b = ITimeControlClient::connectI(i);
    bool c = IRadioClient::connectI(i);
    bool d = PluginBase::connectI(i);
    return a || b || c || d;
}


bool Recording::disconnectI(Interface *i)
{
    bool a = IRecording::disconnectI(i);
    bool b = ITimeControlClient::disconnectI(i);
    bool c = IRadioClient::disconnectI(i);
    bool d = PluginBase::connectI(i);
    return a || b || c || d;
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
    notifyRecordingConfigChanged(m_config);
//    if (c->readBoolEntry("monitoring", false) && m_context.state() != RecordingContext::rsRunning)
//        startMonitoring();
}


ConfigPageInfo  Recording::createConfigurationPage()
{
    RecordingConfiguration *c = new RecordingConfiguration(NULL);
    connectI(c);
    return ConfigPageInfo(c,
                          i18n("Recording"),
                          i18n("Recording"),
                          "kradio_record");
}


AboutPageInfo Recording::createAboutPage()
{
    KAboutData aboutData("kradio",
                         NULL,
                         NULL,
                         I18N_NOOP("Recording Monitor for KRadio"),
                         KAboutData::License_GPL,
                         "(c) 2002, 2003 Martin Witte",
                         0,
                         "http://sourceforge.net/projects/kradio",
                         0);
    aboutData.addAuthor("Martin Witte",  "", "witte@kawo1.rwth-aachen.de");

    return AboutPageInfo(
              new KRadioAboutWidget(aboutData, KRadioAboutWidget::AbtTabbed),
              i18n("Recording"),
              i18n("Recording Plugin"),
              "kradio_record"
           );
}


// IRecording

bool  Recording::startRecording()
{
    bool oldPower = queryIsPowerOn();
    sendPowerOn();

    bool ok = true;
    if (m_context.state() == RecordingContext::rsRunning)
        return true;

    if (m_context.state() != RecordingContext::rsMonitor)
        ok &= openDevice();

    QString filename;
    ok &= openOutput(filename);
    logInfo(i18n("Recording starting"));
    if (ok) {
        m_context.start(filename, m_config);
        notifyRecordingStarted();
        notifyRecordingContextChanged(m_context);
        return true;
    } else {
        if (!oldPower)
            sendPowerOff();
        logError(i18n("Recording stopped with error"));
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
    logInfo(i18n("Monitoring starting"));
    if (ok) {
        m_context.startMonitor(m_config);
        notifyMonitoringStarted();
        notifyRecordingContextChanged(m_context);
        return true;
    } else {
        logError(i18n("Monitoring stopped with error"));
        m_context.setError();
        stopMonitoring();
        return false;
    }
}


bool Recording::stopRecording()
{
    RecordingContext::RecordingState oldState = m_context.oldState();
    logDebug("Recording::oldstate: " + QString().setNum(oldState));
    switch (m_context.state()) {
        case RecordingContext::rsRunning:
            if (!m_encodingThread->IsDone()) {
                closeDevice();
                m_encodingThread->setDone();
                break;
            }
        case RecordingContext::rsError:
            closeDevice();
            closeOutput();
            m_context.stop();
            notifyRecordingContextChanged(m_context);
            notifyRecordingStopped();
            logInfo(i18n("Recording stopped"));
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
            logInfo(i18n("Monitoring stopped"));
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

    m_devfd = open(m_config.device, O_RDONLY | O_NONBLOCK);

    bool err = m_devfd < 0;
    if (err)
        logError(i18n("Cannot open DSP device %1").arg(m_config.device));

    int format = m_config.getOSSFormat();
    err |= (ioctl(m_devfd, SNDCTL_DSP_SETFMT, &format) != 0);
    if (err)
        logError(i18n("Cannot set sample format for recording"));

    int channels = m_config.channels;
    err |= (ioctl(m_devfd, SNDCTL_DSP_CHANNELS, &channels) != 0);
    if (err)
        logError(i18n("Cannot set number of channels for recording"));

    int rate = m_config.rate;
    err |= (ioctl(m_devfd, SNDCTL_DSP_SPEED, &rate) != 0);
    if (err)
        logError(i18n("Cannot set sampling rate for recording"));
    if (rate != m_config.rate) {
        logWarning(i18n("Asking for recording at %1 Hz but hardware uses %2 Hz").
                   arg(QString::number(m_config.rate)).
                   arg(QString::number(rate)));
        m_config.rate = rate;
    }

    int stereo = m_config.channels == 2;
    err |= (ioctl(m_devfd, SNDCTL_DSP_STEREO, &stereo) != 0);
    if (err)
        logError(i18n("Cannot set stereo mode for recording"));

    int sampleSize = m_config.bits;
    err |= (ioctl(m_devfd, SNDCTL_DSP_SAMPLESIZE, &sampleSize) != 0);
    if (err || sampleSize != m_config.bits)
        logError(i18n("Cannot set sample size for recording"));

    // setup buffer, ask for 10ms latency
    int tmp  = (100 * m_config.frameSize() * m_config.rate) / 1000;
    int mask = -1;    for (; tmp; tmp >>= 1) ++mask;
    if (mask < 8) mask = 12;  // default 4kB
    mask |= 0x7FFF0000;
    err |= ioctl (m_devfd, SNDCTL_DSP_SETFRAGMENT, &mask);
    if (err)
        logError(i18n("Cannot set recording buffers"));

    err |= ioctl (m_devfd, SNDCTL_DSP_GETBLKSIZE, &m_bufferBlockSize);
    if (err)
        logError(i18n("Cannot read recording buffer size"));

    logInfo(i18n("Hardware uses buffer blocks of %1 bytes").arg(QString::number(m_bufferBlockSize)));

    m_buffer = new char[m_config.encodeBufferSize];

    int trigger = ~PCM_ENABLE_INPUT;
    ioctl(m_devfd, SNDCTL_DSP_SETTRIGGER, &trigger);
    trigger = PCM_ENABLE_INPUT;
    ioctl(m_devfd, SNDCTL_DSP_SETTRIGGER, &trigger);

    if (!err) {
/*        fd_set rfds;
        struct timeval tv;
        FD_ZERO(&rfds); FD_SET(m_devfd,&rfds);
        tv.tv_sec=0; tv.tv_usec = 0;
        if(!select(m_devfd+1, &rfds, NULL, NULL, &tv)) {
            logWarning(i18n("Your sound device does not support the select function. Using polling timer."));*/
            // setup polling
            m_pollTimer.start(40);
/*         } else {
            // setup Socket Notifier
            m_notifier = new QSocketNotifier(m_devfd, QSocketNotifier::Read, this);
            QObject::connect(m_notifier, SIGNAL(activated(int)),
                              this, SLOT(slotSoundDataAvailable()));
         }*/
    }

    if (err) closeDevice();

    m_skipCount    = 0;

    return !err;
}


void Recording::closeDevice()
{
    m_pollTimer.stop();

    if (m_notifier) delete m_notifier;
    m_notifier = NULL;

    if (m_devfd >= 0) close (m_devfd);
    m_devfd = -1;

    if (m_buffer) delete m_buffer;
    m_buffer = NULL;
    m_bufferBlockSize  = 0;
}


bool Recording::openOutput(QString &output)
{
    if (m_encodingThread)
        return false;

    QString ext = ".wav";
    switch (m_config.outputFormat) {
        case RecordingConfig::outputWAV:  ext = ".wav";  break;
        case RecordingConfig::outputAIFF: ext = ".aiff"; break;
        case RecordingConfig::outputAU:   ext = ".au";   break;
#ifdef HAVE_LAME_LAME_H
        case RecordingConfig::outputMP3:  ext = ".mp3";  break;
#endif
        case RecordingConfig::outputRAW:  ext = ".raw";  break;
        default:                          ext = ".wav";  break;
    }
    QString station = queryCurrentStation().name();
    station.replace(QRegExp("[/*?]"), "_");
    output = m_config.directory
        + "/kradio-recording-"
        + station + "-" +
        + QDateTime::currentDateTime().toString(Qt::ISODate)
        + ext;

    logInfo(i18n("Recording::outputFile: ") + output);

    m_encodingThread = new RecordingEncoding(this, m_bufferBlockSize, m_config);

    m_encodingThread->openOutput(output, station);

    if (m_encodingThread->error()) {
        m_context.setError();
        logError(m_encodingThread->errorString());
    } else {
        m_encodingThread->start();
    }
    return !m_encodingThread->error();
}


void Recording::closeOutput()
{
    if (m_encodingThread) {
        m_encodingThread->setDone();
#if (KDE_VERSION_MAJOR >= 3) && (KDE_VERSION_MINOR >= 1)
        if (!m_encodingThread->wait(5000)) {
            m_context.setError();
            logError(i18n("The encoding thread did not finish. It will be killed now."));
            m_encodingThread->terminate();
            m_encodingThread->wait();
        } else {
#else
            logError(i18n("Waiting for encoding thread to terminate."));
            m_encodingThread->wait();
#endif
            if (m_encodingThread->error()) {
                m_context.setError();
                logError(m_encodingThread->errorString());
            } else {
                unsigned int low, high;
                m_encodingThread->getEncodedSize(low, high);
                m_context.setEncodedSize(low, high);
                notifyRecordingContextChanged(m_context);
            }
        }
        delete m_encodingThread;
        m_encodingThread=NULL;
    }
}


bool Recording::event(QEvent *e)
{
    if (e->type() == EncodingStep || e->type() == EncodingTerminated) {
        if (m_encodingThread) {
            if (m_encodingThread->error()) {
                m_context.setError();
                stopRecording();
            } else {
                unsigned int low, high;
                m_encodingThread->getEncodedSize(low, high);
                m_context.setEncodedSize(low, high);
                notifyRecordingContextChanged(m_context);
                if (e->type() == EncodingTerminated) {
                    stopRecording();
                }
            }
        }
        return true;
    } else {
        return QObject::event(e);
    }
}


void Recording::slotSoundDataAvailable()
{
    if (m_context.state() == RecordingContext::rsRunning ||
        m_context.state() == RecordingContext::rsMonitor)
    {
        char *buf = m_buffer;
        bool skip = false;
        unsigned int bufferSize = m_config.encodeBufferSize;
        if (m_encodingThread) {
            buf = m_encodingThread->lockInputBuffer(bufferSize);
            if (buf && m_skipCount) {
                logWarning(i18n("Input buffer overflow. Skipped %1 input bytes").arg(QString::number(m_skipCount)));
                m_skipCount = 0;
            }
            if (!buf) {
                buf = m_buffer;
                skip = true;
            }
        }
        int err = 0;
        int bytesRead = 0;
        if (buf) {
            bytesRead = read(m_devfd, buf, bufferSize);
            if (bytesRead > 0) {
                m_context.addInput(buf, bytesRead, 0);
                if (skip)
                    m_skipCount += bytesRead;
            } else if (bytesRead < 0 && errno == EAGAIN) {
                bytesRead = 0;
            } else if (bytesRead == 0) {
                err = -1;
                logError(i18n("No data to record"));
            } else {
                err = errno;
            }

            if (!skip && m_encodingThread)
                m_encodingThread->unlockInputBuffer(bytesRead > 0 ? bytesRead : 0);
        }

        if (m_encodingThread && m_encodingThread->error()) {
            logError(m_encodingThread->errorString());
            err = true;
        }

        if (!err) {
            if (bytesRead) notifyRecordingContextChanged(m_context);
        } else {
            logError(i18n("Error %1 while recording").arg(QString().setNum(err)));
            m_context.setError();
            stopRecording();
        }
    }
}






RecordingEncoding::RecordingEncoding(QObject *parent, unsigned int bufferBlockSize, const RecordingConfig &cfg)
    :
      m_parent(parent),
      m_config(cfg),
      m_inputAvailableLock(1),
      m_error(false),
      m_errorString(QString::null),
      m_done(false),
      m_buffersInput(NULL),
      m_buffersInputFill(NULL),
      m_currentInputBuffer(0),
      m_encodedSizeLow(0),
      m_encodedSizeHigh(0),
      m_output(NULL),
#ifdef HAVE_LAME_LAME_H
      m_MP3Buffer(NULL),
      m_MP3BufferSize(0),
      m_MP3Output(NULL),
      m_ID3Tags(NULL),
      m_LAMEFlags(NULL),
      m_MP3LBuffer(NULL),
      m_MP3RBuffer(NULL)
#endif
{
    if (m_config.encodeBufferCount < 3)
        m_config.encodeBufferCount = 3;
    if (m_config.encodeBufferSize < bufferBlockSize)
        m_config.encodeBufferSize = bufferBlockSize;

    m_buffersInput = new char* [m_config.encodeBufferCount];
    m_buffersInputFill = new unsigned int [m_config.encodeBufferCount];
    for (unsigned int i = 0; i < m_config.encodeBufferCount; ++i) {
        m_buffersInput[i] = new char [m_config.encodeBufferSize];
        m_buffersInputFill[i] = 0;
    }

    m_inputAvailableLock++;
}


RecordingEncoding::~RecordingEncoding()
{
    for (unsigned int i = 0; i < m_config.encodeBufferCount; ++i) {
        delete m_buffersInput[i];
    }
    delete m_buffersInput;
    delete m_buffersInputFill;
    m_buffersInputFill = NULL;
    m_buffersInput = NULL;
    closeOutput();
}


char *RecordingEncoding::lockInputBuffer(unsigned int &bufferSize)
{
    if (m_done)
        return NULL;

    m_bufferInputLock.lock();

    unsigned int bytesAvailable = 0;

    do {
        bytesAvailable = m_config.encodeBufferSize - m_buffersInputFill[m_currentInputBuffer];
        if (bytesAvailable > 0) {
            bufferSize = bytesAvailable;
            return m_buffersInput[m_currentInputBuffer] + m_buffersInputFill[m_currentInputBuffer];
        } else if (m_currentInputBuffer + 1 < m_config.encodeBufferCount) {
            ++m_currentInputBuffer;
            m_inputAvailableLock--;
        }
    } while (m_currentInputBuffer + 1 < m_config.encodeBufferCount);
    m_bufferInputLock.unlock();
    return NULL;
}


void  RecordingEncoding::unlockInputBuffer(unsigned int bufferSize)
{
    if (m_done)
        return;

    if (m_buffersInputFill[m_currentInputBuffer] + bufferSize > m_config.encodeBufferSize) {
        m_error = true;
        m_errorString = "Buffer Overflow";
    } else {
        m_buffersInputFill[m_currentInputBuffer] += bufferSize;
    }
    m_bufferInputLock.unlock();
}


void RecordingEncoding::setDone()
{
    m_done = true;
    m_inputAvailableLock--;
}


static QMutex  lameSerialization;

void RecordingEncoding::run()
{
    while (!m_error) {
        if (!m_done)
            m_inputAvailableLock++;

        if (!m_buffersInputFill[0]) {
            if (m_done)
                break;
            else
                continue;
        }

#ifdef HAVE_LAME_LAME_H
        if (m_config.outputFormat != RecordingConfig::outputMP3) {
#endif
            m_encodedSizeLow += m_buffersInputFill[0];
            if (m_encodedSizeLow < m_buffersInputFill[0])
                ++m_encodedSizeHigh;

            int err = sf_write_raw(m_output, m_buffersInput[0], m_buffersInputFill[0]);
            if ((m_error = (err != (int)m_buffersInputFill[0])))
                m_errorString = i18n("Error %1 writing output").arg(QString().setNum(err));

#ifdef HAVE_LAME_LAME_H
        } else {
            short int *buffer = (short int*)m_buffersInput[0];
            int j       = 0,
                j_inc   = (m_config.channels == 1) ? 1 : 2,
                dj      = (m_config.channels == 1) ? 0 : 1,
                samples = m_buffersInputFill[0] / m_config.frameSize();
            for (int i = 0; i < samples; ++i, j+=j_inc) {
                m_MP3LBuffer[i] = buffer[j];
                m_MP3RBuffer[i] = buffer[j+dj];
            }

            int n = m_MP3BufferSize;
            lameSerialization.lock();
            n = lame_encode_buffer(m_LAMEFlags,
                                   m_MP3LBuffer,
                                   m_MP3RBuffer,
                                   samples,
                                   m_MP3Buffer,
                                   m_MP3BufferSize);
            lameSerialization.unlock();
            if (n < 0) {
                m_errorString = i18n("Error %1 while encoding mp3").arg(QString().setNum(n));
                m_error       = true;
            } else if (n > 0) {
                m_encodedSizeLow += n;
                if (m_encodedSizeLow < (unsigned)n)
                    ++m_encodedSizeHigh;

                int r = fwrite(m_MP3Buffer, n, 1, m_MP3Output);
                if (r <= 0) {
                    m_errorString = i18n("Error %1 writing output").arg(QString().setNum(r));
                    m_error = true;
                }
            }
        }
#endif

        m_bufferInputLock.lock();

        char *tmpBuf = m_buffersInput[0];
        for (unsigned int i = 0; i < m_config.encodeBufferCount - 1; ++i) {
            m_buffersInput    [i] = m_buffersInput    [i+1];
            m_buffersInputFill[i] = m_buffersInputFill[i+1];
        }
        m_buffersInput    [m_config.encodeBufferCount - 1] = tmpBuf;
        m_buffersInputFill[m_config.encodeBufferCount - 1] = 0;
        m_currentInputBuffer--;

        m_bufferInputLock.unlock();

        QApplication::postEvent(m_parent, new QEvent(EncodingStep));
    }
    closeOutput();
    QApplication::postEvent(m_parent, new QEvent(EncodingTerminated));
}


bool RecordingEncoding::openOutput(const QString &output, const QString &station)
{
#ifdef HAVE_LAME_LAME_H
    if (m_config.outputFormat != RecordingConfig::outputMP3) {
#endif
        SF_INFO sinfo;
        m_config.getSoundFileInfo(sinfo, false);
        m_output = sf_open(output, SFM_WRITE, &sinfo);

        if (!m_output) {
            m_error = true;
            m_errorString = i18n("Cannot open output file %1").arg(output);
        }

#ifdef HAVE_LAME_LAME_H
    } else {
        m_output = NULL;
        m_LAMEFlags = lame_init();

        bool ok = m_LAMEFlags;

        if (ok) {
            lame_set_in_samplerate(m_LAMEFlags, m_config.rate);
            lame_set_num_channels(m_LAMEFlags, 2);
            lame_set_quality(m_LAMEFlags, m_config.mp3Quality);

            lame_set_mode(m_LAMEFlags, m_config.channels == 1 ? MONO : JOINT_STEREO);

    //        lame_seterrorf(m_LAMEFlags, ...);
    //        lame_setdebugf(m_LAMEFlags, ...);
    //        lame_setmsgf(m_LAMEFlags, ...);

            lame_set_VBR(m_LAMEFlags, vbr_default);
            lame_set_VBR_q(m_LAMEFlags, m_config.mp3Quality);

            id3tag_init(m_LAMEFlags);
            id3tag_add_v2(m_LAMEFlags);
            QString title  = station + QString().sprintf(" - %s", (const char*)(QDateTime::currentDateTime().toString(Qt::ISODate)));
            QString comment = i18n("Recorded by KRadio");
            int l = title.length() + comment.length() + 10;
            m_ID3Tags = new char[l];
            char *ctitle   = m_ID3Tags;
            strcpy(ctitle, title.latin1());
            char *ccomment = m_ID3Tags + strlen(ctitle) + 1;
            strcpy(ccomment, comment.latin1());
            id3tag_set_title(m_LAMEFlags, ctitle);
            id3tag_set_comment(m_LAMEFlags, ccomment);

            ok &= (lame_init_params(m_LAMEFlags) != -1);

            if (!ok)
                m_errorString = i18n("Cannot initialize liblame").arg(output);

            m_MP3Output = fopen(output, "w");
            if (!m_MP3Output)
                m_errorString = i18n("Cannot open output file %1").arg(output);

            int nSamples = m_config.encodeBufferSize / m_config.frameSize();
            m_MP3BufferSize = nSamples + nSamples / 4 + 7200;
            m_MP3Buffer  = new unsigned char[m_MP3BufferSize];

            m_MP3LBuffer = new short int[nSamples];
            m_MP3RBuffer = new short int[nSamples];
        }

        ok &= m_MP3Output && m_MP3Buffer;
        if (!ok) {
            if (m_LAMEFlags) lame_close(m_LAMEFlags);
            m_LAMEFlags = NULL;
            if (m_MP3Output) fclose(m_MP3Output);
            m_MP3Output = NULL;
            if (m_MP3Buffer) delete [] m_MP3Buffer;
            m_MP3Buffer = NULL;
            m_MP3BufferSize = 0;
            if (m_ID3Tags) delete [] m_ID3Tags;
            m_ID3Tags = NULL;
            if (m_MP3LBuffer) delete[] m_MP3LBuffer;
            if (m_MP3RBuffer) delete[] m_MP3RBuffer;
            m_MP3LBuffer = m_MP3RBuffer = NULL;

            m_error = true;
        }
    }
#endif
    m_error = !m_output && !m_MP3Output;
    return !m_error;
}



void RecordingEncoding::closeOutput()
{
    if (m_output) sf_close (m_output);
    m_output = NULL;
#ifdef HAVE_LAME_LAME_H
    if (m_LAMEFlags) {
        if (m_config.outputFormat == RecordingConfig::outputMP3) {
            int n = lame_encode_flush(m_LAMEFlags,
                                      m_MP3Buffer,
                                      m_MP3BufferSize);
            if (n < 0) {
                m_error = true;
                m_errorString = i18n("Error %1 while encoding mp3").arg(QString().setNum(n));
            } else if (n > 0) {
                int r = fwrite(m_MP3Buffer, n, 1, m_MP3Output);
                if (r <= 0) {
                    m_error = true;
                    m_errorString = i18n("Error %1 writing output").arg(QString().setNum(r));
                } else {
                    lame_mp3_tags_fid(m_LAMEFlags, m_MP3Output);
                }
            }
        }
        if (m_LAMEFlags) lame_close(m_LAMEFlags);
        m_LAMEFlags = NULL;
        if (m_MP3Output) fclose(m_MP3Output);
        m_MP3Output = NULL;
        m_MP3BufferSize = 0;
        if (m_MP3Buffer) delete [] m_MP3Buffer;
        m_MP3Buffer = NULL;
        if (m_ID3Tags) delete [] m_ID3Tags;
        m_ID3Tags = NULL;
        if (m_MP3LBuffer) delete[] m_MP3LBuffer;
        if (m_MP3RBuffer) delete[] m_MP3RBuffer;
        m_MP3LBuffer = m_MP3RBuffer = NULL;
    }
#endif
}
