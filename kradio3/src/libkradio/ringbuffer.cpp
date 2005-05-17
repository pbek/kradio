/***************************************************************************
                          ringbuffer.cpp  -  description
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

#include "ringbuffer.h"
#include <string.h>

RingBuffer::RingBuffer(unsigned size)
{
    m_Buffer   = new char [size];
    m_Size     = size;
    m_FillSize = 0;
    m_Start    = 0;
}


RingBuffer::~RingBuffer()
{
    delete[] m_Buffer;
    m_Buffer = NULL;
    m_Size   = 0;
}


bool RingBuffer::resize(unsigned new_size)
{
    if (new_size >= m_FillSize && new_size > 0) {
        char      *newBuffer = new char[new_size];
        unsigned   newFill = 0;
        while (m_FillSize > 0)
            newFill += takeData(newBuffer + newFill, m_FillSize);

        delete[] m_Buffer;

        m_FillSize = newFill;
        m_Start    = 0;
        m_Buffer   = newBuffer;
        m_Size     = new_size;
        return true;
    }
    return false;
}


unsigned RingBuffer::addData (const char *src, unsigned size)
{
    unsigned written = 0;
    if (m_Start + m_FillSize < m_Size) {
        unsigned rest = m_Size - m_Start - m_FillSize;
        if (rest > size)
            rest = size;
        memmove (m_Buffer + m_Start + m_FillSize, src, rest);
        m_FillSize += rest;
        written    += rest;
        size       -= rest;
        src        += rest;
    }
    if (size > 0 && m_FillSize < m_Size) {
        unsigned rest = size;
        if (rest > m_Size - m_FillSize)
            rest = m_Size - m_FillSize;
        memmove(m_Buffer + m_Start + m_FillSize - m_Size, src, rest);
        m_FillSize += rest;
        written    += rest;
    }
    return written;
}


unsigned RingBuffer::takeData(char *dst, unsigned size)
{
    unsigned read = 0;
    while (m_FillSize > 0 && size > 0) {
        unsigned n = size;
        if (n > m_FillSize)
            n = m_FillSize;
        if (n > m_Size - m_Start)
            n = m_Size - m_Start;
        memmove (dst, m_Buffer + m_Start, n);
        m_FillSize -= n;
        m_Start    += n;
        read       += n;
        size       -= n;
        if (m_Start >= m_Size)
            m_Start -= m_Size;

    }
    return read;
}


char      *RingBuffer::getFreeSpace(unsigned &size)
{
    if (m_FillSize == m_Size) {
        size = 0;
        return NULL;
    }

    if (m_Start + m_FillSize >= m_Size) {
        size = m_Size - m_FillSize;
        return m_Buffer + m_Start + m_FillSize - m_Size;
    } else {
        size = m_Size - m_Start - m_FillSize;
        return m_Buffer + m_Start + m_FillSize;
    }
}


unsigned   RingBuffer::removeFreeSpace(unsigned size)
{
    if (m_FillSize == m_Size)
        return 0;

    if (m_Start + m_FillSize >= m_Size) {
        if (size > m_Size - m_FillSize)
            size = m_Size - m_FillSize;
        m_FillSize += size;
        return size;
    } else {
        if (m_Start + m_FillSize + size >= m_Size)
            size = m_Size - m_Start - m_FillSize;
        m_FillSize += size;
        return size;
    }
}


char      *RingBuffer::getData(unsigned &size)
{
    if (m_Start + m_FillSize >= m_Size) {
        size = m_Size - m_Start;
    } else {
        size = m_FillSize;
    }
    return m_Buffer + m_Start;
}


unsigned   RingBuffer::removeData(unsigned size)
{
    unsigned n = 0;
    if (size > m_FillSize)
        size = m_FillSize;
    if (m_Start + size >= m_Size) {
        n = m_Size - m_Start;
        m_Start = 0;
    } else {
        m_Start += size;
        n = size;
    }
    m_FillSize -= n;
    return n;
}


void RingBuffer::clear()
{
    m_Start    = 0;
    m_FillSize = 0;
}
