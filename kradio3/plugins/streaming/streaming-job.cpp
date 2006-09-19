/***************************************************************************
                          streaming-job.cpp  -  description
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

#include "streaming-job.h"

#include "../../src/libkradio/utils.h"
#include <kurl.h>

#include <kio/job.h>


StreamingJob::StreamingJob()
  : QObject(),
    m_URL(QString::null),
    m_SoundFormat(),
    m_BufferSize(65536),
    m_Buffer(m_BufferSize),
    m_OpenCounter(0),
    m_StreamPos(0),
    m_StartTime(0),
    m_SkipCount(0),
    m_KIO_Job(NULL),
    m_capturing(false)
{
}

StreamingJob::StreamingJob(const QString &_URL, const SoundFormat &_SoundFormat, size_t _bufferSize)
  : QObject(),
    m_URL(_URL),
    m_SoundFormat(_SoundFormat),
    m_BufferSize(_bufferSize),
    m_Buffer(m_BufferSize),
    m_OpenCounter(0),
    m_StreamPos(0),
    m_StartTime(0),
    m_SkipCount(0),
    m_KIO_Job(NULL),
    m_capturing(false)
{
}

StreamingJob::StreamingJob(const StreamingJob &c)
  : QObject(),
    m_URL(c.m_URL),
    m_SoundFormat(c.m_SoundFormat),
    m_BufferSize(c.m_BufferSize),
    m_Buffer(m_BufferSize),
    m_OpenCounter(0),
    m_StreamPos(0),
    m_StartTime(0),
    m_SkipCount(0),
    m_KIO_Job(NULL),
    m_capturing(c.m_capturing)
{
}

StreamingJob::~StreamingJob()
{
}


void StreamingJob::setURL(const QString &url)
{
    if (m_URL != url) {
        m_URL = url;
        delete m_KIO_Job;
        m_KIO_Job = NULL;
        if (!m_capturing) {
            startPutJob();
        } else {    
            startGetJob();
        }
    }
}


void StreamingJob::setSoundFormat(const SoundFormat &sf)
{
    m_SoundFormat = sf;
}


void StreamingJob::setBufferSize(size_t buffer_size)
{
    if (m_BufferSize != buffer_size) {
        m_Buffer.clear();
        m_Buffer.resize(m_BufferSize = buffer_size);
    }
}


bool StreamingJob::startPutJob()
{
    m_KIO_Job = KIO::put(m_URL, -1, true, false, false);
    if (!m_KIO_Job)
        return false;
    m_KIO_Job->setAsyncDataEnabled(true);
    connect (m_KIO_Job, SIGNAL(dataReq(KIO::Job *job, QByteArray &data)),
        this, SLOT(slotWriteData  (KIO::Job *job, QByteArray &data)));
    connect (m_KIO_Job, SIGNAL(result(KIO::Job *)),
        this, SLOT(slotIOJobResult(KIO::Job *)));
    return true;
}


bool StreamingJob::startPlayback()
{
    if (!m_OpenCounter) {
        m_Buffer.clear();
        m_OpenCounter = 1;
        if (!startPutJob())
            return false;
        m_StartTime = time(NULL);
        m_StreamPos = 0;
        if (m_KIO_Job->error()) {
            emit logStreamError(m_URL, m_KIO_Job->errorString());
        }
        return m_KIO_Job->error() == 0;
    }
    else {
        return true;
    }
}

bool StreamingJob::stopPlayback()
{
    if (m_OpenCounter) {
        if (!--m_OpenCounter) {
            delete m_KIO_Job;
            m_KIO_Job = NULL;
        }
    }
    return true;
}


bool StreamingJob::startGetJob()
{
    m_KIO_Job = KIO::get(m_URL, false, false);
    if (!m_KIO_Job)
        return false;
    m_KIO_Job->setAsyncDataEnabled(true);
    connect (m_KIO_Job, SIGNAL(data(KIO::Job *, const QByteArray &)),
                this, SLOT(slotReadData(KIO::Job *, const QByteArray &)));
    connect (m_KIO_Job, SIGNAL(result(KIO::Job *)),
                this, SLOT(slotIOJobResult(KIO::Job *)));
    return true;
}


bool StreamingJob::startCapture(const SoundFormat &/*proposed_format*/,
                                SoundFormat       &real_format,
                                bool               /*force_format*/)
{
    if (!m_OpenCounter) {
        m_capturing = true;
        m_Buffer.clear();
        if (!startGetJob())
            return false;
        m_StartTime = time(NULL);
        m_StreamPos = 0;
        if (m_KIO_Job->error()) {
            emit logStreamError(m_URL, m_KIO_Job->errorString());
        }
        return m_KIO_Job->error() == 0;
    }
    ++m_OpenCounter;
    real_format = m_SoundFormat;
    return true;
}


bool StreamingJob::stopCapture()
{
    if (m_OpenCounter) {
        if (!--m_OpenCounter) {
            delete m_KIO_Job;
            m_KIO_Job = NULL;
        }
    }
    return true;
}


void   StreamingJob::slotReadData  (KIO::Job */*job*/, const QByteArray &data)
{
    size_t free = m_Buffer.getFreeSize();
    if (free < data.size()) {
        m_SkipCount += data.size() - free;
        emit logStreamWarning(m_URL, i18n("skipped %1 bytes").arg(data.size() - free)); 
    }
    else {
        free = data.size();
    }

    m_Buffer.addData(data.data(), free);
    m_StreamPos += free;

    if (m_Buffer.getFreeSize() < data.size()) {
        m_KIO_Job->suspend();
    }
}


void   StreamingJob::slotWriteData (KIO::Job */*job*/, QByteArray &)
{
    size_t size = m_Buffer.getFillSize();
    if (size) {
        char *buf = new char [size];
        size = m_Buffer.takeData(buf, size);
        QByteArray data;
        data.assign(buf, size);
        m_KIO_Job->sendAsyncData(data);
        m_StreamPos += size;
    }
    else {
        // does a warning really make sense here?
        //emit logStreamWarning(m_URL, i18n("buffer underrun")); 
        m_SkipCount++; 
    }
}


void StreamingJob::playData(const char *data, size_t size, size_t &consumed_size)
{
    size_t free = m_Buffer.getFreeSize();
    consumed_size = (consumed_size == SIZE_T_DONT_CARE) ? free : min(consumed_size, free);
    if (free > size) {
        free = size;
    }
    m_Buffer.addData(data, free);
}


bool   StreamingJob::hasRecordedData() const
{
    return m_Buffer.getFillSize() > m_Buffer.getSize() / 3;
}


void StreamingJob::lockData(const char *&data, size_t &size, SoundMetaData &meta_data)
{
    data = m_Buffer.getData(size);
    time_t cur_time = time(NULL);
    meta_data = SoundMetaData(m_StreamPos, cur_time - m_StartTime, cur_time, m_URL);
}


void StreamingJob::removeData(size_t size)
{
    m_Buffer.removeData(size);
    if (m_Buffer.getFreeSize() > m_Buffer.getSize() / 2) {
        m_KIO_Job->resume();
    }
}

void   StreamingJob::slotIOJobResult (KIO::Job *job)
{
    if (job && job->error()) {
        emit logStreamError(m_URL, job->errorString());
    }
}
