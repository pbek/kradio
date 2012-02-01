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
  : m_processedSize(0),
    m_MetaData(0,0,0)
{
}


DataBuffer::DataBuffer(size_t reservedSize, const char *data, size_t dataSize, const SoundMetaData &md, const SoundFormat &sf)
  : m_data(data, dataSize),
    m_processedSize(0),
    m_MetaData(md),
    m_SoundFormat(sf)
{
    m_data.reserve(reservedSize);
}


DataBuffer::DataBuffer(const DataBuffer &b)
  : m_data(b.m_data),
    m_processedSize(b.m_processedSize),
    m_MetaData(b.m_MetaData),
    m_SoundFormat(b.m_SoundFormat)
{
}


DataBuffer::~DataBuffer()
{
    m_processedSize = 0;
}


bool DataBuffer::isValid() const
{
    return m_data.capacity() > 0;
}


void DataBuffer::addData(const char* data, size_t data_size)
{
    m_data.append(data, data_size);
}


void DataBuffer::addProcessedSize(size_t s) {
    m_processedSize += s;
}
