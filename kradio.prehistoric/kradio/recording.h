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
#ifdef HAVE_LAME_LAME_H
    #include <lame/lame.h>
#endif

#include <qobject.h>
#include <qstring.h>
#include <qmutex.h>
#include <qsemaphore.h>
#include <qthread.h>

#include "plugins.h"
#include "recording-interfaces.h"
#include "recording-context.h"
#include "timecontrol_interfaces.h"
#include "radio_interfaces.h"

class RadioStation;
class StationList;
class QSocketNotifier;
class RecordingEncoding;

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

    virtual bool   connectI(Interface *i);
    virtual bool   disconnectI(Interface *i);

    // PluginBase

public:
    virtual void   saveState (KConfig *) const;
    virtual void   restoreState (KConfig *);

    virtual ConfigPageInfo  createConfigurationPage();
    virtual AboutPageInfo   createAboutPage();


// IRecording

RECEIVERS:
    bool  startRecording();
    bool  startMonitoring();
    bool  stopRecording();
    bool  stopMonitoring();
    bool  setRecordingConfig(const RecordingConfig &);

ANSWERS:
    bool                    isRecording() const;
    bool                    isMonitoring() const;
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
    bool noticePresetFileChanged(const QString &/*f*/)           { return false; }



protected slots:

    bool event(QEvent *e);
    void slotSoundDataAvailable();

protected:

    bool openDevice();
    void closeDevice();

    bool openOutput(QString &filename);
    void closeOutput();

protected:
    int                m_devfd;
    char              *m_buffer;
    unsigned int       m_bufferBlockSize;

    RecordingConfig    m_config;
    RecordingContext   m_context;
    QSocketNotifier   *m_notifier;
    RecordingEncoding *m_encodingThread;
    int                m_skipCount;
};


class RecordingEncoding : public QThread
{
public:
    RecordingEncoding(QObject *parent, unsigned int bufferBlockSize, const RecordingConfig &c);
    RecordingEncoding::~RecordingEncoding();

    void run();

    char              *lockInputBuffer(unsigned int bufferSize);    // bytes we whish to write
    void               unlockInputBuffer(unsigned int bufferSize);  // bytes we actually wrote

    bool               error() const { return m_error; }
    const QString     &errorString() const { return m_errorString; }

    void               setDone();
    bool               IsDone() { return m_done; }

    bool               openOutput(const QString &outputFile, const QString &stationName);
    void               closeOutput();

    void               getEncodedSize (unsigned int &low, unsigned int &high) { low = m_encodedSizeLow; high =     m_encodedSizeHigh; }

protected:

    QObject           *m_parent;
    RecordingConfig    m_config;
    QMutex             m_bufferInputLock;
    QSemaphore         m_inputAvailableLock;
    bool               m_error;
    QString            m_errorString;
    bool               m_done;

    char             **m_buffersInput;
    unsigned int      *m_buffersInputFill,
                       m_currentInputBuffer,

                       m_encodedSizeLow,
                       m_encodedSizeHigh;

    SNDFILE           *m_output;
#ifdef HAVE_LAME_LAME_H
    unsigned char     *m_MP3Buffer;
    unsigned int       m_MP3BufferSize;
    FILE              *m_MP3Output;
    char              *m_ID3Tags;
    lame_global_flags *m_LAMEFlags;
    short int         *m_MP3LBuffer,
                      *m_MP3RBuffer;
#endif
};


#endif
