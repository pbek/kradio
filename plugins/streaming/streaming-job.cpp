/***************************************************************************
                          streaming-job.cpp  -  description
                             -------------------
    begin                : Sun Sept 3 2006
    copyright            : (C) 2006 by Martin Witte
    email                : emw-kradio@nocabal.de
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

#include "utils.h"


#include <QFile>
#include <QSocketNotifier>
#include <QIODevice>
#include <kurl.h>
#include <kio/job.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#ifndef O_DIRECT
#define O_DIRECT 0
#endif

StreamingJob::StreamingJob()
  : QObject(),
    m_SoundFormat(),
    m_BufferSize(65536),
    m_Buffer(m_BufferSize),
    m_OpenCounter(0),
    m_StreamPos(0),
    m_StartTime(0),
    m_SkipCount(0),
    m_KIO_Job(NULL),
    m_SocketNotifier(NULL),
    m_File(NULL),
    m_capturing(false)
{
}

StreamingJob::StreamingJob(const KUrl &_URL, const SoundFormat &_SoundFormat, size_t _bufferSize)
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
    m_SocketNotifier(NULL),
    m_File(NULL),
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
    m_SocketNotifier(NULL),
    m_File(NULL),
    m_capturing(c.m_capturing)
{
}

StreamingJob::~StreamingJob()
{
    if (m_KIO_Job)
        m_KIO_Job->kill();
    if (m_SocketNotifier)
        delete m_SocketNotifier;
    if (m_File)
        delete m_File;

    m_SocketNotifier = NULL;
    m_File           = NULL;
    m_KIO_Job        = NULL;
}


void StreamingJob::setURL(const KUrl &url)
{
    if (m_URL != url) {
        m_URL = url;
        if (m_KIO_Job)
            m_KIO_Job->kill();
        if (m_SocketNotifier)
            delete m_SocketNotifier;
        if (m_File)
            delete m_File;
        m_KIO_Job        = NULL;
        m_SocketNotifier = NULL;
        m_File           = NULL;
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
    if (m_URL.isLocalFile()) {
        m_File = new QFile(m_URL.pathOrUrl());
        m_File->open(QIODevice::WriteOnly | QIODevice::Append);
        int ret = fcntl(m_File->handle(), F_SETFL, O_APPEND | O_NONBLOCK | O_DIRECT);
        if (ret < 0) {
            emit logStreamWarning(m_URL, i18n("error %1 (%2)", strerror(errno), errno));
        }
        m_SocketNotifier = new QSocketNotifier(m_File->handle(), QSocketNotifier::Write);
        connect(m_SocketNotifier, SIGNAL(activated(int)), this, SLOT(slotWriteData(int)));
        m_SocketNotifier->setEnabled(false);
    } else {
        m_KIO_Job = KIO::put(m_URL, -1, KIO::Overwrite);
        if (!m_KIO_Job)
            return false;
        m_KIO_Job->setAsyncDataEnabled(true);
        connect (m_KIO_Job, SIGNAL(dataReq(KIO::Job *, QByteArray &)),
                 this,      SLOT(slotWriteData  (KIO::Job *, QByteArray &)));
        connect (m_KIO_Job, SIGNAL(result(KIO::Job *)),
                 this,      SLOT(slotIOJobResult(KIO::Job *)));
    }
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
        if (m_KIO_Job && m_KIO_Job->error()) {
            emit logStreamError(m_URL, i18n("%1 (%2)", m_KIO_Job->errorString(), m_KIO_Job->error()));
            m_KIO_Job->kill();
            m_KIO_Job = NULL;
            m_OpenCounter = 0;
            return false;
        }
        else if (m_File && m_File->error()) {
            emit logStreamError(m_URL, i18n("%1 (%2)", m_File->errorString(), m_File->error()));
            delete m_SocketNotifier;
            delete m_File;
            m_File           = NULL;
            m_SocketNotifier = NULL;
            m_OpenCounter = 0;
            return false;
        }
        return true;
    }
    else {
        return true;
    }
}

bool StreamingJob::stopPlayback()
{
    if (m_OpenCounter) {
        if (!--m_OpenCounter) {
            if (m_KIO_Job)
                m_KIO_Job->kill();
            if (m_SocketNotifier)
                delete m_SocketNotifier;
            if (m_File)
                delete m_File;
            m_File           = NULL;
            m_SocketNotifier = NULL;
            m_KIO_Job        = NULL;
        }
    }
    return true;
}


bool StreamingJob::startGetJob()
{
    if (m_URL.isLocalFile()) {
        m_File = new QFile(m_URL.pathOrUrl());
        m_File->open(QIODevice::ReadOnly);
        int fileno = m_File->handle();
        int ret = fcntl(fileno, F_SETFL, O_NONBLOCK);
        if (ret < 0) {
            int err = errno;
            emit logStreamWarning(m_URL, i18n("error %1 (%2)", strerror(errno), err));
        }
        m_SocketNotifier = new QSocketNotifier(m_File->handle(), QSocketNotifier::Read);
        connect(m_SocketNotifier, SIGNAL(activated(int)), this, SLOT(slotReadData(int)));
        m_SocketNotifier->setEnabled(true);
    } else {
        m_KIO_Job = KIO::get(m_URL, KIO::NoReload, KIO::DefaultFlags);
        if (!m_KIO_Job)
            return false;
        m_KIO_Job->setAsyncDataEnabled(true);
        connect (m_KIO_Job, SIGNAL(data(KIO::Job *, const QByteArray &)),
                 this,      SLOT(slotReadData(KIO::Job *, const QByteArray &)));
        connect (m_KIO_Job, SIGNAL(result(KIO::Job *)),
                 this,      SLOT(slotIOJobResult(KIO::Job *)));
    }
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
        if (m_KIO_Job && m_KIO_Job->error()) {
            emit logStreamError(m_URL, m_KIO_Job->errorString());
            m_KIO_Job->kill();
            m_KIO_Job = NULL;
            m_OpenCounter = 0;
            return false;
        }
        else if (m_File && m_File->error()) {
            emit logStreamError(m_URL, i18n("%1 (%2)", m_File->errorString(), m_File->error()));
            delete m_SocketNotifier;
            delete m_File;
            m_File           = NULL;
            m_SocketNotifier = NULL;
            m_OpenCounter = 0;
            return false;
        }
    }
    ++m_OpenCounter;
    real_format = m_SoundFormat;
    return true;
}


bool StreamingJob::stopCapture()
{
    if (m_OpenCounter) {
        if (!--m_OpenCounter) {
            if (m_KIO_Job)
                m_KIO_Job->kill();
            if (m_SocketNotifier)
                delete m_SocketNotifier;
            if (m_File)
                delete m_File;

            m_File           = NULL;
            m_SocketNotifier = NULL;
            m_KIO_Job        = NULL;
        }
    }
    return true;
}


void   StreamingJob::slotReadData  (KIO::Job */*job*/, const QByteArray &data)
{
    qint64 free = m_Buffer.getFreeSize();
    if (free < data.size()) {
        m_SkipCount += data.size() - free;
        emit logStreamWarning(m_URL, i18np("skipped %1 byte", "skipped %1 bytes", data.size() - free));
    }
    else {
        free = data.size();
    }

    m_Buffer.addData(data.data(), free);
    m_StreamPos += free;

    if (m_Buffer.getFreeSize() < (quint64)data.size()) {
        if (m_KIO_Job)
            m_KIO_Job->suspend();
    }
}



