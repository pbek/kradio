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

#ifndef _KRADIO_FILE_RING_BUFFER_H
#define _KRADIO_FILE_RING_BUFFER_H

#include <kdemacros.h>
#include <QtCore/QString>
#include <stdio.h>

class KDE_EXPORT FileRingBuffer
{
public:
    FileRingBuffer(const QString &filename, quint64 max_size);
    ~FileRingBuffer();

    bool       resize(const QString &filename, quint64 new_max_size);

    size_t     addData (const char *src, size_t size);
    size_t     takeData(char *dst, size_t size);
    quint64    removeData(quint64 size);

    const QString &getFileName() const { return m_FileName; }
    quint64        getMaxSize () const { return m_MaxSize;  }
    quint64        getRealSize() const { return m_RealSize; }
    quint64        getFillSize() const { return m_FillSize; }
    quint64        getFreeSize() const { return (m_Start + m_FillSize > m_RealSize) ? m_RealSize - m_FillSize : m_MaxSize - m_FillSize; }

    void           clear();

    bool           error()       const { return m_error; }
    const QString &errorString() const { return m_errorString; }

protected:
    quint64     getFreeSpace(quint64 &size);  // returns position in file + size
    quint64     removeFreeSpace(quint64 size);

    quint64     getData(quint64  &size);  // returns position in file + size


    int        m_FileIdx;
    QString    m_BaseFileName;
    QString    m_FileName;
    FILE      *m_File;
    quint64    m_Start;
    quint64    m_MaxSize;
    quint64    m_RealSize;
    quint64    m_FillSize;

    QString   m_errorString;
    bool      m_error;
};

#endif
