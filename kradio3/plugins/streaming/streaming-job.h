/***************************************************************************
                          streaming-job.h  -  description
                             -------------------
    begin                : Sun Sept 3 2006
    copyright            : (C) 2006 by Martin Witte
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

#ifndef _KRADIO_STREAMING_JOB_H
#define _KRADIO_STREAMING_JOB_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../../src/include/ringbuffer.h"
#include "../../src/include/soundformat.h"
#include "../../src/include/soundstreamclient_interfaces.h"

#include <qobject.h>

#include <kio/jobclasses.h>

class StreamingJob : public QObject
{
Q_OBJECT
public:
    StreamingJob();
    StreamingJob(const QString &_URL, const SoundFormat &_SoundFormat, size_t _bufferSize);
    StreamingJob(const StreamingJob &c);

    virtual ~StreamingJob();

    const QString     &getURL()         const { return m_URL; }
    const SoundFormat &getSoundFormat() const { return m_SoundFormat; }
    int                getBufferSize()  const { return m_BufferSize; }

    void setURL(const QString &);
    void setSoundFormat(const SoundFormat &);
    void setBufferSize(size_t buffer_size);

    bool startPlayback();
    bool stopPlayback();

    bool startCapture(const SoundFormat &proposed_format,
                      SoundFormat       &real_format,
                      bool               force_format);
    bool stopCapture();


    void playData(const char *data, size_t size, size_t &consumed_size);
    bool hasRecordedData() const;
    void lockData(const char *&data, size_t &size, SoundMetaData &meta_data);
    void removeData(size_t);

protected slots:

    void   slotReadData  (KIO::Job *job, const QByteArray &data);
    void   slotWriteData (KIO::Job *job, QByteArray &data);
    void   slotIOJobResult (KIO::Job *job);

signals:

    void   logStreamError(const KURL &url, const QString &s);
    void   logStreamWarning(const KURL &url, const QString &s);

protected:

    bool   startGetJob();
    bool   startPutJob();


    QString           m_URL;
    SoundFormat       m_SoundFormat;

    size_t            m_BufferSize;
    RingBuffer        m_Buffer;

    unsigned          m_OpenCounter;
    Q_UINT64          m_StreamPos;
    time_t            m_StartTime;

    size_t            m_SkipCount;

    KIO::TransferJob *m_KIO_Job;
    bool              m_capturing;
};



#endif
