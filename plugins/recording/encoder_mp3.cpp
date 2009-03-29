/***************************************************************************
                          encoder_mp3.cpp
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

#include "encoder_mp3.h"

#include <QtCore/QMutex>
#include <QtCore/QDateTime>
#include <klocale.h>

RecordingEncodingMP3::RecordingEncodingMP3(QObject *parent,            SoundStreamID ssid,
                                           const RecordingConfig &cfg, const RadioStation *rs,
                                           const QString &filename)
    : RecordingEncoding(parent, ssid, cfg, rs, filename)
#ifdef HAVE_LAME
      ,
      m_MP3Buffer(NULL),
      m_MP3BufferSize(0),
      m_MP3Output(NULL),
      m_ID3Tags(NULL),
      m_LAMEFlags(NULL),
      m_MP3LBuffer(NULL),
      m_MP3RBuffer(NULL)
#endif
{
    m_config.m_OutputFormat = RecordingConfig::outputMP3;
    m_config.m_SoundFormat.m_Encoding = "mp3";
    openOutput(filename);
}


RecordingEncodingMP3::~RecordingEncodingMP3()
{
    closeOutput();
}


static QMutex  lameSerialization;

void RecordingEncodingMP3::encode(const char *_buffer, size_t buffer_size, char *&export_buffer, size_t &export_buffer_size)
{
    if (m_error)
        return;

#ifdef HAVE_LAME
            short int *buffer = (short int*)_buffer;
            size_t j       = 0,
                   j_inc   = (m_config.m_SoundFormat.m_Channels == 1) ? 1 : 2,
                   dj      = (m_config.m_SoundFormat.m_Channels == 1) ? 0 : 1,
                   samples = buffer_size / m_config.m_SoundFormat.frameSize();

            for (size_t i = 0; i < samples; ++i, j+=j_inc) {
                m_MP3LBuffer[i] = buffer[j];
                m_MP3RBuffer[i] = buffer[j+dj];
            }

            int n = 0;
            lameSerialization.lock();
            n = lame_encode_buffer(m_LAMEFlags,
                                   m_MP3LBuffer,
                                   m_MP3RBuffer,
                                   samples,
                                   m_MP3Buffer,
                                   m_MP3BufferSize);
            lameSerialization.unlock();
            if (n < 0) {
                m_errorString += i18n("Error %1 while encoding mp3. ", QString().setNum(n));
                m_error       = true;
            } else if (n > 0) {
                m_encodedSize += n;

                export_buffer      = (char*)m_MP3Buffer;
                export_buffer_size = n;
                int r = fwrite(m_MP3Buffer, 1, n, m_MP3Output);

                if (r <= 0) {
                    m_errorString += i18n("Error %1 writing output. ", QString().setNum(r));
                    m_error = true;
                }
            }
#endif
}



bool RecordingEncodingMP3::openOutput(const QString &output)
{
#ifdef HAVE_LAME
//         m_output = NULL;
        m_LAMEFlags = lame_init();

        if (!m_LAMEFlags) {
            m_error = true;
            m_errorString += i18n("Cannot initialize lalibmp3lame. ");
        } else {
            lame_set_in_samplerate(m_LAMEFlags, m_config.m_SoundFormat.m_SampleRate);
            lame_set_num_channels(m_LAMEFlags, 2);
            //lame_set_quality(m_LAMEFlags, m_config.mp3Quality);

            lame_set_mode(m_LAMEFlags, m_config.m_SoundFormat.m_Channels == 1 ? MONO : JOINT_STEREO);

    //        lame_seterrorf(m_LAMEFlags, ...);
    //        lame_setdebugf(m_LAMEFlags, ...);
    //        lame_setmsgf(m_LAMEFlags, ...);

            lame_set_VBR(m_LAMEFlags, vbr_default);
            lame_set_VBR_q(m_LAMEFlags, m_config.m_mp3Quality);

            if (lame_init_params(m_LAMEFlags) < 0) {
                m_error = true;
                m_errorString += i18n("Cannot initialize libmp3lame parameters. ", output);
            }

            if (!m_error) {
                id3tag_init(m_LAMEFlags);
                id3tag_add_v2(m_LAMEFlags);
                QString title = m_RadioStation ? m_RadioStation->name() : i18n("unknown station");
                title += QString(" - %1").arg(QDateTime::currentDateTime().toString(Qt::ISODate));
                QString comment = i18n("Recorded by KRadio");
                size_t l = title.length() + comment.length() + 10;
                m_ID3Tags = new char[l];
                char *ctitle   = m_ID3Tags;
                strcpy(ctitle, title.toLocal8Bit());
                char *ccomment = m_ID3Tags + strlen(ctitle) + 1;
                strcpy(ccomment, comment.toLocal8Bit());
                id3tag_set_title(m_LAMEFlags, ctitle);
                id3tag_set_comment(m_LAMEFlags, ccomment);
            }

            m_MP3Output = fopen(output.toLocal8Bit(), "wb+");
            if (!m_MP3Output) {
                m_errorString += i18n("Cannot open output file %1. ", output);
                m_error = true;
            }

            size_t nSamples = m_config.m_EncodeBufferSize / m_config.m_SoundFormat.frameSize();
            m_MP3BufferSize = nSamples + nSamples / 4 + 7200;
            m_MP3Buffer  = new unsigned char[m_MP3BufferSize];

            m_MP3LBuffer = new short int[nSamples];
            m_MP3RBuffer = new short int[nSamples];

            if (!m_MP3Buffer || !m_MP3LBuffer || !m_MP3RBuffer) {
                m_error = true;
                m_errorString += i18n("Cannot allocate buffers for mp3 encoding. ");
            }
        }

        if (m_error) {
            if (m_LAMEFlags) lame_close(m_LAMEFlags);
            m_LAMEFlags = NULL;
            if (m_MP3Output) fclose(m_MP3Output);
            m_MP3Output = NULL;
            if (m_MP3Buffer) delete [] m_MP3Buffer;
            m_MP3Buffer = NULL;
            m_MP3BufferSize = 0;
            if (m_ID3Tags) delete [] m_ID3Tags;
            m_ID3Tags = NULL;
            if (m_MP3LBuffer) delete[] m_MP3LBuffer;
            if (m_MP3RBuffer) delete[] m_MP3RBuffer;
            m_MP3LBuffer = m_MP3RBuffer = NULL;
        }
#endif
    return !m_error;
}


void RecordingEncodingMP3::closeOutput()
{
#ifdef HAVE_LAME
    if (m_LAMEFlags) {
        if (m_config.m_OutputFormat == RecordingConfig::outputMP3) {
            int n = lame_encode_flush(m_LAMEFlags,
                                      m_MP3Buffer,
                                      m_MP3BufferSize);
            if (n < 0) {
                m_error = true;
                m_errorString += i18n("Error %1 while encoding mp3. ", QString().setNum(n));
            } else if (n > 0) {
                int r = fwrite(m_MP3Buffer, 1, n, m_MP3Output);
                if (r <= 0) {
                    m_error = true;
                    m_errorString += i18n("Error %1 writing output. ", QString().setNum(r));
                } else {
                    lame_mp3_tags_fid(m_LAMEFlags, m_MP3Output);
                }
            }
        }
        if (m_LAMEFlags) lame_close(m_LAMEFlags);
        m_LAMEFlags = NULL;
        if (m_MP3Output) fclose(m_MP3Output);
        m_MP3Output = NULL;
        m_MP3BufferSize = 0;
        if (m_MP3Buffer) delete [] m_MP3Buffer;
        m_MP3Buffer = NULL;
        if (m_ID3Tags) delete [] m_ID3Tags;
        m_ID3Tags = NULL;
        if (m_MP3LBuffer) delete[] m_MP3LBuffer;
        if (m_MP3RBuffer) delete[] m_MP3RBuffer;
        m_MP3LBuffer = m_MP3RBuffer = NULL;
    }
#endif
}
