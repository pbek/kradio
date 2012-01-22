/***************************************************************************
                          databuffer.h  -  description
                             -------------------
    begin                : Feb 2009
    copyright            : (C) 2002-2009 Ernst Martin Witte
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

#ifndef KRADIO_INTERNETRADIO_DATABUFFER_H
#define KRADIO_INTERNETRADIO_DATABUFFER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include "soundformat.h"
#include "sound_metadata.h"

class DataBuffer {
    public:
        DataBuffer();
        // this constructor creates a copy of the data!
        DataBuffer(const char *data, size_t size, const SoundMetaData &md, const SoundFormat &);
        // copy constructor does not copy the data!
        DataBuffer(const DataBuffer &b);
        // the m_data field is not freed automatically!!!
        ~DataBuffer();

        void                 freeData(); // must be called MANUALL. Destructor will not delete data!

        bool                 isValid() const;

        char   *             currentPointer() const;
        size_t               remainingSize()  const;
        void                 addProcessedSize(size_t s);

        const SoundMetaData &metaData()    const { return m_MetaData; }
        const SoundFormat   &soundFormat() const { return m_SoundFormat; }

    protected:
        char                *m_Data;
        size_t               m_Size;
        size_t               m_processedSize;
        SoundMetaData        m_MetaData;
        SoundFormat          m_SoundFormat;
    };


#endif
