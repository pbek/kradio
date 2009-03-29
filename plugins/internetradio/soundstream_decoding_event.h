/***************************************************************************
                          soundstream_decoding_event.h  -  description
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

#ifndef KRADIO_INTERNETRADIO_SOUNDSTREAM_DECODING_EVENT_H
#define KRADIO_INTERNETRADIO_SOUNDSTREAM_DECODING_EVENT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QtCore/QEvent>

#include "sound_metadata.h"
#include "soundformat.h"

const QEvent::Type SoundStreamDecodingTerminated = (QEvent::Type)(QEvent::User+100);
const QEvent::Type SoundStreamDecodingStep       = (QEvent::Type)(QEvent::User+101);

class SoundStreamDecodingEvent : public QEvent
{
public:
    SoundStreamDecodingEvent(QEvent::Type t, const SoundFormat &f) : QEvent(t), m_SoundFormat(f) {}

    const SoundFormat   &getSoundFormat()   const { return m_SoundFormat; }

    static bool          isSoundStreamDecodingEvent (const QEvent *e) { return e && ((e->type() == SoundStreamDecodingTerminated) || (e->type() == SoundStreamDecodingStep)); }

protected:
    SoundFormat   m_SoundFormat;
};

#endif
