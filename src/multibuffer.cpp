/***************************************************************************
                          multibuffer.cpp
                             -------------------
    begin                : Sat Aug 20 2005
    copyright            : (C) 2005 by Martin Witte
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

#include <klocalizedstring.h>

#include "multibuffer.h"

MultiBuffer::MultiBuffer(size_t n_buffers, size_t buffersize)
    : m_nBuffers(n_buffers),
      m_BufferSize(buffersize),
      m_currentReadBuffer(m_nBuffers-1), // during wait4read, this will be incremented to 0
      m_currentWriteBuffer(0),
      m_readLock(n_buffers),
      m_errorString(QString::null),
      m_error(false)
{
    m_readLock.acquire(m_nBuffers);

    m_buffers = new char* [m_nBuffers];
    m_buffersFill = new size_t [m_nBuffers];
    for (size_t i = 0; i < m_nBuffers; ++i) {
        m_buffers    [i] = new char [m_BufferSize];
        m_buffersFill[i] = 0;
    }
}

MultiBuffer::~MultiBuffer()
{
    for (size_t i = 0; i < m_nBuffers; ++i) {
        delete m_buffers[i];
    }
    delete m_buffers;
    delete m_buffersFill;
    m_buffersFill = NULL;
    m_buffers = NULL;
}

size_t MultiBuffer::getAvailableWriteBuffer() const
{
    size_t bytesAvailable = m_BufferSize - m_buffersFill[m_currentWriteBuffer];
    return m_currentWriteBuffer != m_currentReadBuffer ? bytesAvailable : 0;
}

size_t MultiBuffer::getAvailableReadBuffers() const
{
    return m_readLock.available();
}

char *MultiBuffer::lockWriteBuffer(size_t &bufferSize)
{
    size_t bytesAvailable = m_BufferSize - m_buffersFill[m_currentWriteBuffer];

    if (m_currentWriteBuffer != m_currentReadBuffer && bytesAvailable > 0) {
        bufferSize = bytesAvailable;
        return m_buffers[m_currentWriteBuffer] + m_buffersFill[m_currentWriteBuffer];
    }
/*    QString tmp;
    IErrorLogClient::staticLogDebug(tmp.sprintf("current input buffer: %li", m_currentInputBuffer));
    IErrorLogClient::staticLogDebug(tmp.sprintf("inputAvailableLock: %i",   m_inputAvailableLock.available()));
    for (size_t i = 0; i < m_config.m_EncodeBufferCount; ++i) {
        IErrorLogClient::staticLogDebug(tmp.sprintf("input buffer %li: fill = %li", i, m_buffersInputFill[i]));
    }
*/
/*    m_error = true;
    m_errorString += i18n("Buffer Overflow. ");*/
    return NULL;
}


bool  MultiBuffer::unlockWriteBuffer(size_t bufferSize) // return value: complete buffer ready for read
{
    bool retval = false;
    if (m_buffersFill[m_currentWriteBuffer] + bufferSize > m_BufferSize) {
        m_error = true;
        m_errorString += i18n("Buffer Overflow. ");
    } else if (bufferSize > 0) {
        m_buffersFill[m_currentWriteBuffer] += bufferSize;

        if (m_buffersFill[m_currentWriteBuffer] == m_BufferSize) {
            m_currentWriteBuffer = (m_currentWriteBuffer+1 < m_nBuffers) ? m_currentWriteBuffer + 1 : 0;
            m_readLock.release();
            retval = true;
        }
    }
    return retval;
}


void  MultiBuffer::unlockAllWriteBuffers()
{
    m_currentWriteBuffer = m_currentReadBuffer;
    // there are at maximum m_nBuffers - 1 full buffers. The nth buffer is the current read buffer
    m_readLock.release(m_nBuffers - 1 - m_readLock.available());
}


char *MultiBuffer::wait4ReadBuffer(size_t &buffer_fill)
{
    m_buffersFill[m_currentReadBuffer] = 0; // mark buffer as empty again
    m_readLock.acquire();
    m_currentReadBuffer = (m_currentReadBuffer+1 < m_nBuffers) ? m_currentReadBuffer + 1 : 0;
    buffer_fill = m_buffersFill[m_currentReadBuffer];
    return m_buffers[m_currentReadBuffer];
}


char *MultiBuffer::getCurrentReadBuffer(size_t &buffer_fill) const
{
    buffer_fill = m_buffersFill[m_currentReadBuffer];
    return m_buffers[m_currentReadBuffer];
}

void MultiBuffer::resetError()
{
    m_error = false;
    m_errorString = QString::null;
}
