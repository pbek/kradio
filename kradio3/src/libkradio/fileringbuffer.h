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

#ifndef _KRADIO_FILE_RING_BUFFER_H
#define _KRADIO_FILE_RING_BUFFER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qstring.h>

class FileRingBuffer
{
public:
    FileRingBuffer(const QString &filename, Q_UINT64 max_size);
    ~FileRingBuffer();

    bool       resize(const QString &filename, Q_UINT64 new_max_size);

    unsigned   addData (const char *src, unsigned size);
    unsigned   takeData(char *dst, unsigned size);
    Q_UINT64   removeData(Q_UINT64 size);

    const QString &getFileName () const { return m_FileName; }
    Q_UINT64       getMaxSize()   const { return m_MaxSize;  }
    Q_UINT64       getRealSize()  const { return m_RealSize; }
    Q_UINT64       getFillSize()  const { return m_FillSize; }
    Q_UINT64       getFreeSize()  const { return (m_Start + m_FillSize > m_RealSize) ? m_RealSize - m_FillSize : m_MaxSize - m_FillSize; }

    void       clear();

    bool       error()       { return m_error; }
    bool       errorString() { return m_errorString; }

protected:
    Q_UINT64    getFreeSpace(Q_UINT64 &size);  // returns position in file + size
    Q_UINT64    removeFreeSpace(Q_UINT64 size);

    Q_UINT64    getData(Q_UINT64 &size);  // returns position in file + size


    int        m_FileIdx;
    QString    m_BaseFileName;
    QString    m_FileName;
    FILE      *m_File;
    Q_UINT64   m_Start;
    Q_UINT64   m_MaxSize;
    Q_UINT64   m_RealSize;
    Q_UINT64   m_FillSize;

    QString   m_errorString;
    bool      m_error;
};

#endif
