/***************************************************************************
                          ringbuffer.cpp  -  description
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

#include "fileringbuffer.h"

#include <QtCore/QString>
#include <QtCore/QFile>
#include <unistd.h>
#include <klocale.h>

FileRingBuffer::FileRingBuffer(const QString &filename, quint64 max_size)
{
    m_BaseFileName = filename;
    m_FileIdx  = 0;
    m_FileName = m_BaseFileName + "_" + QString::number(++m_FileIdx);
    m_File     = fopen(QFile::encodeName(m_FileName), "w+");
    m_MaxSize  = max_size;
    m_RealSize = 0;
    m_FillSize = 0;
    m_Start    = 0;
    m_error    = m_File == NULL;
    m_errorString = m_File ? QString() : i18n("cannot open buffer file %1", m_FileName);
}


FileRingBuffer::~FileRingBuffer()
{
    if (m_File) {
        fclose (m_File);
        unlink (QFile::encodeName(m_FileName));
    }
    m_File   = NULL;
    m_FileName = QString::null;
    m_MaxSize  = 0;
    m_RealSize = 0;
    m_FillSize = 0;
    m_Start    = 0;
    m_error    = false;
    m_errorString = QString();
}


bool FileRingBuffer::resize(const QString &filename, quint64 new_max_size)
{
    if (filename != m_BaseFileName) {
        clear();
        if (m_File) {
            fclose (m_File);
            unlink (QFile::encodeName(m_FileName));
        }
        m_BaseFileName = filename;
        m_FileName = m_BaseFileName + "_" + QString::number(++m_FileIdx);
        m_File     = fopen(QFile::encodeName(m_FileName), "w+");
        m_error    = m_File == NULL;
        m_errorString = m_File ? QString() : i18n("cannot open buffer file %1", m_FileName);
    }

    if (new_max_size >= m_RealSize) {
        m_MaxSize = new_max_size;
    }
    else if ((m_Start + m_FillSize < m_RealSize) && (new_max_size > m_Start + m_FillSize)) {
        ftruncate(fileno(m_File), new_max_size);
        m_MaxSize = new_max_size;
    }
    else if (new_max_size >= m_FillSize) {
        const size_t buffer_size = 65536;
        char           buffer[buffer_size];

        QString tmp_file_name = m_BaseFileName + "_" + QString::number(++m_FileIdx);
        FILE *tmp_file = fopen (QFile::encodeName(tmp_file_name), "w+");
        quint64  newFill = 0;
        if (tmp_file) {
            while (!m_error && m_FillSize > 0) {
                int tmp_size = takeData(buffer, buffer_size);
                if (tmp_size > 0) {
                    if (fwrite (buffer, tmp_size, 1, tmp_file) > 0) {
                        newFill += tmp_size;
                    } else {
                        m_error = true;
                        m_errorString += i18n("FileRingbuffer::resize: Writing to temporary file %1 failed. ", tmp_file_name);
                    }
                }
            }
        } else {
            m_error = true;
            m_errorString += i18n("FileRingbuffer::resize: Opening temporary file %1 failed. ", tmp_file_name);
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


size_t FileRingBuffer::addData (const char *src, size_t size)
{
    size_t written = 0;
    if (!m_error && m_Start + m_FillSize <= m_RealSize) {
        quint64 rest = m_MaxSize - (m_Start + m_FillSize);
        if (rest > size)
            rest = size;
        fseek(m_File, m_Start + m_FillSize, SEEK_SET);
        if (rest > 0 && fwrite(src, rest, 1, m_File) <= 0) {
            m_error = true;
            m_errorString += i18n("FileRingBuffer::addData: failed writing data to file %1.", m_FileName);
        } else {
            m_FillSize += rest;
            if (m_Start + m_FillSize > m_RealSize)
                m_RealSize = m_Start + m_FillSize;
            written    += rest;
            size       -= rest;
            src        += rest;
        }
    }
    if (!m_error && size > 0 && m_FillSize < m_RealSize) {
        size_t rest = size;
        if (rest > m_RealSize - m_FillSize)
            rest = m_RealSize - m_FillSize;

        fseek(m_File, m_Start + m_FillSize - m_RealSize, SEEK_SET);
        if (fwrite(src, rest, 1, m_File) <= 0) {
            m_error = true;
            m_errorString += i18n("FileRingBuffer::addData: failed writing data to file %1.", m_FileName);
        } else {
            m_FillSize += rest;
            written    += rest;
            //fflush(m_File); // debug only
        }
    }
    return written;
}


size_t FileRingBuffer::takeData(char *dst, size_t size)
{
    size_t read = 0;
    while (!m_error && m_FillSize > 0 && size > 0) {
        size_t n = size;
        if (n > m_FillSize)
            n = m_FillSize;
        if (n > m_RealSize - m_Start)
            n = m_RealSize - m_Start;
        fseek(m_File, m_Start, SEEK_SET);
        if (fread(dst+read, n, 1, m_File) <= 0) {
            m_error = true;
            m_errorString += i18n("FileRingBuffer::takeData: failed reading data to file %1.", m_FileName);
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


quint64 FileRingBuffer::getFreeSpace(quint64 &size)
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


quint64   FileRingBuffer::removeFreeSpace(quint64 size)
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


quint64 FileRingBuffer::getData(quint64 &size)
{
    if (m_Start + m_FillSize >= m_RealSize) {
        size = m_RealSize - m_Start;
    } else {
        size = m_FillSize;
    }
    return m_Start;
}


quint64 FileRingBuffer::removeData(quint64 size)
{
    if (size > m_FillSize)
        size = m_FillSize;
    if (m_Start + size >= m_RealSize) {
        m_Start = m_Start + size - m_RealSize;
    } else {
        m_Start += size;
    }
    m_FillSize -= size;
    return size;
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
