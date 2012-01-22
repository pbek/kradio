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
    m_inputBufferSize   (0),
    m_readPending       (0)
{
}


StreamInputBuffer::~StreamInputBuffer()
{
    resetBuffer();
}



// blocking function if buffer is empty!
QByteArray StreamInputBuffer::readInputBuffer(size_t minSize, size_t maxSize, KUrl &currentUrl_out, bool consume)
{
    bool       isfull        = false;
    bool       resetDetected = false;
    QByteArray retval;

    m_readPending = minSize;

    // block until at least 1 byte is readable
    m_inputBufferSize.acquire(minSize);

    {   QMutexLocker  lock(&m_inputBufferAccessLock);

        // cleanup after possible reset conditions
        int n_suggested = m_inputBufferSize.available() + minSize;
        int n_real      = m_inputBuffer.size();
        if (n_suggested > n_real) {
            m_inputBufferSize.acquire(n_suggested - n_real);
            resetDetected = true;
        }

        QByteArray shared = m_inputBuffer.left(maxSize);

        if (!resetDetected && (size_t)shared.size() > minSize) {
            retval = QByteArray(shared.data(), shared.size()); // force deep copy for threading reasons
            if (consume) {
                m_inputBuffer.remove(0, retval.size());
            }
            isfull = (size_t)m_inputBuffer.size() >= m_inputBufferMaxSize;
        }

        // must be inside locked region in order to guarantee the resetBuffer works correctly
        // keep track of the bytes read
        if (consume) {
            if ((size_t)retval.size() > minSize) {
                m_inputBufferSize.acquire(retval.size() - minSize);
            }
        } else {
            m_inputBufferSize.release(qMin((size_t)retval.size(), minSize));
        }

        // let's hope that this is really a deep copy
        currentUrl_out = m_inputUrl.url();

        m_readPending = 0;
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


void StreamInputBuffer::resetBuffer()
{
    QMutexLocker  lock(&m_inputBufferAccessLock);

    while (m_inputBufferSize.available()) {
        m_inputBufferSize.tryAcquire(m_inputBufferSize.available());
    }
    m_inputBuffer.clear();
    m_inputUrl = KUrl();
    m_inputBufferSize.release(m_readPending); // ensure that a waiting read gets released (they will return 0 bytes)
    emit sigInputBufferNotFull();
}


KUrl StreamInputBuffer::getInputUrl() const
{
    QMutexLocker  lock(&m_inputBufferAccessLock);

    // let's hope that this is really a deep copy
    KUrl currentUrl_out = m_inputUrl.url();
    return currentUrl_out;
}
