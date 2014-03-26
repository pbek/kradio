/***************************************************************************
                          encoder_pcm.cpp
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

#include "encoder_pcm.h"

#include <QtCore/QFile>

#include <klocale.h>

RecordingEncodingPCM::RecordingEncodingPCM(QObject *parent,            SoundStreamID ssid,
                                           const RecordingConfig &cfg, const RadioStation *rs,
                                           const QString &filename)
    : RecordingEncoding(parent, ssid, cfg, rs, filename),
      m_output(NULL)
{
    m_config.m_SoundFormat.m_Encoding = "raw";
    openOutput(filename);
}


RecordingEncodingPCM::~RecordingEncodingPCM()
{
    closeOutput();
}



void RecordingEncodingPCM::encode(const char *buffer, size_t buffer_size, char *&export_buffer, size_t &export_buffer_size)
{
    if (m_error)
        return;
    m_encodedSize += buffer_size;

    export_buffer      = const_cast<char*>(buffer);
    export_buffer_size = buffer_size;
    int err = sf_write_raw(m_output, const_cast<char*>(buffer), buffer_size);

    if (err != (int)buffer_size) {
        m_error = true;
        m_errorString += i18n("Error %1 writing output. ", QString().setNum(err));
    }
}


bool RecordingEncodingPCM::openOutput(const QString &output)
{
    SF_INFO sinfo;
    m_config.getSoundFileInfo(sinfo, false);
    m_output = sf_open(QFile::encodeName(output), SFM_WRITE, &sinfo);

    if (!m_output) {
        m_error = true;
        m_errorString += i18n("Cannot open output file %1. ", output);
    }
    return !m_error;
}


void RecordingEncodingPCM::closeOutput()
{
    if (m_output) sf_close (m_output);
    m_output = NULL;
}


