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

#include "fileringbuffer.h"

#include <qstring.h>

FileRingBuffer::FileRingBuffer(const QString &filename, Q_UINT64 max_size)
{
    m_BaseFileName = filename;
    m_FileIdx  = 0;
    m_FileName = m_BaseFileName + "_" + QString::number(++m_FileIdx);
    m_File     = fopen(m_FileName, "w+");
    m_MaxSize  = max_size;
    m_RealSize = 0;
    m_FillSize = 0;
    m_Start    = 0;
    m_error    = m_File == NULL;
    m_errorString = m_File ? QString::null : "cannot open buffer file " + filename;
}


FileRingBuffer::~FileRingBuffer()
{
    if (m_File) {
        fclose (m_File);
        unlink (m_FileName);
    }
    m_File   = NULL;
    m_FileName = QString::null;
    m_MaxSize  = 0;
    m_RealSize = 0;
    m_FillSize = 0;
    m_Start    = 0;
    m_error    = false;
    m_errorString = QString::null;
}


bool FileRingBuffer::resize(const QString &filename, Q_UINT64 new_max_size)
{
    if (filename != m_BaseFileName) {
        clear();
        if (m_File) {
            fclose (m_File);
            unlink (m_FileName);
        }
        m_BaseFileName = filename;
        m_FileName = m_BaseFileName + "_" + QString::number(++m_FileIdx);
        m_File     = fopen(m_FileName, "w+");
        m_error    = m_File == NULL;
        m_errorString = m_File ? QString::null : "cannot open buffer file " + filename;
    }

    if (new_max_size >= m_RealSize) {
        m_MaxSize = new_max_size;
    }
    else if (m_Start + m_FillSize < m_RealSize && new_max_size > m_Start + m_FillSize) {
        ftruncate(fileno(m_File), new_max_size);
        m_MaxSize = new_max_size;
    }
    else if (new_max_size >= m_FillSize) {
        const unsigned buffer_size = 65536;
        char           buffer[buffer_size];

        QString tmp_file_name = m_BaseFileName + "_" + QString::number(++m_FileIdx);
        FILE *tmp_file = fopen (tmp_file_name, "w+");
        Q_UINT64  newFill = 0;
        if (tmp_file) {
            while (!m_error && m_FillSize > 0) {
                int tmp_size = takeData(buffer, buffer_size);
                if (tmp_size > 0) {
                    if (fwrite (buffer, tmp_size, 1, tmp_file) > 0) {
                        newFill += tmp_size;
                    } else {
                        m_error = true;
                        m_errorString += "FileRingbuffer::resize: Writing to tmpfile failed. ";
                    }
                }
            }
        } else {
            m_error = true;
            m_errorString += "FileRingbuffer::resize: Opening tmpfile failed. ";
        }

        if (!m_error) {
            fclose (m_File);
            m_FileName = tmp_file_name;
            m_File     = tmp_file;
            m_FillSize = newFill;
            m_Start    = 0;
            m_MaxSize  = new_max_size;
            m_RealSize = newFill;
        }
        return true;
    }
    return false;
}


unsigned FileRingBuffer::addData (const char *src, unsigned size)
{
    unsigned written = 0;
    if (m_Start + m_FillSize <= m_RealSize) {
        Q_UINT64 rest = m_MaxSize - (m_Start + m_FillSize);
        if (rest > size)
            rest = size;
        fseek(m_File, m_Start + m_FillSize, SEEK_SET);
        if (fwrite(src, rest, 1, m_File) <= 0) {
            m_error = true;
            m_errorString += "FileRingBuffer::addData: failed writing data to file " + m_FileName + ". ";
        } else {
            m_FillSize += rest;
            if (m_Start + m_FillSize > m_RealSize)
                m_RealSize = m_Start + m_FillSize;
            written    += rest;
            size       -= rest;
            src        += rest;
        }
    }
    if (size > 0 && m_FillSize < m_RealSize) {
        unsigned rest = size;
        if (rest > m_RealSize - m_FillSize)
            rest = m_RealSize - m_FillSize;

        fseek(m_File, m_Start + m_FillSize - m_RealSize, SEEK_SET);
        if (fwrite(src, rest, 1, m_File) <= 0) {
            m_error = true;
            m_errorString += "FileRingBuffer::addData: failed writing data to file " + m_FileName  + ". ";
        } else {
            m_FillSize += rest;
            written    += rest;
        }
    }
    return written;
}


unsigned FileRingBuffer::takeData(char *dst, unsigned size)
{
    unsigned read = 0;
    while (!m_error && m_FillSize > 0 && size > 0) {
        unsigned n = size;
        if (n > m_FillSize)
            n = m_FillSize;
        if (n > m_RealSize - m_Start)
            n = m_RealSize - m_Start;
        fseek(m_File, m_Start, SEEK_SET);
        if (fread(dst, n, 1, m_File) <= 0) {
            m_error = true;
            m_errorString += "FileRingBuffer::takeData: failed reading data to file " + m_FileName + ". ";
        } else {
            m_FillSize -= n;
            m_Start    += n;
            read       += n;
            size       -= n;
            if (m_Start >= m_RealSize)
                m_Start -= m_RealSize;
        }

    }
    return read;
}


Q_UINT64 FileRingBuffer::getFreeSpace(Q_UINT64 &size)
{
    if (m_FillSize == m_RealSize) {
        size = 0;
        return 0;
    }

    if (m_Start + m_FillSize >= m_RealSize) {
        size = m_RealSize - m_FillSize;
        return m_Start + m_FillSize - m_RealSize;
    } else {
        size = m_MaxSize - m_Start - m_FillSize;
        return m_Start + m_FillSize;
    }
}


Q_UINT64   FileRingBuffer::removeFreeSpace(Q_UINT64 size)
{
    if (m_FillSize == m_RealSize)
        return 0;

    if (m_Start + m_FillSize >= m_RealSize) {
        if (size > m_RealSize - m_FillSize)
            size = m_RealSize - m_FillSize;
        m_FillSize += size;
        return size;
    } else {
        if (m_Start + m_FillSize + size >= m_MaxSize)
            size = m_MaxSize - m_Start - m_FillSize;
        m_FillSize += size;
        return size;
    }
}


Q_UINT64 FileRingBuffer::getData(Q_UINT64 &size)
{
    if (m_Start + m_FillSize >= m_RealSize) {
        size = m_RealSize - m_Start;
    } else {
        size = m_FillSize;
    }
    return m_Start;
}


Q_UINT64 FileRingBuffer::removeData(Q_UINT64 size)
{
    Q_UINT64 n = 0;
    if (size > m_FillSize)
        size = m_FillSize;
    if (m_Start + size >= m_RealSize) {
        n = m_RealSize - m_Start;
        m_Start = 0;
    } else {
        m_Start += size;
        n = size;
    }
    m_FillSize -= n;
    return n;
}


void FileRingBuffer::clear()
{
    if (!m_error) {
        ftruncate(fileno(m_File), 0);
        m_Start    = 0;
        m_FillSize = 0;
        m_RealSize = 0;
    }
}
