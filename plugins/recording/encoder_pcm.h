/***************************************************************************
                          encoder_pcm.h
                             -------------------
    begin                : Sat Aug 20 2005
    copyright            : (C) 2005 by Martin Witte
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

#ifndef KRADIO_RECORDING_ENCODER_PCM_H
#define KRADIO_RECORDING_ENCODER_PCM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "encoder.h"

#include <sndfile.h>

class RecordingEncodingPCM : public RecordingEncoding
{
public:
    RecordingEncodingPCM(QObject *parent, SoundStreamID id, const RecordingConfig &cfg, const RadioStation *rs, const QString &filename);
    virtual ~RecordingEncodingPCM();

    bool               openOutput(const QString &outputFile);
    void               closeOutput();

protected:
    void               encode(const char *_buffer, size_t buffer_size, char *&export_buffer, size_t &export_buffer_size);


    SNDFILE           *m_output;
};


#endif
