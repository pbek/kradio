/***************************************************************************
                          databuffer.cpp  -  description
                             -------------------
    begin                : Mon Feb 23 CET 2009
    copyright            : (C) 2009 by Ernst Martin Witte
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "databuffer.h"

DataBuffer::DataBuffer()
  : m_Data(NULL),
    m_Size(0),
    m_processedSize(0),
    m_MetaData(0,0,0)
{
}

DataBuffer::DataBuffer(const char *data, size_t size, const SoundMetaData &md, const SoundFormat &sf)
  : m_Data(NULL),
    m_Size(size),
    m_processedSize(0),
    m_MetaData(md),
    m_SoundFormat(sf)
{
    m_Data = new char [m_Size];
    memcpy (m_Data, data, m_Size);
}

DataBuffer::DataBuffer(const DataBuffer &b)
  : m_Data(b.m_Data),
    m_Size(b.m_Size),
    m_processedSize(b.m_processedSize),
    m_MetaData(b.m_MetaData),
    m_SoundFormat(b.m_SoundFormat)
{
}

DataBuffer::~DataBuffer()
{
    m_Data          = NULL;
    m_Size          = 0;
    m_processedSize = 0;
}


void DataBuffer::freeData() // must be called MANUALL. Destructor will not delete data!
{
    if (m_Data) {
        delete m_Data;
    }
    m_Data          = NULL;
    m_Size          = 0;
    m_processedSize = 0;
}


bool DataBuffer::isValid() const
{
    return m_Data;
}

char *DataBuffer::currentPointer() const
{
    return m_Data ? m_Data + m_processedSize : NULL;
}

size_t DataBuffer::remainingSize() const
{
    return m_Data ? m_Size - m_processedSize : 0;
}

void DataBuffer::addProcessedSize(size_t s) {
    m_processedSize += s;
}
