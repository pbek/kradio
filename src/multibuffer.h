/***************************************************************************
                          multibuffer.h
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

#ifndef KRADIO_MULTIBUFFER_H
#define KRADIO_MULTIBUFFER_H

#include <QtCore/QSemaphore>
#include <QtCore/QString>

#include "kradio-def.h"

class KRADIO5_EXPORT MultiBuffer
{
public:
    MultiBuffer(size_t n_buffers, size_t buffersize);
    ~MultiBuffer();

    char *lockWriteBuffer     (size_t &bufferSize);
    bool  unlockWriteBuffer   (size_t bufferSize); // return value: complete buffer full / ready for read
    void  unlockAllWriteBuffers();
    char *wait4ReadBuffer     (size_t &buffer_fill);
    char *getCurrentReadBuffer(size_t &buffer_fill) const;

    const QString &getErrorString() const { return m_errorString; }
    bool           hasError()       const { return m_error; }
    void           resetError();

    size_t  getWriteBufferFill()  const { return (m_currentReadBuffer != m_currentWriteBuffer) ? m_buffersFill[m_currentWriteBuffer] : 0; }
    size_t  getAvailableWriteBuffer()  const;
    size_t  getAvailableReadBuffers()   const;
    size_t  getCurrentReadBufferIdx()  const { return m_currentReadBuffer; }
    size_t  getCurrentWriteBufferIdx() const { return m_currentWriteBuffer; }

protected:

    size_t             m_nBuffers;
    size_t             m_BufferSize;

    char             **m_buffers;
    size_t            *m_buffersFill;
    size_t             m_currentReadBuffer;
    size_t             m_currentWriteBuffer;
    QSemaphore         m_readLock;

    QString            m_errorString;
    bool               m_error;
};

#endif
