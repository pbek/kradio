/***************************************************************************
                          encoder_ogg.cpp
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

#include "encoder_ogg.h"

#include <QtCore/QDateTime>
#include <QtCore/QTextCodec>

#include <klocale.h>
#include <stdlib.h>


#include <errorlog_interfaces.h>

RecordingEncodingOgg::RecordingEncodingOgg(QObject *parent,            SoundStreamID ssid,
                                           const RecordingConfig &cfg, const RadioStation *rs,
                                           const QString &filename)
    : RecordingEncoding(parent, ssid, cfg, rs, filename)
#ifdef HAVE_OGG
      ,
      m_OggOutput(NULL),
      m_OggExportBuffer(NULL),
      m_OggExportBufferSize(0)
#endif
{
    m_config.m_OutputFormat = RecordingConfig::outputOGG;
    m_config.m_SoundFormat.m_Encoding = "ogg";
    openOutput(filename);
}


RecordingEncodingOgg::~RecordingEncodingOgg()
{
    closeOutput();
}

void RecordingEncodingOgg::encode(const char *_buffer, size_t buffer_size, char *&export_buffer, size_t &export_buffer_size)
{
    if (m_error)
        return;

#ifdef HAVE_OGG
    SoundFormat &sf = m_config.m_SoundFormat;
    ogg_page     ogg_pg;
    ogg_packet   ogg_pkt;

    size_t samples = buffer_size / sf.frameSize();

    // buffer[channel][sample], normalized to -1..0..+1
    float **buffer = vorbis_analysis_buffer(&m_VorbisDSP, (samples < 512 ? 512 : samples));

    sf.convertSamplesToFloatNonInterleaved(_buffer, buffer, samples);

    /* Tell the library how many samples (per channel) we wrote
       into the supplied buffer */
    vorbis_analysis_wrote(&m_VorbisDSP, samples);

    /* While we can get enough data from the library to analyse, one
       block at a time... */

    bool eos = false;
    while(!m_error && !eos && vorbis_analysis_blockout(&m_VorbisDSP, &m_VorbisBlock) == 1) {

        /* Do the main analysis, creating a packet */
        vorbis_analysis(&m_VorbisBlock, NULL);
        vorbis_bitrate_addblock(&m_VorbisBlock);

        while(!m_error && vorbis_bitrate_flushpacket(&m_VorbisDSP, &ogg_pkt)) {
            /* Add packet to bitstream */
            ogg_stream_packetin(&m_OggStream,&ogg_pkt);

            /* If we've gone over a page boundary, we can do actual output,
               so do so (for however many pages are available) */

            while(!m_error && !eos) {
                int result = ogg_stream_pageout(&m_OggStream, &ogg_pg);
                if (!result) break;

                int n  = m_OggOutput->write((const char*)ogg_pg.header, ogg_pg.header_len);
                    n += m_OggOutput->write((const char*)ogg_pg.body,   ogg_pg.body_len);

                m_encodedSize += n;

                if (n != (ogg_pg.header_len + ogg_pg.body_len)) {
                    m_error = true;
                    m_errorString += i18n("Failed writing data to Ogg/Vorbis output stream. ");
                    break;
                } else {

                    if (m_OggExportBufferSize < export_buffer_size + n) {
                        m_OggExportBuffer = (char*)realloc(m_OggExportBuffer, m_OggExportBufferSize + 2 * n);
                        m_OggExportBufferSize += 2 * n;
                    }

                    memcpy (m_OggExportBuffer + export_buffer_size, ogg_pg.header, ogg_pg.header_len);
                    export_buffer_size += ogg_pg.header_len;
                    memcpy (m_OggExportBuffer + export_buffer_size, ogg_pg.body,   ogg_pg.body_len);
                    export_buffer_size += ogg_pg.body_len;

                }
                if (ogg_page_eos(&ogg_pg))
                    eos = 1;
            }
        }
    }

    export_buffer = m_OggExportBuffer;
#endif
}


#ifdef HAVE_OGG
static void vorbis_comment_add_tag_new(vorbis_comment *vc, const QString &tag, const QString &value)
{
    QByteArray   ba_tag    = tag  .toUtf8();
    QByteArray   ba_value  = value.toUtf8();
    vorbis_comment_add_tag(vc, ba_tag, ba_value);
}
#endif

