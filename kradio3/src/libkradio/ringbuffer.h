/***************************************************************************
                          ringbuffer.h  -  description
                             -------------------
    begin                : Sun March 21 2004
    copyright            : (C) 2004 by Martin Witte
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

#ifndef _KRADIO_RING_BUFFER_H
#define _KRADIO_RING_BUFFER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>

class RingBuffer
{
public:
    RingBuffer(size_t size);
    ~RingBuffer();

    bool       resize(size_t new_size);

    size_t   addData (const char *src, size_t size);
    size_t   takeData(char *dst, size_t size);

    char      *getFreeSpace(size_t &size);
    size_t     removeFreeSpace(size_t size);

    char      *getData(size_t &size);
    size_t     removeData(size_t size);

    size_t    getSize()     const { return m_Size; }
    size_t    getFillSize() const { return m_FillSize; }
    size_t    getFreeSize() const { return m_Size - m_FillSize; }

    void       clear();

protected:

    char     *m_Buffer;
    size_t    m_Start;
    size_t    m_Size,
              m_FillSize;
};

#endif
