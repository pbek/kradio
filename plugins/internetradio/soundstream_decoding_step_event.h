/***************************************************************************
                          soundstream_decoding_step_event.h  -  description
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

#ifndef KRADIO_INTERNETRADIO_SOUNDSTREAM_DECODING_STEP_EVENT_H
#define KRADIO_INTERNETRADIO_SOUNDSTREAM_DECODING_STEP_EVENT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "soundstream_decoding_event.h"

class SoundStreamDecodingStepEvent : public SoundStreamDecodingEvent
{
public:
    SoundStreamDecodingStepEvent(const SoundFormat &f, const char *data, size_t size, const SoundMetaData &md);
    virtual ~SoundStreamDecodingStepEvent();

    void                 freeData();
    char                *takeData();
    const char          *data()     const;
    size_t               size()     const;
    const SoundMetaData &metaData() const;

    static bool          isSoundStreamDecodingStep (const QEvent *e);

protected:
    char         *m_Data;
    size_t        m_Size;
    SoundMetaData m_MetaData;
};

#endif
