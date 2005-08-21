/***************************************************************************
                          encoder_ogg.h
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

#ifndef KRADIO_RECORDING_ENCODER_OGG_H
#define KRADIO_RECORDING_ENCODER_OGG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "encoder.h"

#if defined(HAVE_VORBIS_VORBISENC_H) && defined(HAVE_OGG_OGG_H)
    #include <vorbis/vorbisenc.h>
#endif

class RecordingEncodingOgg : public RecordingEncoding
{
public:
    RecordingEncodingOgg(QObject *parent, SoundStreamID id, const RecordingConfig &cfg, const RadioStation *rs, const QString &filename);
    virtual ~RecordingEncodingOgg();

    bool               openOutput(const QString &outputFile);
    void               closeOutput();

protected:
    void               encode(const char *_buffer, size_t buffer_size, char *&export_buffer, size_t &export_buffer_size);

#if defined(HAVE_VORBIS_VORBISENC_H) && defined(HAVE_OGG_OGG_H)
    FILE              *m_OggOutput;
    char              *m_OggExportBuffer;
    size_t             m_OggExportBufferSize;
    ogg_stream_state   m_OggStream;
    vorbis_dsp_state   m_VorbisDSP;
    vorbis_block       m_VorbisBlock;
    vorbis_info        m_VorbisInfo;
#endif
};


#endif
