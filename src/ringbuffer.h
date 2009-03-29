/***************************************************************************
                          ringbuffer.h  -  description
                             -------------------
    begin                : Sun March 21 2004
    copyright            : (C) 2004 by Martin Witte
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

#ifndef _KRADIO_RING_BUFFER_H
#define _KRADIO_RING_BUFFER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <kdemacros.h>
#include <QtCore/QSemaphore>

class KDE_EXPORT RingBuffer
{
public:
    RingBuffer(size_t size, bool synchronized = false);
    ~RingBuffer();

    bool       resize(size_t new_size);

    size_t     addData (const char *src, size_t size);
    size_t     takeData(char *dst, size_t size, bool lock = true);

    char      *getFreeSpace(size_t &size);
    size_t     removeFreeSpace(size_t size);

    char      *getData(size_t &size);
    size_t     removeData(size_t size);

    size_t     getSize()     const;
    size_t     getFillSize() const;
    size_t     getFreeSize() const;

    void       clear();

    void       lockTransaction() const;
    void       unlockTransaction() const;

protected:
    void       lock()   const;
    void       unlock() const;

    char      *m_Buffer;
    size_t     m_Start;
    size_t     m_Size,
               m_FillSize;

    bool               m_synchronized;
    mutable QSemaphore m_synchronizer;
    mutable QSemaphore m_transactionSynchronizer;
};

#endif