bool RecordingEncodingOgg::openOutput(const QString &output)
{
#ifdef HAVE_OGG
    m_OggOutput = new QFile(output);
    m_OggOutput->open(QIODevice::WriteOnly);
    if (m_OggOutput->error()) {
        m_errorString += i18n("Cannot open Ogg/Vorbis output file %1: %2", output, m_OggOutput->errorString());
        m_error = true;
    }

    m_OggExportBuffer = (char*)malloc(m_OggExportBufferSize = 65536); // start with a 64k buffer


    /* Have vorbisenc choose a mode for us */
    vorbis_info_init(&m_VorbisInfo);

    SoundFormat &sf = m_config.m_SoundFormat;
    if (vorbis_encode_setup_vbr(&m_VorbisInfo, sf.m_Channels, sf.m_SampleRate, m_config.m_oggQuality)) {
        m_error = true;
        m_errorString = i18n("Ogg/Vorbis Mode initialisation failed: invalid parameters for quality");
        vorbis_info_clear(&m_VorbisInfo);
        return false;
    }

    /* Turn off management entirely (if it was turned on). */
    vorbis_encode_ctl(&m_VorbisInfo, OV_ECTL_RATEMANAGE_SET, NULL);
    vorbis_encode_setup_init(&m_VorbisInfo);

    /* Now, set up the analysis engine, stream encoder, and other
       preparation before the encoding begins.
     */

    vorbis_analysis_init(&m_VorbisDSP, &m_VorbisInfo);
    vorbis_block_init(&m_VorbisDSP, &m_VorbisBlock);

    ogg_stream_init (&m_OggStream, m_SoundStreamID.getID());

    /* Now, build the three header packets and send through to the stream
       output stage (but defer actual file output until the main encode loop) */

    ogg_packet header_main;
    ogg_packet header_comments;
    ogg_packet header_codebooks;

    /* Build the packets */
    vorbis_comment  vc;
    vorbis_comment_init (&vc);
    vorbis_comment_add_tag_new(&vc, "creator", "KRadio" KRADIO_VERSION);
    vorbis_comment_add_tag_new(&vc, "title",   m_config.m_template.id3Title);
    vorbis_comment_add_tag_new(&vc, "artist",  m_config.m_template.id3Artist);
    vorbis_comment_add_tag_new(&vc, "genre",   m_config.m_template.id3Genre);
    vorbis_comment_add_tag_new(&vc, "date",    QDateTime::currentDateTime().toString(Qt::ISODate));

    vorbis_analysis_headerout(&m_VorbisDSP, &vc,
                              &header_main, &header_comments, &header_codebooks);

    /* And stream them out */
    ogg_stream_packetin(&m_OggStream, &header_main);
    ogg_stream_packetin(&m_OggStream, &header_comments);
    ogg_stream_packetin(&m_OggStream, &header_codebooks);

    int      result;
    ogg_page ogg_page;
    while((result = ogg_stream_flush(&m_OggStream, &ogg_page))) {

        if (!result) break;

        int n  = m_OggOutput->write((const char*)ogg_page.header, ogg_page.header_len);
            n += m_OggOutput->write((const char*)ogg_page.body,   ogg_page.body_len);

        if(n != ogg_page.header_len + ogg_page.body_len) {
            m_error = true;
            m_errorString += i18n("Failed writing Ogg/Vorbis header to output stream");
            break;
        }
    }

    vorbis_comment_clear (&vc);

    if (m_error) {
        if (m_OggOutput) {
            if (m_OggOutput->isOpen()) {
                m_OggOutput->close();
            }
            delete m_OggOutput;
            m_OggOutput = NULL;
        }
        free(m_OggExportBuffer);
        m_OggExportBuffer     = NULL;
        m_OggExportBufferSize = 0;

        ogg_stream_clear(&m_OggStream);
        vorbis_block_clear(&m_VorbisBlock);
        vorbis_dsp_clear(&m_VorbisDSP);
        vorbis_info_clear(&m_VorbisInfo);
    }

    return !m_error;
#endif
}


void RecordingEncodingOgg::closeOutput()
{
#ifdef HAVE_OGG
    if (m_OggOutput) {

        char     *tmp_buf  = NULL;
        size_t    tmp_size = 0;
        // flush buffer
        encode(tmp_buf, tmp_size, tmp_buf, tmp_size);

        if (m_OggOutput->isOpen()) {
            m_OggOutput->close();
        }
        delete m_OggOutput;
        m_OggOutput = NULL;

        free(m_OggExportBuffer);
        m_OggExportBuffer     = NULL;
        m_OggExportBufferSize = 0;

        ogg_stream_clear(&m_OggStream);
        vorbis_block_clear(&m_VorbisBlock);
        vorbis_dsp_clear(&m_VorbisDSP);
        vorbis_info_clear(&m_VorbisInfo);
    }
#endif
}


