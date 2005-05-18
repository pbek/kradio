/***************************************************************************
                          encoder.cpp  -  description
                             -------------------
    begin                : Thu May 05 2005
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

#include <kradio/radio-stations/radiostation.h>
#include <kradio/interfaces/errorlog-interfaces.h>
#include <kradio/libkradio-gui/aboutwidget.h>

#include "recording.h"
#include "recording-configuration.h"
#include "soundstreamevent.h"

#include <qsocketnotifier.h>
#include <qevent.h>
#include <qapplication.h>
#include <qregexp.h>

#include <kconfig.h>
#include <kdeversion.h>
#include <klocale.h>

#include <sndfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>




RecordingEncoding::RecordingEncoding(QObject *parent,            SoundStreamID ssid,
                                     const RecordingConfig &cfg, const RadioStation *rs,
                                     const QString &filename)
    :
      m_parent(parent),
      m_config(cfg),
      m_RadioStation(rs ? rs->copy() : NULL),
      m_SoundStreamID(ssid),
      m_inputAvailableLock(1),
      m_error(false),
      m_errorString(QString::null),
      m_done(false),
      m_buffersInput(NULL),
      m_buffersInputFill(NULL),
      m_currentInputBuffer(0),
      m_buffersMetaData(NULL),
      m_encodedSize(0),
      m_InputStartTime(0),
      m_InputStartPosition(0),
      m_outputURL(filename),
      m_output(NULL)
#ifdef HAVE_LAME_LAME_H
      ,
      m_MP3Buffer(NULL),
      m_MP3BufferSize(0),
      m_MP3Output(NULL),
      m_ID3Tags(NULL),
      m_LAMEFlags(NULL),
      m_MP3LBuffer(NULL),
      m_MP3RBuffer(NULL)
#endif
#if defined(HAVE_VORBIS_VORBISENC_H) && defined(HAVE_OGG_OGG_H)
      ,
      m_OggOutput(NULL),
      m_OggExportBuffer(NULL),
      m_OggExportBufferSize(0)
#endif
{
    if (m_config.m_EncodeBufferCount < 3)
        m_config.m_EncodeBufferCount = 3;
    if (m_config.m_EncodeBufferSize < 4096)
        m_config.m_EncodeBufferSize = 4096;

    m_buffersInput = new char* [m_config.m_EncodeBufferCount];
    m_buffersInputFill = new unsigned int [m_config.m_EncodeBufferCount];
    m_buffersMetaData  = new QPtrList<BufferSoundMetaData> *[m_config.m_EncodeBufferCount];
    for (unsigned int i = 0; i < m_config.m_EncodeBufferCount; ++i) {
        m_buffersInput    [i] = new char [m_config.m_EncodeBufferSize];
        m_buffersMetaData [i] = new QPtrList<BufferSoundMetaData>;
        m_buffersMetaData [i]->setAutoDelete(true);
        m_buffersInputFill[i] = 0;
    }

    m_inputAvailableLock++;

    switch (m_config.m_OutputFormat) {
        case RecordingConfig::outputMP3 :
            m_config.m_SoundFormat.m_Encoding = "mp3";
            break;
        case RecordingConfig::outputOGG :
            m_config.m_SoundFormat.m_Encoding = "ogg";
            break;
        default :
            m_config.m_SoundFormat.m_Encoding = "raw";
            break;
    }
    openOutput(filename);
}


RecordingEncoding::~RecordingEncoding()
{
    for (unsigned int i = 0; i < m_config.m_EncodeBufferCount; ++i) {
        delete m_buffersInput[i];
        delete m_buffersMetaData[i];
    }
    delete m_buffersInput;
    delete m_buffersInputFill;
    delete m_buffersMetaData;
    m_buffersInputFill = NULL;
    m_buffersInput = NULL;
    delete m_RadioStation;
    closeOutput();
}


char *RecordingEncoding::lockInputBuffer(unsigned int &bufferSize)
{
    if (m_done || m_error)
        return NULL;

    m_bufferInputLock.lock();

    unsigned int bytesAvailable = 0;

    do {
        bytesAvailable = m_config.m_EncodeBufferSize - m_buffersInputFill[m_currentInputBuffer];
        if (bytesAvailable > 0) {
            bufferSize = bytesAvailable;
            return m_buffersInput[m_currentInputBuffer] + m_buffersInputFill[m_currentInputBuffer];
        } else if (m_currentInputBuffer + 1 < m_config.m_EncodeBufferCount) {
            ++m_currentInputBuffer;
            m_inputAvailableLock--;
        }
    } while (m_currentInputBuffer + 1 < m_config.m_EncodeBufferCount);
    m_bufferInputLock.unlock();
    return NULL;
}


void  RecordingEncoding::unlockInputBuffer(unsigned int bufferSize, const SoundMetaData &md)
{
    if (m_done)
        return;

    if (m_buffersInputFill[m_currentInputBuffer] + bufferSize > m_config.m_EncodeBufferSize) {
        m_error = true;
        m_errorString += i18n("Buffer Overflow. ");
    } else {
        if (!m_InputStartTime) {
            m_InputStartTime     = md.absoluteTimestamp();
            m_InputStartPosition = md.position();
        }
        BufferSoundMetaData *bmd = new BufferSoundMetaData(
                                md.position() - m_InputStartPosition,
                                md.absoluteTimestamp() - m_InputStartTime,
                                md.absoluteTimestamp(),
                                md.url(),
                                m_buffersInputFill[m_currentInputBuffer]);
        m_buffersMetaData[m_currentInputBuffer]->append(bmd);
        m_buffersInputFill[m_currentInputBuffer] += bufferSize;
    }
    m_bufferInputLock.unlock();
}


void RecordingEncoding::setDone()
{
    m_done = true;
    m_inputAvailableLock--;
}


static QMutex  lameSerialization;

void RecordingEncoding::run()
{
    BufferSoundMetaData  last_md;

    while (!m_error) {
        if (!m_done)
            m_inputAvailableLock++;

        if (!m_buffersInputFill[0]) {
            if (m_done)
                break;
            else
                continue;
        }

        char        *export_buffer = NULL;
        unsigned int export_buffer_size = 0;

        Q_UINT64  old_pos  = m_encodedSize;

        switch(m_config.m_OutputFormat) {
            case RecordingConfig::outputMP3 :
                encode_mp3(m_buffersInput[0], m_buffersInputFill[0], export_buffer, export_buffer_size);
                break;
            case RecordingConfig::outputOGG :
                encode_ogg(m_buffersInput[0], m_buffersInputFill[0], export_buffer, export_buffer_size);
                break;
            default:
                encode_pcm(m_buffersInput[0], m_buffersInputFill[0], export_buffer, export_buffer_size);
                break;
        }

        SoundStreamEncodingStepEvent *step_event = NULL;

        if (!m_error) {

            // event data must be copied before buffer rotation
            last_md = *m_buffersMetaData[0]->first();
            SoundMetaData  md(old_pos, last_md.relativeTimestamp(), last_md.absoluteTimestamp(), m_outputURL);
            step_event = new SoundStreamEncodingStepEvent(m_SoundStreamID, export_buffer, export_buffer_size, md);

            // rotate buffers
            m_bufferInputLock.lock();

            char                          *tmpBuf  = m_buffersInput[0];
            QPtrList<BufferSoundMetaData> *tmpList = m_buffersMetaData[0];
            for (unsigned int i = 0; i < m_config.m_EncodeBufferCount - 1; ++i) {
                m_buffersInput    [i] = m_buffersInput    [i+1];
                m_buffersInputFill[i] = m_buffersInputFill[i+1];
                m_buffersMetaData [i] = m_buffersMetaData [i+1];
            }
            m_buffersInput    [m_config.m_EncodeBufferCount - 1] = tmpBuf;
            m_buffersInputFill[m_config.m_EncodeBufferCount - 1] = 0;
            m_buffersMetaData [m_config.m_EncodeBufferCount - 1] = tmpList;
            tmpList->clear();
            m_currentInputBuffer--;

            m_bufferInputLock.unlock();
        }

        if (step_event)
            QApplication::postEvent(m_parent, step_event);
    }
    m_done = true;
    closeOutput();

    SoundMetaData        md(m_encodedSize, last_md.relativeTimestamp(), last_md.absoluteTimestamp(), m_outputURL);
    QApplication::postEvent(m_parent, new SoundStreamEncodingStepEvent(m_SoundStreamID, NULL, 0, md));

    QApplication::postEvent(m_parent, new SoundStreamEncodingTerminatedEvent(m_SoundStreamID));
}


void RecordingEncoding::encode_pcm(const char *buffer, unsigned buffer_size, char *&export_buffer, unsigned &export_buffer_size)
{
    if (m_error)
        return;
    m_encodedSize += buffer_size;

    export_buffer      = const_cast<char*>(buffer);
    export_buffer_size = buffer_size;
    int err = sf_write_raw(m_output, const_cast<char*>(buffer), buffer_size);

    if (err != (int)buffer_size) {
        m_error = true;
        m_errorString += i18n("Error %1 writing output. ").arg(QString().setNum(err));
    }
}


void RecordingEncoding::encode_mp3(const char *_buffer, unsigned buffer_size, char *&export_buffer, unsigned &export_buffer_size)
{
    if (m_error)
        return;

#ifdef HAVE_LAME_LAME_H
            short int *buffer = (short int*)_buffer;
            int j       = 0,
                j_inc   = (m_config.m_SoundFormat.m_Channels == 1) ? 1 : 2,
                dj      = (m_config.m_SoundFormat.m_Channels == 1) ? 0 : 1,
                samples = buffer_size / m_config.m_SoundFormat.frameSize();

            for (int i = 0; i < samples; ++i, j+=j_inc) {
                m_MP3LBuffer[i] = buffer[j];
                m_MP3RBuffer[i] = buffer[j+dj];
            }

            int n = m_MP3BufferSize;
            lameSerialization.lock();
            n = lame_encode_buffer(m_LAMEFlags,
                                   m_MP3LBuffer,
                                   m_MP3RBuffer,
                                   samples,
                                   m_MP3Buffer,
                                   m_MP3BufferSize);
            lameSerialization.unlock();
            if (n < 0) {
                m_errorString += i18n("Error %1 while encoding mp3. ").arg(QString().setNum(n));
                m_error       = true;
            } else if (n > 0) {
                m_encodedSize += n;

                export_buffer      = (char*)m_MP3Buffer;
                export_buffer_size = n;
                int r = fwrite(m_MP3Buffer, 1, n, m_MP3Output);

                if (r <= 0) {
                    m_errorString += i18n("Error %1 writing output. ").arg(QString().setNum(r));
                    m_error = true;
                }
            }
#endif
}


void RecordingEncoding::encode_ogg(const char *_buffer, unsigned buffer_size, char *&export_buffer, unsigned &export_buffer_size)
{
    if (m_error)
        return;

#if defined(HAVE_VORBIS_VORBISENC_H) && defined(HAVE_OGG_OGG_H)
    SoundFormat &sf = m_config.m_SoundFormat;
    ogg_page     ogg_pg;
    ogg_packet   ogg_pkt;

    unsigned samples = buffer_size / sf.frameSize();

    // buffer[channel][sample], normalized to -1..0..+1
    float **buffer = vorbis_analysis_buffer(&m_VorbisDSP, (samples < 512 ? 512 : samples));

    sf.convertSamplesToFloat(_buffer, buffer, samples);

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

                int n  = fwrite(ogg_pg.header, 1, ogg_pg.header_len, m_OggOutput);
                n     += fwrite(ogg_pg.body,   1, ogg_pg.body_len,   m_OggOutput);

                m_encodedSize += n;

                if (n != (ogg_pg.header_len + ogg_pg.body_len)) {
                    m_error = true;
                    m_errorString += i18n("Failed writing data to ogg/vorbis output stream. ");
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


bool RecordingEncoding::openOutput(const QString &output)
{
    if (m_config.m_OutputFormat == RecordingConfig::outputMP3) {

        return openOutput_mp3(output);

    } else if (m_config.m_OutputFormat == RecordingConfig::outputOGG) {

        return openOutput_ogg(output);

    } else {

        return openOutput_pcm(output);

    }
}


bool RecordingEncoding::openOutput_pcm(const QString &output)
{
    SF_INFO sinfo;
    m_config.getSoundFileInfo(sinfo, false);
    m_output = sf_open(output, SFM_WRITE, &sinfo);

    if (!m_output) {
        m_error = true;
        m_errorString += i18n("Cannot open output file %1. ").arg(output);
    }
    return !m_error;
}


bool RecordingEncoding::openOutput_mp3(const QString &output)
{
#ifdef HAVE_LAME_LAME_H
        m_output = NULL;
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
                m_errorString += i18n("Cannot initialize libmp3lame parameters. ").arg(output);
            }

            if (!m_error) {
                id3tag_init(m_LAMEFlags);
                id3tag_add_v2(m_LAMEFlags);
                QString title  = m_RadioStation->name() + QString().sprintf(" - %s", (const char*)(QDateTime::currentDateTime().toString(Qt::ISODate)));
                QString comment = i18n("Recorded by KRadio");
                int l = title.length() + comment.length() + 10;
                m_ID3Tags = new char[l];
                char *ctitle   = m_ID3Tags;
                strcpy(ctitle, title.latin1());
                char *ccomment = m_ID3Tags + strlen(ctitle) + 1;
                strcpy(ccomment, comment.latin1());
                id3tag_set_title(m_LAMEFlags, ctitle);
                id3tag_set_comment(m_LAMEFlags, ccomment);
            }

            m_MP3Output = fopen(output, "wb+");
            if (!m_MP3Output) {
                m_errorString += i18n("Cannot open output file %1. ").arg(output);
                m_error = true;
            }

            int nSamples = m_config.m_EncodeBufferSize / m_config.m_SoundFormat.frameSize();
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



static void vorbis_comment_add_tag_new(vorbis_comment *vc, const QString &tag, const QString &value)
{
    char *stag   = strdup(tag);
    char *svalue = strdup(value.utf8());
    vorbis_comment_add_tag(vc, stag, svalue);
    delete stag;
    delete svalue;
}


bool RecordingEncoding::openOutput_ogg(const QString &output)
{
#if defined(HAVE_VORBIS_VORBISENC_H) && defined(HAVE_OGG_OGG_H)
    m_OggOutput = fopen(output, "wb+");
    if (!m_OggOutput) {
        m_errorString += i18n("Cannot open Ogg/Vorbis output file %1. ").arg(output);
        m_error = true;
    }

    m_OggExportBuffer = (char*)malloc(m_OggExportBufferSize = 65536); // start with a 64k buffer


    /* Have vorbisenc choose a mode for us */
    vorbis_info_init(&m_VorbisInfo);

    SoundFormat &sf = m_config.m_SoundFormat;
    if (vorbis_encode_setup_vbr(&m_VorbisInfo, sf.m_Channels, sf.m_SampleRate, m_config.m_oggQuality)) {
        m_error = true;
        m_errorString = "Ogg/Vorbis Mode initialisation failed: invalid parameters for quality\n";
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
    vorbis_comment_add_tag_new(&vc, "creator", "KRadio" VERSION);
    vorbis_comment_add_tag_new(&vc, "title",   m_RadioStation->longName().utf8());
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

        int n = fwrite(ogg_page.header, 1, ogg_page.header_len, m_OggOutput);
        n    += fwrite(ogg_page.body,   1, ogg_page.body_len,   m_OggOutput);

        if(n != ogg_page.header_len + ogg_page.body_len) {
            m_error = true;
            m_errorString += i18n("Failed writing Ogg/Vorbis header to output stream\n");
            break;
        }
    }

    vorbis_comment_clear (&vc);

    if (m_error) {
        if (m_OggOutput)  fclose (m_OggOutput);
        m_OggOutput = NULL;
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


void RecordingEncoding::closeOutput()
{
    if (m_config.m_OutputFormat == RecordingConfig::outputMP3) {

        return closeOutput_mp3();

    } else if (m_config.m_OutputFormat == RecordingConfig::outputOGG) {

        return closeOutput_ogg();

    } else {

        return closeOutput_pcm();;

    }
}

void RecordingEncoding::closeOutput_pcm()
{
    if (m_output) sf_close (m_output);
    m_output = NULL;
}


void RecordingEncoding::closeOutput_mp3()
{
#ifdef HAVE_LAME_LAME_H
    if (m_LAMEFlags) {
        if (m_config.m_OutputFormat == RecordingConfig::outputMP3) {
            int n = lame_encode_flush(m_LAMEFlags,
                                      m_MP3Buffer,
                                      m_MP3BufferSize);
            if (n < 0) {
                m_error = true;
                m_errorString += i18n("Error %1 while encoding mp3. ").arg(QString().setNum(n));
            } else if (n > 0) {
                int r = fwrite(m_MP3Buffer, 1, n, m_MP3Output);
                if (r <= 0) {
                    m_error = true;
                    m_errorString += i18n("Error %1 writing output. ").arg(QString().setNum(r));
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

void RecordingEncoding::closeOutput_ogg()
{
#if defined(HAVE_VORBIS_VORBISENC_H) && defined(HAVE_OGG_OGG_H)
    if (m_OggOutput) {

        char     *tmp_buf  = NULL;
        unsigned  tmp_size = 0;
        // flush buffer
        encode_ogg(tmp_buf, tmp_size, tmp_buf, tmp_size);

        fclose(m_OggOutput);
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

