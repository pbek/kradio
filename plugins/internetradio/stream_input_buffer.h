/***************************************************************************
                          stream_input_buffer.h  -  description
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

#ifndef KRADIO_STREAM_INPUT_BUFFER_H
#define KRADIO_STREAM_INPUT_BUFFER_H


#include <QtCore/QObject>
#include <QtCore/QSemaphore>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

#include <kurl.h>


class StreamInputBuffer : public QObject
{
Q_OBJECT
public:
    StreamInputBuffer(int max_size);
    ~StreamInputBuffer();

    void                  resetBuffer();

    void                  writeInputBuffer(const QByteArray &data, bool &isFull, const KUrl &inputUrl);
    QByteArray            readInputBuffer(size_t minSize, size_t maxSize, KUrl &currentUrl_out, bool consume);
    KUrl                  getInputUrl() const;

signals:
    // connects with this signal need to be Qt::QueuedConnection in order to avoid race conditions when suspending/waking up writing thread
    void                  sigInputBufferNotFull();

protected:

    void                  clearReset();
    
    size_t                m_inputBufferMaxSize;
    QByteArray            m_inputBuffer;
    mutable QMutex        m_inputBufferAccessLock;
    QSemaphore            m_inputBufferSize;
    KUrl                  m_inputUrl;

    size_t                m_readPending;
    size_t                m_readPendingReleased;
};


#endif


