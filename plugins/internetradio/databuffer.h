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
        DataBuffer(size_t reserved_size, const char *data, size_t data_size, const SoundMetaData &md, const SoundFormat &);
        // copy constructor uses implicit sharing in QByteArray
        DataBuffer(const DataBuffer &b);
        ~DataBuffer();

        bool                 isValid() const;

        // adds a copy of the data to the buffer
        void                 addData(const char *data, size_t data_size);

        QByteArray           remainingData()     const { return m_data.mid(m_processedSize);       }
        size_t               remainingSize()     const { return m_data.size() - m_processedSize;   }
        size_t               remainingCapacity() const { return m_data.capacity() - m_data.size(); }
        size_t               fullSize()          const { return m_data.size();                     }
        size_t               processedSize()     const { return m_processedSize; }
        void                 addProcessedSize(size_t s);

        const SoundMetaData &metaData()    const { return m_MetaData; }
        const SoundFormat   &soundFormat() const { return m_SoundFormat; }

    protected:
        QByteArray           m_data;
        size_t               m_processedSize;
        SoundMetaData        m_MetaData;
        SoundFormat          m_SoundFormat;
    };


#endif
