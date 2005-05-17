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

class RingBuffer
{
public:
    RingBuffer(unsigned size);
    ~RingBuffer();

    bool       resize(unsigned new_size);

    unsigned   addData (const char *src, unsigned size);
    unsigned   takeData(char *dst, unsigned size);

    char      *getFreeSpace(unsigned &size);
    unsigned   removeFreeSpace(unsigned size);

    char      *getData(unsigned &size);
    unsigned   removeData(unsigned size);

    unsigned   getSize()     const { return m_Size; }
    unsigned   getFillSize() const { return m_FillSize; }
    unsigned   getFreeSize() const { return m_Size - m_FillSize; }

    void       clear();

protected:

    char     *m_Buffer;
    unsigned  m_Start;
    unsigned  m_Size,
              m_FillSize;
};

#endif
