/***************************************************************************
                          soundstreamevent.h  -  description
                             -------------------
    begin                : Fri May 06 2005
    copyright            : (C) 2005 by Martin Witte
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

#ifndef KRADIO_RECORDING_SOUNDSTREAM_EVENT_H
#define KRADIO_RECORDING_SOUNDSTREAM_EVENT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qevent.h>

#include "../../src/libkradio/sound_metadata.h"

const QEvent::Type EncodingTerminated = (QEvent::Type)(QEvent::User+1);
const QEvent::Type EncodingStep       = (QEvent::Type)(QEvent::User+2);

class SoundStreamEvent : public QEvent
{
public:
    SoundStreamEvent(QEvent::Type t, SoundStreamID id) : QEvent(t), m_SSID(id) {}
    const SoundStreamID &getSoundStreamID() const { return m_SSID; }

    static bool isSoundStreamEvent (const QEvent *e) { return e && ((e->type() == EncodingTerminated) || (e->type() == EncodingStep)); }

protected:
    SoundStreamID m_SSID;
};






class SoundStreamEncodingTerminatedEvent : public SoundStreamEvent
{
public:
    SoundStreamEncodingTerminatedEvent(SoundStreamID id) : SoundStreamEvent(EncodingTerminated, id) {}
};






class SoundStreamEncodingStepEvent : public SoundStreamEvent
{
public:
    SoundStreamEncodingStepEvent(SoundStreamID id, const char *data, size_t size, const SoundMetaData &md)
         : SoundStreamEvent(EncodingStep, id),
           m_Size(size),
           m_MetaData(md)
    {
        m_Data = new char [m_Size];
        memcpy (m_Data, data, m_Size);
    }
    virtual ~SoundStreamEncodingStepEvent() { freeData(); }

    void freeData() { if (m_Data) delete m_Data; m_Data = NULL; m_Size = 0; }  // _MUST_ be called by event receiver

    const char  *data() const { return m_Data; }
    size_t       size() const { return m_Size; }
    const SoundMetaData &metaData()  const { return m_MetaData; }

    static bool isSoundStreamEncodingStep (const QEvent *e) { return e && (e->type() == EncodingStep); }

protected:
    char         *m_Data;
    size_t        m_Size;
    SoundMetaData m_MetaData;
};

#endif
