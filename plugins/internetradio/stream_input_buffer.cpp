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

StreamInputBuffer::StreamInputBuffer(int max_size)
  : m_inputBufferMaxSize(max_size),
    m_inputBufferSize(0)
{
}


// blocking function if buffer is empty!
QByteArray StreamInputBuffer::readInputBuffer(size_t maxSize, KUrl &currentUrl_out)
{
    bool       isfull = false;
    QByteArray retval;

    // block until at least 1 byte is readable
    m_inputBufferSize.acquire(1);

    {   QMutexLocker  lock(&m_inputBufferAccessLock);

        QByteArray shared = m_inputBuffer.left(maxSize);
        retval = QByteArray(shared.data(), shared.size()); // force deep copy for threading reasons
        m_inputBuffer.remove(0, retval.size());
        isfull = (size_t)m_inputBuffer.size() >= m_inputBufferMaxSize;

        // must be inside locked region in order to guarantee the clearBuffer works correctly
        // keep track of the bytes read
        if (retval.size() > 0) {
            m_inputBufferSize.acquire(retval.size() - 1); // -1 since we already aquired 1 byte above
        }

        // let's hope that this is really a deep copy
        currentUrl_out = m_inputUrl.url();
    }

    if (!isfull) {
        emit sigInputBufferNotFull();
    }
    return retval;
}


void StreamInputBuffer::writeInputBuffer(const QByteArray &data, bool &isFull, const KUrl &inputUrl)
{
    QMutexLocker  lock(&m_inputBufferAccessLock);

    m_inputBuffer.append(data.data(), data.size()); // force deep copy
    isFull     = (size_t)m_inputBuffer.size() >= m_inputBufferMaxSize;
    m_inputUrl = inputUrl;
    m_inputBufferSize.release(data.size());
}


void StreamInputBuffer::clearBuffer()
{
    QMutexLocker  lock(&m_inputBufferAccessLock);

    while (m_inputBufferSize.available()) {
        m_inputBufferSize.tryAcquire(m_inputBufferSize.available());
    }
    m_inputBuffer.clear();
    m_inputUrl = KUrl();
    emit sigInputBufferNotFull();
}

