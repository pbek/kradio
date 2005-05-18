/***************************************************************************
                      sound_metadata.h  -  description
                             -------------------
    begin                : Sun May 15 2005
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

#ifndef KRADIO_SOUND_METADATA_H
#define KRADIO_SOUND_METADATA_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <time.h>
#include <endian.h>
#include <qstring.h>
#include <kconfig.h>
#include <kurl.h>

class SoundMetaData
{
public:
    SoundMetaData (Q_UINT64 pos, time_t rel_ts, time_t abs_ts, const KURL &url = KURL())
        : m_DataPosition(pos), m_relativeTimestamp(rel_ts), m_absoluteTimestamp(abs_ts), m_URL(url) {}

    Q_UINT64 position()  const { return m_DataPosition; }
    KURL     url()       const { return m_URL; }
    time_t   relativeTimestamp() const { return m_relativeTimestamp; }
    time_t   absoluteTimestamp() const { return m_absoluteTimestamp; }

protected:
    Q_UINT64 m_DataPosition;
    time_t   m_relativeTimestamp;
    time_t   m_absoluteTimestamp;
    KURL     m_URL;
};


#endif
