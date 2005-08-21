/***************************************************************************
                          encoder_mp3.h
                             -------------------
    begin                : Sat Aug 20 2005
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

#ifndef KRADIO_RECORDING_ENCODER_MP3_H
#define KRADIO_RECORDING_ENCODER_MP3_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "encoder.h"

#ifdef HAVE_LAME_LAME_H
    #include <lame/lame.h>
#endif

class RecordingEncodingMP3 : public RecordingEncoding
{
public:
    RecordingEncodingMP3(QObject *parent, SoundStreamID id, const RecordingConfig &cfg, const RadioStation *rs, const QString &filename);
    virtual ~RecordingEncodingMP3();

    bool               openOutput(const QString &outputFile);
    void               closeOutput();

protected:
    void               encode(const char *_buffer, size_t buffer_size, char *&export_buffer, size_t &export_buffer_size);

#ifdef HAVE_LAME_LAME_H
    unsigned char     *m_MP3Buffer;
    size_t            m_MP3BufferSize;
    FILE              *m_MP3Output;
    char              *m_ID3Tags;
    lame_global_flags *m_LAMEFlags;
    short int         *m_MP3LBuffer,
                      *m_MP3RBuffer;
#endif
};



#endif