void   StreamingJob::slotReadData  (int fileno)
{
    size_t free = m_Buffer.getFreeSize();
    if (free == 0) {
        m_SocketNotifier->setEnabled(false);
        // no skips, streaming can wait ;-) we just need first a bit time to playback buffer
//         m_SkipCount++;
/*    } else if (m_SkipCount) {
        emit logStreamWarning(m_URL, i18n("skipped %1 reads", m_SkipCount));
        m_SkipCount = 0;*/
    }
    while ((free = m_Buffer.getFreeSize()) > 0) {
        char *buf = m_Buffer.getFreeSpace(free);
        ssize_t len = read(fileno, buf, free);
        if (len > 0) {
            m_Buffer.removeFreeSpace(len);
            m_StreamPos += len;
        } else if (len == 0) {
            m_SocketNotifier->setEnabled(false);
            // end of file reached. just disable and break. Streaming is however not intended for "finite" file length ;-)
            break;
        } else {
            int err = errno;
            if (err == EAGAIN) {
                // do nothing ( and break.. see below)
            } else {
                m_SocketNotifier->setEnabled(false);
                emit logStreamWarning(m_URL, i18n("error no %1", err));
            }
            break;
        }
    }
}


void   StreamingJob::slotWriteData (KIO::Job */*job*/, QByteArray &)
{
    size_t size = m_Buffer.getFillSize();
    if (size) {
        if (m_SkipCount) {
            emit logStreamWarning(m_URL, i18np("skipped %1 write", "skipped %1 writes", m_SkipCount));
            m_SkipCount = 0;
        }
        char *buf = new char [size];
        size = m_Buffer.takeData(buf, size);
        if (m_KIO_Job)
            m_KIO_Job->sendAsyncData(QByteArray::fromRawData(buf, size));
        delete buf;
        m_StreamPos += size;
    }
    else {
        // does a warning really make sense here?
        //emit logStreamWarning(m_URL, i18n("buffer underrun"));
        m_SkipCount++;
    }
}

void   StreamingJob::slotWriteData (int fileno)
{
    m_SocketNotifier->setEnabled(false);
    size_t size = m_Buffer.getFillSize();
    if (size) {
        if (m_SkipCount) {
            emit logStreamWarning(m_URL, i18np("skipped %1 write", "skipped %1 writes", m_SkipCount));
            m_SkipCount = 0;
        }
        size_t buf_size = 0;
        char *buf = m_Buffer.getData(buf_size);
        ssize_t len = write(fileno, buf, buf_size);
        if (len >= 0) {
            m_Buffer.removeData(len);
            m_StreamPos += len;
            m_SocketNotifier->setEnabled(m_Buffer.getFillSize() > 0);
        } else {
            int err = errno;
            if (err == EAGAIN) {
                m_SocketNotifier->setEnabled(true);
            } else {
                emit logStreamWarning(m_URL, i18n("error no %1", err));
            }
        }
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
    consumed_size = (consumed_size == SIZE_T_DONT_CARE) ? free : qMin(consumed_size, free);
    if (free > size) {
        free = size;
    }
    m_Buffer.addData(data, free);
    if (m_SocketNotifier && m_Buffer.getFillSize() > 0) {
        m_SocketNotifier->setEnabled(true);
    }
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
        if (m_KIO_Job)
            m_KIO_Job->resume();
    }
    if (m_SocketNotifier && m_Buffer.getFreeSize() > 0) {
        m_SocketNotifier->setEnabled(true);
    }
}

void   StreamingJob::slotIOJobResult (KIO::Job *job)
{
    if (job && job->error()) {
        emit logStreamError(m_URL, i18n("Error: %1 (%2)", job->errorString(), job->error()));
    }
}

#include "streaming-job.moc"

