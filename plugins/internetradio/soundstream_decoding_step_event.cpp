/***************************************************************************
                          soundstream_decoding_step_event.cpp  -  description
                             -------------------
    begin                : Mon Feb 23 2009
    copyright            : (C) 2009 by Martin Witte
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

#include "soundstream_decoding_step_event.h"

SoundStreamDecodingStepEvent::SoundStreamDecodingStepEvent(const SoundFormat &f, const char *data, size_t size, const SoundMetaData &md)
 :  SoundStreamDecodingEvent(SoundStreamDecodingStep, f),
    m_Size(size),
    m_MetaData(md)
{
    m_Data = new char [m_Size];
    memcpy (m_Data, data, m_Size);
}

SoundStreamDecodingStepEvent::~SoundStreamDecodingStepEvent()
{
    freeData();
}

void SoundStreamDecodingStepEvent::freeData()
{
    if (m_Data) {
        delete m_Data;
    }
    m_Data = NULL;
    m_Size = 0;
}  // _MUST_ be called by event receiver


char *SoundStreamDecodingStepEvent::takeData()
{
    char *tmp = m_Data;
    m_Data = NULL;
    return tmp;
}

const char *SoundStreamDecodingStepEvent::data() const
{
    return m_Data;
}

size_t SoundStreamDecodingStepEvent::size() const
{
    return m_Size;
}

const SoundMetaData &SoundStreamDecodingStepEvent::metaData() const
{
    return m_MetaData;
}


