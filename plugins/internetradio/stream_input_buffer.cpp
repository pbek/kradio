/***************************************************************************
                          stream_input_buffer.cpp  -  description
                             -------------------
    begin                : Sun Jan 22 2012
    copyright            : (C) 2012 by Martin Witte
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

#include "stream_input_buffer.h"
#include "errorlog_interfaces.h"

#include <klocale.h>

#include <stdio.h>

StreamInputBuffer::StreamInputBuffer(int max_size)
  : m_inputBufferMaxSize (max_size),
    m_inputBufferSize    (0),
    m_readPending        (0),
    m_readPendingReleased(0)
{
}


StreamInputBuffer::~StreamInputBuffer()
{
    resetBuffer();
}



// blocking function if buffer is empty!
QByteArray StreamInputBuffer::readInputBuffer(size_t minSize, size_t maxSize, bool consume, bool &err)
{
    minSize = qMin(minSize, maxSize);

    bool       isfull        = false;
    bool       resetDetected = false;
    QByteArray retval;

    {   QMutexLocker  lock(&m_inputBufferAccessLock);
        m_readPending += minSize;
    }

    // block until at least 1 byte is readable
//     printf ("buffer start read(minsize = %zi, maxsize = %zi) - acquire\n", minSize, maxSize);
    m_inputBufferSize.acquire(minSize);

    {   QMutexLocker  lock(&m_inputBufferAccessLock);
//         printf ("buffer start read(minsize = %zi, maxsize = %zi) - locked\n", minSize, maxSize);

        // new reset detection code
        // we had a reset if (m_readPendingReleased != 0)
        // this means that the semaphore was m_readPendingReleased higher than the buffer size when
        // the acquire above succeeded.
        // For us it means, that the requested minSize is completely unavailable from the buffer.
        if (m_readPendingReleased > 0) {
//             printf ("buffer reset detected\n");
            resetDetected = true;
            if (minSize > m_readPendingReleased) {
                // this should never happen, but let's handle it gracefully:
                m_inputBufferSize.release(minSize - m_readPendingReleased);
                IErrorLogClient::staticLogError(i18n("This should never happen: inconsistency in buffer locking / buffer size!"));
            }
            m_readPendingReleased -= qMin(m_readPendingReleased, minSize);
        }

        // although it should not happen, we need to consider that some other process could
        // be also acquiring concurrently
        size_t realSize = qMin(maxSize, (size_t)m_inputBufferSize.available() + minSize);
//         realSize = qMax(minSize, realSize - realSize % 8); // alignment! Otherwise decoder might fail!
//         printf ("bufferSize = %i      minSize = %zi,  maxSize = %zi       realSize = %zi\n", m_inputBuffer.size(), minSize, maxSize, realSize);
        QByteArray shared = m_inputBuffer.left(realSize);
//         printf ("reading stream input buffer: shared.size = %i\n", shared.size());

        if (!resetDetected && (size_t)shared.size() >= minSize) {
            retval = QByteArray(shared.data(), shared.size()); // force deep copy for threading reasons
            if (consume) {
                m_inputBuffer.remove(0, retval.size());
            }
            isfull = (size_t)m_inputBuffer.size() >= m_inputBufferMaxSize;
        }

        // must be inside locked region in order to guarantee the resetBuffer works correctly
        // keep track of the bytes read

        // we got more data than minSize: thus we need to acquire more
        if ((size_t)retval.size() > minSize) {
            m_inputBufferSize.acquire(retval.size() - minSize);
        }
        // if we don't consume the data, we need to release what we acquired before
        if (!consume && retval.size()) {
            m_inputBufferSize.release(retval.size());
        }

        m_readPending -= minSize;
    }

//     printf ("reading stream input buffer: %i bytes (min = %zi, max = %zi)\n", retval.size(), minSize, maxSize);
    if (!isfull) {
        emit sigInputBufferNotFull();
    }
    err = resetDetected;
    return retval;
}


void StreamInputBuffer::slotWriteInputBuffer(QByteArray data)
{
    bool isFull = false;
//     int oldSize = 0;
    {   QMutexLocker  lock(&m_inputBufferAccessLock);

//         oldSize = m_inputBuffer.size();
        m_inputBuffer.append(data.data(), data.size()); // force deep copy
        isFull = (size_t)m_inputBuffer.size() >= m_inputBufferMaxSize;

        m_inputBufferSize.release(data.size());
    }
    if (isFull) {
        emit sigInputBufferFull();
    }
//     printf ("wrote stream input buffer: %i bytes (oldSize = %i, newSize = %i)\n", data.size(), oldSize, m_inputBuffer.size());
}


void StreamInputBuffer::resetBuffer()
{
    QMutexLocker  lock(&m_inputBufferAccessLock);

//     printf ("reset input buffer\n");

    while (m_inputBufferSize.available()) {
        m_inputBufferSize.tryAcquire(m_inputBufferSize.available());
    }
    m_inputBuffer.clear();
    m_readPendingReleased += m_readPending;
    m_inputBufferSize.release(m_readPending); // ensure that a waiting read gets released (they will return 0 bytes)
    emit sigInputBufferNotFull();
}


