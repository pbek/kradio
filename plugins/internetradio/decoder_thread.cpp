/***************************************************************************
                          decoder_thread.cpp  -  description
                             -------------------
    begin                : Thu Feb 23 2009
    copyright            : (C) 2009 by Martin Witte
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

#include <math.h>
#include <unistd.h>

#include <QFile>

#include <kio/jobclasses.h>
#include <kio/netaccess.h>
#include <kio/job.h>
#include <kconfig.h>
#include <kconfiggroup.h>

#include "decoder_thread.h"
#include "errorlog_interfaces.h"

#include "libav-global.h"
extern "C" {
  #include <libavformat/avio.h>
  #include <libavformat/avformat.h>
  #include <libavutil/opt.h>
}

#ifndef AV_INPUT_BUFFER_PADDING_SIZE /* LIBAVCODEC_VERSION_INT < AV_VERSION_INT(56, 60, 100) */
# define AV_INPUT_BUFFER_PADDING_SIZE FF_INPUT_BUFFER_PADDING_SIZE
#endif


InternetRadioDecoder::InternetRadioDecoder(QObject                    *event_parent,
                                           const InternetRadioStation &rs,
#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
                                           const KUrl::List           &playlist,
#else
                                           const KUrl                 &currentStreamUrl,
                                           StreamInputBuffer          *streamInputBuffer,
                                           QString                     contentType,
#endif
                                           int                         max_buffers,
                                           int                         max_singleBufferSize,
                                           int                         max_probe_size_bytes,
                                           float                       max_analyze_secs,
                                           int                         max_retries
                                          )
:   m_decoderOpened       (false),
    m_av_pFormatCtx       (NULL),
    m_av_pFormatCtx_opened(false),
    m_av_audioStream      (-1),
    m_av_aCodecCtx        (NULL),
    m_av_aCodec           (NULL),
#if  LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(52, 110, 0) // checked: avformat_open_input in ffmpeg >= 0.7
    m_av_byteio_contextPtr(NULL),
#endif
#if  LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 42, 0) // checked: avcodec_decode_audio4 in ffmpeg >= 0.9
    m_decoded_frame       (NULL),
#endif
#if defined(HAVE_LIBAVRESAMPLE) || defined (HAVE_LIBSWRESAMPLE)
    m_resample_context    (NULL),
#endif

#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    m_is_mms_stream (false),
    m_mms_stream    (NULL),
#endif

    m_parent        (event_parent),
    m_RadioStation  (rs),

    m_error         (false),
    m_done          (false),

    m_decodedSize   (0),
    m_encodedSize   (0),
    m_startTime     (0),

#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    m_playListURLs  (playlist),
#endif
/*    m_inputURL      (m_RadioStation.pathOrUrl()),*/
    m_bufferCountSemaphore(max_buffers),
    m_maxBufferCount      (max_buffers),
    m_maxSingleBufferSize (max_singleBufferSize),
#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    m_inputUrl(currentStreamUrl),
    m_streamInputBuffer(streamInputBuffer),
    m_contentType(contentType),
#endif
    m_maxProbeSize  (max_probe_size_bytes > 2048 ? max_probe_size_bytes : 8192),
    m_maxAnalyzeTime(max_analyze_secs     > 0.3  ? max_analyze_secs     : 0.8),
    m_maxRetries    (max_retries)
#ifdef DEBUG_DUMP_DECODER_STREAMS
    , m_debugCodedStream(NULL)
    , m_debugDecodedStream(NULL)
    , m_debugMetaStream(NULL)
#endif
{
#ifdef DEBUG_DUMP_DECODER_STREAMS
    m_debugCodedStream   = fopen("/tmp/kradio-debug-coded-stream",   "w");
    m_debugDecodedStream = fopen("/tmp/kradio-debug-decoded-stream", "w");
    m_debugMetaStream    = fopen("/tmp/kradio-debug-meta-stream",    "w");
#endif

    QObject::connect(this, SIGNAL(sigSelfTrigger()), this, SLOT(run()), Qt::QueuedConnection);
    emit sigSelfTrigger();
}

InternetRadioDecoder::~InternetRadioDecoder()
{
    flushBuffers();
    closeAVStream();
#ifdef DEBUG_DUMP_DECODER_STREAMS
    fclose(m_debugCodedStream);
    fclose(m_debugDecodedStream);
    fclose(m_debugMetaStream);
#endif
}


#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
void InternetRadioDecoder::selectStreamFromPlaylist()
{
    int retries = m_maxRetries;
    unsigned int start_play_idx = (unsigned int) rint((m_playListURLs.size()-1) * (float)rand() / (float)RAND_MAX);
    int n = m_playListURLs.size();
    for (int i = 0; !m_decoderOpened && i < n; ++i) {
        m_playURL = m_playListURLs[(start_play_idx + i) % n];
        for (int j = 0; j < retries && !m_decoderOpened; ++j) {
            log(ThreadLogging::LogDebug, i18n("opening stream %1", m_playURL.pathOrUrl()));
            openAVStream(m_playURL.pathOrUrl(), true);
        }
        if (!m_decoderOpened) {
            log(ThreadLogging::LogWarning, i18n("Failed to open %1. Trying next stream in play list.", m_playURL.pathOrUrl()));
        }
    }
    if (!m_decoderOpened) {
        m_error = true;
        log(ThreadLogging::LogError, i18n("Could not open any input stream of %1.", m_RadioStation.url().pathOrUrl()));
    }
}
#endif


bool InternetRadioDecoder::decodePacket(AVPacket &pkt, int &processed_input_bytes)
{

#ifdef DEBUG_EXIT_ON_EXAMPLE_ERROR_PACKET
    if (m_debugPacketCount == 29289) {
        printf("packet %zi: (buf: %p)\n", m_debugPacketCount, pkt.data);
        for (int i = 0; i < 8 ; ++i) {
            printf("%02x ", (unsigned char)pkt.data[i]);
        }
        printf("\n");
        exit (-1);
    }
#endif

    char *output_buf             = NULL;
    int   generated_output_bytes = 0;
          processed_input_bytes  = 0;
#if  LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 42, 0) // checked: avcodec_decode_audio4 in ffmpeg >= 0.9
    int   got_frame              = 0;
#endif

#if  LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 42, 0) // checked: avcodec_decode_audio4 in ffmpeg >= 0.9
#if  LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 28, 1) // libav commit: https://gitorious.org/libav/libav/commit/d7b3ee9a3a03ab88d61a5895fbdbc6689f4dd671
    av_frame_unref(m_decoded_frame);
#else
    avcodec_get_frame_defaults(m_decoded_frame);
#endif
    processed_input_bytes = avcodec_decode_audio4(m_av_aCodecCtx,
                                                  m_decoded_frame,
                                                  &got_frame,
                                                  &pkt);

#if defined(HAVE_LIBAVRESAMPLE) || defined (HAVE_LIBSWRESAMPLE)
    DECLARE_ALIGNED(32, uint8_t, tmp_resample_output_buffer)[m_decoded_frame->nb_samples * m_soundFormat.frameSize() + 64];
#endif
    if (processed_input_bytes > 0 && got_frame) {
        /* if a frame has been decoded, output it */
        int lineSize = 0;
        generated_output_bytes = av_samples_get_buffer_size(&lineSize,
                                                             m_av_aCodecCtx->channels,
                                                             m_decoded_frame->nb_samples,
                                                             m_av_aCodecCtx->sample_fmt,
                                                             0
                                                           );
        output_buf = (char*)m_decoded_frame->data[0];

#if defined(HAVE_LIBAVRESAMPLE) || defined (HAVE_LIBSWRESAMPLE)
        uint8_t *tmpBuf[2]        = { tmp_resample_output_buffer, NULL };
        int      resampleLineSize = sizeof(tmp_resample_output_buffer) / m_soundFormat.m_Channels;
        #ifdef HAVE_LIBAVRESAMPLE
            int ret = avresample_convert(m_resample_context, 
                                         tmpBuf, resampleLineSize, m_decoded_frame->nb_samples, 
                                         m_decoded_frame->extended_data, lineSize, m_decoded_frame->nb_samples
                                        );
        #elif  defined (HAVE_LIBSWRESAMPLE)
            int ret = swr_convert       (m_resample_context, 
                                         tmpBuf, m_decoded_frame->nb_samples, 
                                         (const uint8_t**)m_decoded_frame->extended_data, m_decoded_frame->nb_samples
                                        );
        #endif
        if (ret < 0) {
            processed_input_bytes = 0;
            got_frame             = 0;
        }
        output_buf = (char*)tmp_resample_output_buffer;
        generated_output_bytes = ret * m_soundFormat.frameSize();
#endif
    }

#elif  LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 72, 2) // checked: avcodec_decode_audio3 in ffmpeg >= 0.6
    DECLARE_ALIGNED(16, char, output_buf_data)[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
    output_buf             = output_buf_data;
    generated_output_bytes = sizeof(output_buf_data);

    processed_input_bytes  = avcodec_decode_audio3(m_av_aCodecCtx,
                                                   (int16_t *)output_buf,
                                                   &generated_output_bytes,
                                                   &pkt);
#else
    DECLARE_ALIGNED(16, char, output_buf_data)[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
    output_buf             = output_buf_data;
    generated_output_bytes = sizeof(output_buf_data);
    processed_input_bytes  = avcodec_decode_audio2(m_av_aCodecCtx,
                                                   (int16_t *)output_buf,
                                                   &generated_output_bytes,
                                                   pkt.data,
                                                   pkt.size);
#endif


#ifdef DEBUG_DUMP_DECODER_STREAMS
    fprintf(m_debugMetaStream, "    processed input  chunk size %zi @ pos %zi\n",
            (size_t)processed_input_bytes,
            (size_t)m_encodedSize
           );
    fprintf(m_debugMetaStream, "    generated output chunk size %zi (%zi samples) @ pos %zi (%zi samples)\n",
            (size_t)(generated_output_bytes),
            (size_t)(generated_output_bytes / m_soundFormat.frameSize()),
            (size_t)(m_decodedSize),
            (size_t)(m_decodedSize          / m_soundFormat.frameSize())
           );
    fflush(m_debugMetaStream);
#endif

    m_encodedSize += (processed_input_bytes > 0) ? processed_input_bytes : 0;


    if (processed_input_bytes < 0) {
        /* if error, skip frame */
        log(ThreadLogging::LogWarning, i18n("%1: error decoding packet. Discarding packet",
#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
                         m_playURL.pathOrUrl()
#else
                         m_inputUrl.pathOrUrl()
#endif
                        ));
        return false;
    }
    else if (generated_output_bytes > 0) {

#ifdef DEBUG_DUMP_DECODER_STREAMS
        fwrite  (output_buf, (size_t)generated_output_bytes, 1, m_debugDecodedStream); fflush(m_debugDecodedStream);
#endif

        time_t cur_time = time(NULL);
        SoundMetaData  md(m_decodedSize,
                          cur_time - m_startTime,
                          cur_time,
                          m_i18nInternalStream
                         );

        if (!m_soundFormat.isValid()) {
            updateSoundFormat();
        }


#ifdef DEBUG_LOG_BUFFER_STATE
        printf ("free buffers: %i\n", m_bufferCountSemaphore.available());
        printf ("storing decoded data ...");
#endif

#ifndef DEBUG_DISCARD_DECODED_PCM_DATA
        pushBuffer(output_buf, generated_output_bytes, md, m_soundFormat);
#endif

#ifdef DEBUG_LOG_BUFFER_STATE
        printf (" done\n");
#endif

        m_decodedSize += generated_output_bytes;
    }

    return true;
}



bool InternetRadioDecoder::readFrame(AVPacket &pkt)
{
    int frame_read_res = av_read_frame(m_av_pFormatCtx, &pkt);

//     printf ("readFrame: res = %i", frame_read_res);

    if (frame_read_res < 0) {
        if (frame_read_res == (int)AVERROR_EOF || (m_av_pFormatCtx->pb && m_av_pFormatCtx->pb->eof_reached)) {
            m_done = true;
            return false;
        }
        if (m_av_pFormatCtx->pb && m_av_pFormatCtx->pb->error) {
            m_error = true;
            return false;
        }
        usleep(20000);
        return false;
    }
    return true;
}


void InternetRadioDecoder::run()
{
    while (!m_error && !m_done) {

#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
        selectStreamFromPlaylist();
        m_i18nInternalStream = i18n("internal stream, not stored (%1)", m_playURL.pathOrUrl());
#else
        openAVStream(m_inputUrl.pathOrUrl(), false);
        m_i18nInternalStream = i18n("internal stream, not stored (%1)", m_inputUrl.pathOrUrl());
#endif

        AVPacket    pkt;

#if  LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 42, 0) // checked: avcodec_decode_audio4 in ffmpeg >= 0.9
#if  LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 28, 1) // libav commit: https://gitorious.org/libav/libav/commit/d7b3ee9a3a03ab88d61a5895fbdbc6689f4dd671
        m_decoded_frame = av_frame_alloc();
#else
        m_decoded_frame = avcodec_alloc_frame();
#endif
        if (!m_decoded_frame) {
            m_error = true;
            log(ThreadLogging::LogError, i18n("Failed allocating AVFrame."));
        }
#endif

        m_startTime = time(NULL);

        while (!m_error && !m_done && m_decoderOpened) {

            if (!readFrame(pkt)) {
                continue;
            }

            if (!m_done && pkt.stream_index == m_av_audioStream) {

#ifdef DEBUG_DUMP_DECODER_STREAMS
                m_debugPacketCount++;
                fprintf(m_debugMetaStream,  "received packet nr. %zi (stream %i), size %zi\n", m_debugPacketCount, pkt.stream_index, (size_t)pkt.size); fflush(m_debugMetaStream);
                fwrite  (pkt.data, (size_t)pkt.size, 1, m_debugCodedStream); fflush(m_debugCodedStream);
                fprintf(m_debugMetaStream,  "   first 8 bytes (buf: %p): ", pkt.data);
                for (int i = 0; i < 8 ; ++i) {
                    fprintf(m_debugMetaStream, "%02x ", (unsigned char)pkt.data[i]);
                }
                fprintf(m_debugMetaStream, "\n"); fflush(m_debugMetaStream);
#endif

//                 AVDictionaryEntry *t = NULL;
//                 int                n = 0;
//                 while ((t = av_dict_get(m_av_pFormatCtx->streams[m_av_audioStream]->metadata, "", t, AV_DICT_IGNORE_SUFFIX))) {
//                     ++n;
//                 }
//                 IErrorLogClient::staticLogDebug(QString("stream metadata: %1 entries").arg(n));

                uint8_t *audio_pkt_org_data = pkt.data;
                int      audio_pkt_org_size = pkt.size;

                while (!m_error && !m_done && m_decoderOpened && (pkt.size > 0)) {
                    int processed_input_bytes = 0;
                    if (!decodePacket(pkt, processed_input_bytes)) {
                        break;
                    }
                    pkt.data += processed_input_bytes;
                    pkt.size -= processed_input_bytes;
                }

                // restore original ptrs in order to allow a proper freeing
                pkt.data = audio_pkt_org_data;
                pkt.size = audio_pkt_org_size;
            } else {
#ifdef DEBUG_DUMP_DECODER_STREAMS
                fprintf(m_debugMetaStream,  "discarded packet with size %zi\n", (size_t)pkt.size); fflush(m_debugMetaStream);
                fprintf(m_debugMetaStream,  "   first 8 bytes (buf: %p): ", pkt.data);
                for (int i = 0; i < 8 ; ++i) {
                    fprintf(m_debugMetaStream, "%02x ", (unsigned char)pkt.data[i]);
                }
                fprintf(m_debugMetaStream, "\n"); fflush(m_debugMetaStream);
                fflush(stdout);
                if (m_debugPacketCount >= 29289) {
                    exit (-1);
                }
#endif
            }

#if  LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 16, 0) // in ffmpeg >= 2.1
            av_packet_unref(&pkt);
#else
            av_free_packet(&pkt);
            memset(&pkt, 0, sizeof(pkt));
#endif
        //         printf ("waiting for next packet\n");
        }

        //     printf ("closing stream\n");
        closeAVStream();
#if  LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 42, 0) // checked: avcodec_decode_audio4 in ffmpeg >= 0.9
        av_free(m_decoded_frame);
        m_decoded_frame = NULL;
#endif
    }

#ifdef DEBUG_DUMP_DECODER_STREAMS
    fprintf(m_debugMetaStream, "left decoder thread main loop @ pos %zi, left %zi bytes in input buffer\n",
            (size_t)m_encodedSize,
            m_streamInputBuffer->debugBytesAvailable()
           );
    fflush(m_debugMetaStream);
#endif
    thread()->exit();
}


void InternetRadioDecoder::pushBuffer(const char *_data, size_t dataSize, const SoundMetaData &md, const SoundFormat &orgSF)
{
    if (m_done) {
        return;
    }

    SoundFormat sf = orgSF;
    sf.m_IsPlanar  = false;

    const char *data = _data;
    char        convBuf[dataSize];
    if (orgSF.m_IsPlanar) {
        data = convBuf;
        size_t nFrames = dataSize / orgSF.frameSize();
        orgSF.convertNonInterleavedToInterleaved(_data, convBuf, nFrames);
    }

    bool    foundBuffer       = false;
#ifdef DEBUG_DUMP_DECODER_STREAMS
    size_t  bufferFullSize    = 0;
    size_t  remainingCapacity = 0;
    size_t  remainingSize     = 0;
    size_t  nBufs             = 0;
#endif
    {   QMutexLocker lock(&m_bufferAccessLock);
        if (m_buffers.size() > 0) {
            DataBuffer &buf = m_buffers.last();
            if (buf.soundFormat() == sf && buf.remainingCapacity() >= dataSize) {
                buf.addData(data, dataSize);
#ifdef DEBUG_DUMP_DECODER_STREAMS
                bufferFullSize    = buf.fullSize();
                remainingCapacity = buf.remainingCapacity();
                remainingSize     = buf.remainingSize();
                nBufs             = m_buffers.size();
#endif
                foundBuffer = true;
            }
        }
    }
    if (!foundBuffer) {
        m_bufferCountSemaphore.acquire();
        QMutexLocker lock(&m_bufferAccessLock);
        m_buffers.push_back(DataBuffer(m_maxSingleBufferSize, data, dataSize, md, sf));
#ifdef DEBUG_DUMP_DECODER_STREAMS
        DataBuffer &buf = m_buffers.last();
        bufferFullSize    = buf.fullSize();
        remainingCapacity = buf.remainingCapacity();
        remainingSize     = buf.remainingSize();
        nBufs             = m_buffers.size();
#endif
    }
#ifdef DEBUG_DUMP_DECODER_STREAMS
    printf("wrote pcm buffer: nbufs=%zi, current: fullSize = %zi, remainingSize = %zi, remainingCapacity = %zi, increment = %zi\n", nBufs, bufferFullSize, remainingSize, remainingCapacity, dataSize);
#endif
}


int InternetRadioDecoder::availableBuffers()
{
    int n = 0;
    { QMutexLocker lock(&m_bufferAccessLock);
      n = m_buffers.size();
    }
    return n;
}


DataBuffer &InternetRadioDecoder::getFirstBuffer()
{
    DataBuffer *tmp = NULL;
    { QMutexLocker lock(&m_bufferAccessLock);
      tmp = &(m_buffers.first());
    }
    return *tmp;
}


void InternetRadioDecoder::popFirstBuffer()
{
    QMutexLocker lock(&m_bufferAccessLock);
    m_buffers.pop_front();
    m_bufferCountSemaphore.release();
}


void InternetRadioDecoder::flushBuffers()
{
    QMutexLocker lock(&m_bufferAccessLock);
    while (m_buffers.size()) {
        m_buffers.pop_front();
        m_bufferCountSemaphore.release();
    }
}


void InternetRadioDecoder::setDone()
{
    m_done = true;
    // guarantee that all blocking functions return from their locking status
    flushBuffers();
#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    m_streamInputBuffer->resetBuffer();
//     m_inputBufferSize.release(m_maxProbeSize);
#endif
}


#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
static int mms_read_packet(void *opaque, uint8_t *buf, int buf_size)
{
    return mmsx_read(NULL, (mmsx_t*)opaque, (char*)buf, buf_size);
}
#else
static int InternetRadioDecoder_readInputBuffer(void *opaque, uint8_t *buffer, int max_size);
#endif



void InternetRadioDecoder::initIOCallbacks(void *opaque, int(*read_packet_func)(void *, uint8_t *, int ))
{
    // paranoia padding: 4x requirement
    unsigned char *ioBuffer = reinterpret_cast<unsigned char*>(av_malloc(DEFAULT_MMS_BUFFER_SIZE + 4 * AV_INPUT_BUFFER_PADDING_SIZE));
    
#if  LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(52, 110, 0) // checked: avformat_open_input in ffmpeg >= 0.7

    m_av_byteio_contextPtr = avio_alloc_context(ioBuffer,
                                                DEFAULT_MMS_BUFFER_SIZE,
                                                /*write_flag = */ false,
                                                opaque, // m_streamInputBuffer,
                                                read_packet_func, //&InternetRadioDecoder_readInputBuffer,
                                                NULL,
                                                NULL
                                               );
    m_av_byteio_contextPtr->seekable = 0;

#else

    init_put_byte(&m_av_byteio_context,
                  ioBuffer,
                  DEFAULT_MMS_BUFFER_SIZE,            // sizeof(m_mms_buffer),
                  false,
                  opaque,
                  &read_packet_func,
                  NULL,
                  NULL
                 );

#endif
}



void InternetRadioDecoder::open_av_input(AVInputFormat *iformat, const QString &stream, bool warningsNotErrors, bool use_io_context)
{
    
#if  LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(52, 110, 0) // checked: avformat_open_input in ffmpeg >= 0.7
//     AVDictionary  *av_params = NULL;
    //av_dict_set(&av_params, "key", "value", 0);
#else
    AVFormatParameters av_params;
    memset(&av_params, 0, sizeof(av_params));
    av_params.prealloced_context = 1;
#endif

    int err = -1;

#if  LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(52, 110, 0) // checked: avformat_open_input in ffmpeg >= 0.7
    if (use_io_context) {
        m_av_pFormatCtx->pb = m_av_byteio_contextPtr;
    }
    err = avformat_open_input(&m_av_pFormatCtx, stream.toUtf8().constData(), iformat, NULL);

    if (err != 0) { // on failure the context is automatically freed. Let's guarantee that the pointer is also nulled
        m_av_pFormatCtx        = NULL;
        m_av_pFormatCtx_opened = false;
    }
#else
    if (use_io_context) {
        err = av_open_input_stream(&m_av_pFormatCtx, &m_av_byteio_context, stream.toUtf8().constData(), iformat, &av_params);
    } else {
        err = av_open_input_file  (&m_av_pFormatCtx, stream.toUtf8().constData(), iformat, 0, &av_params);
    }
#endif

    if (err != 0) {
        if (warningsNotErrors) {
            log(ThreadLogging::LogWarning, i18n("Could not open stream %1", stream)); //m_inputUrl.pathOrUrl()));
        } else {
            m_error = true;
            log(ThreadLogging::LogError, i18n("Could not open stream %1", m_inputUrl.pathOrUrl()));
        }
        closeAVStream();
    } else {
        m_av_pFormatCtx_opened = true;
    }

#if  LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(52, 110, 0) // checked: avformat_open_input in ffmpeg >= 0.7
//     av_dict_free(&av_params);
#endif
}



AVInputFormat *InternetRadioDecoder::getInputFormat(const QString &fallbackFormat, bool warningsNotErrors)
{
    LibAVGlobal::ensureInitDone();
    AVInputFormat   *iformat      = av_find_input_format(m_RadioStation.decoderClass().toLatin1().constData());
    if (!iformat) {
        QByteArray decoderClass;
        if (m_contentType == QLatin1String("audio/mpeg") ||
            m_contentType == QLatin1String("audio/x-mpeg") ||
            m_contentType == QLatin1String("audio/mp3") ||
            m_contentType == QLatin1String("audio/x-mp3") ||
            m_contentType == QLatin1String("video/mpeg") ||
            m_contentType == QLatin1String("video/x-mpeg")) {
            decoderClass = "mp3";
        } else if (m_contentType == QLatin1String("application/x-ogg") ||
                   m_contentType == QLatin1String("application/ogg") ||
                   m_contentType == QLatin1String("audio/ogg") ||
                   m_contentType == QLatin1String("video/ogg")) {
            decoderClass = "ogg";
        } else if (m_contentType == QLatin1String("application/flv")) {
            decoderClass = "FLV";
        } else if (m_contentType == QLatin1String("audio/asf") ||
                   m_contentType == QLatin1String("video/x-ms-asf") ||
                   m_contentType == QLatin1String("application/x-ms-asf-plugin")) {
            decoderClass = "asf";
        }
        if (decoderClass.length()) {
            iformat = av_find_input_format(decoderClass.constData());
            if (iformat) {
                log(ThreadLogging::LogDebug, QString("found content-type = \"%1\": skipping auto detection and setting decoder class to \"%2\"").arg(m_contentType).arg(decoderClass.constData()));
            }
        }
    }


#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD

    // run autodetection if format is not known
    if (!iformat) {

        int        score     = 0;
        bool       err       = false;
        QByteArray probeData = m_streamInputBuffer->readInputBuffer(m_maxProbeSize, m_maxProbeSize, /* consume */ false, err);
//         printf("probe data size = %i\n", probeData.size());

        if (!err) {
            const QByteArray path = QFile::encodeName(m_inputUrl.pathOrUrl());
            
#if  (LIBAVFORMAT_VERSION_MAJOR > 55) // ffmpeg commit 5482780a3b6ef0a8934cf29aa7e2f1ef7ccb701e
            AVProbeData pd = { path.constData(), (unsigned char*)probeData.data(), probeData.size(), NULL };
#else
            AVProbeData pd = { path.constData(), (unsigned char*)probeData.data(), probeData.size()       };
#endif

            iformat = av_probe_input_format2(&pd, 1, &score);
        }

        if (!iformat) {
            if (warningsNotErrors) {
                log(ThreadLogging::LogWarning, i18n("Autodetect of stream type failed for %1", m_inputUrl.pathOrUrl()));
            } else {
                m_error = true;
                log(ThreadLogging::LogError, i18n("Autodetect of stream type failed for %1", m_inputUrl.pathOrUrl()));
            }
            closeAVStream();
        } else {
            log(ThreadLogging::LogDebug, i18n("Autodetected format: %1 (score %2)", iformat->long_name, score));
        }
    }
#else // INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD defined below
#endif

    if (!iformat && fallbackFormat.length()) {
        iformat = av_find_input_format(fallbackFormat.toLatin1().constData());
    }

    return iformat;
}



bool InternetRadioDecoder::retrieveStreamInformation(const QString &stream, bool warningsNotErrors)
{
    // Retrieve stream information
    int err = -1;
#if  LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(52, 111, 0) // checked: avformat_find_stream_info in ffmpeg >= 0.7.8
    err = avformat_find_stream_info(m_av_pFormatCtx, NULL);
#else
    err = av_find_stream_info(m_av_pFormatCtx);
#endif
    if (err < 0) {
        if (warningsNotErrors) {
            log(ThreadLogging::LogWarning, i18n("Could not find stream information in %1", stream));
        } else {
            m_error = true;
            log(ThreadLogging::LogError, i18n("Could not find stream information in %1", stream));
        }
        closeAVStream();
        return false; // Couldn't find stream information
    }

//     IErrorLogClient::staticLogDebug("InternetRadioDecoder::openAVStream: av_find_stream_info done");

//     IErrorLogClient::staticLogDebug(QString("InternetRadioDecoder::openAVStream: Title == %1").arg(m_av_pFormatCtx->title));
//     IErrorLogClient::staticLogDebug(QString("InternetRadioDecoder::openAVStream: Author == %1").arg(m_av_pFormatCtx->author));
//     IErrorLogClient::staticLogDebug(QString("InternetRadioDecoder::openAVStream: Copyright == %1").arg(m_av_pFormatCtx->copyright));
//     IErrorLogClient::staticLogDebug(QString("InternetRadioDecoder::openAVStream: Comment == %1").arg(m_av_pFormatCtx->comment));
//
//     AVMetadata    *metadata     = m_av_pFormatCtx->metadata;
//     AVMetadataTag *metadata_tag = NULL;
//
//     while((metadata_tag = av_metadata_get(metadata, "", metadata_tag, AV_METADATA_IGNORE_SUFFIX))) {
//         IErrorLogClient::staticLogDebug(QString("InternetRadioDecoder::openAVStream: MetadataTag: %1 => %2").arg(metadata_tag->key).arg(metadata_tag->value));
//     }
//
//     for (unsigned int i = 0; i < m_av_pFormatCtx->nb_chapters; i++) {
//         AVChapter *ch = m_av_pFormatCtx->chapters[i];
//         IErrorLogClient::staticLogDebug(QString("InternetRadioDecoder::openAVStream: Chapter %1").arg(i));
//         IErrorLogClient::staticLogDebug(QString("InternetRadioDecoder::openAVStream:     start: %1").arg(ch->start * av_q2d(ch->time_base)));
//         IErrorLogClient::staticLogDebug(QString("InternetRadioDecoder::openAVStream:     end:   %1").arg(ch->end   * av_q2d(ch->time_base)));
//         metadata_tag = NULL;
//         while((metadata_tag = av_metadata_get(ch->metadata, "", metadata_tag, AV_METADATA_IGNORE_SUFFIX))) {
//             IErrorLogClient::staticLogDebug(QString("InternetRadioDecoder::openAVStream:     MetadataTag: %1 => %2").arg(metadata_tag->key).arg(metadata_tag->value));
//         }
//     }
//
//     dump_format(m_av_pFormatCtx, 0, stream.toUtf8(), false);

    for (unsigned int i = 0; i < m_av_pFormatCtx->nb_streams; i++) {
        int rate = m_av_pFormatCtx->streams[i]->codec->sample_rate;
        int ch   = m_av_pFormatCtx->streams[i]->codec->channels;
        int fmt  = m_av_pFormatCtx->streams[i]->codec->sample_fmt;
        int type = m_av_pFormatCtx->streams[i]->codec->codec_type;
        log(ThreadLogging::LogDebug, QString::fromLatin1("stream[%1]: codec_type = %2, channels = %3, sample rate = %4, format-id = %5").arg(i).arg(type).arg(ch).arg(rate).arg(fmt));
    }

#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(52, 122, 0) // checked: av_find_best_stream in ffmpeg >= 0.7
    m_av_audioStream = av_find_best_stream(
        m_av_pFormatCtx,
        AVMEDIA_TYPE_AUDIO,
        -1,
        -1,
        &m_av_aCodec,
        0
    );
#else // LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(52, 122, 0)
    m_av_audioStream = -1;
    for (unsigned int i = 0; i < m_av_pFormatCtx->nb_streams; i++) {
#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(50, 15, 1) // checked: AVMEDIA_TYPE_AUDIO in ffmpeg >= 0.6
        if (m_av_pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) { // take last stream
#else
        if (m_av_pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO) { // take last stream
#endif
            m_av_audioStream = i;
        }
    }
#endif // LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(52, 122, 0)

    if (m_av_audioStream == -1) {
        if (warningsNotErrors) {
            log(ThreadLogging::LogWarning, i18n("Could not find an audio stream in %1", stream));
        } else {
            m_error = true;
            log(ThreadLogging::LogError, i18n("Could not find an audio stream in %1", stream));
        }
        closeAVStream();
        return false;
    }
    updateSoundFormat();
    return m_av_pFormatCtx_opened;
}



bool InternetRadioDecoder::openCodec(const QString &stream, bool warningsNotErrors)
{
    m_av_aCodecCtx = m_av_pFormatCtx->streams[m_av_audioStream]->codec;

    if (!m_av_aCodec) {
        m_av_aCodec = avcodec_find_decoder(m_av_aCodecCtx->codec_id);
    }
    if (!m_av_aCodec) {
        if (warningsNotErrors) {
            log(ThreadLogging::LogWarning, i18n("Could not find a codec for %1", stream));
        } else {
            m_error = true;
            log(ThreadLogging::LogError,   i18n("Could not find a codec for %1", stream));
        }
        closeAVStream();
        return false;
    }

#if  LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 123, 0) // checked: avcodec_open2 in ffmpeg >= 0.7.8
    AVDictionary *codecOpts = NULL;
    av_dict_set(&codecOpts, "threads", "auto", 0);
    int err = avcodec_open2(m_av_aCodecCtx, m_av_aCodec, &codecOpts);
#else
    int err = avcodec_open(m_av_aCodecCtx, m_av_aCodec);
#endif
    if (err < 0) {
        if (warningsNotErrors) {
            log(ThreadLogging::LogWarning, i18n("Opening codec for %1 failed", stream));
        } else {
            m_error = true;
            log(ThreadLogging::LogError, i18n("Opening codec for %1 failed", stream));
        }
        closeAVStream();
        return false;
    }
    
    
#if defined(HAVE_LIBAVRESAMPLE) || defined (HAVE_LIBSWRESAMPLE)
    // Set up SWR context once you've got codec information
#ifdef HAVE_LIBAVRESAMPLE
    m_resample_context = avresample_alloc_context();
#elif defined HAVE_LIBSWRESAMPLE
    m_resample_context = swr_alloc();
#endif
    uint64_t chLayout = m_av_aCodecCtx->channel_layout;
    if (!chLayout) {
        switch(m_av_pFormatCtx->streams[m_av_audioStream]->codec->channels) {
            case 1:
                chLayout = AV_CH_LAYOUT_MONO;
                break;
            case 2:
                chLayout = AV_CH_LAYOUT_STEREO;
                break;
            default:
                m_error = true;
                log(ThreadLogging::LogError, i18n("Unknown channel layout for %1", stream));
                closeAVStream();
                break;
        }
    }
    if (!m_error) {
        av_opt_set_int       (m_resample_context, "in_channel_layout",  chLayout,                       0);
        av_opt_set_int       (m_resample_context, "out_channel_layout", chLayout,                       0);
        av_opt_set_int       (m_resample_context, "in_sample_rate",     m_av_aCodecCtx->sample_rate,    0);
        av_opt_set_int       (m_resample_context, "out_sample_rate",    m_av_aCodecCtx->sample_rate,    0);
#ifdef HAVE_LIBAVRESAMPLE
        av_opt_set_int       (m_resample_context, "in_sample_fmt",      m_av_pFormatCtx->streams[m_av_audioStream]->codec->sample_fmt, 0);
        av_opt_set_int       (m_resample_context, "out_sample_fmt",     AV_SAMPLE_FMT_S16,              0);
#elif defined HAVE_LIBSWRESAMPLE
        av_opt_set_sample_fmt(m_resample_context, "in_sample_fmt",      m_av_pFormatCtx->streams[m_av_audioStream]->codec->sample_fmt, 0);
        av_opt_set_sample_fmt(m_resample_context, "out_sample_fmt",     AV_SAMPLE_FMT_S16,              0);
#endif

#ifdef HAVE_LIBAVRESAMPLE
        avresample_open(m_resample_context);
#elif defined HAVE_LIBSWRESAMPLE
        swr_init(m_resample_context);
#endif
    }
#endif

    if (!m_soundFormat.isValid()) {
        updateSoundFormat();
    }
    return true;
}


void InternetRadioDecoder::openAVStream(const QString &stream, bool warningsNotErrors)
{
    if (m_decoderOpened) {
        return;
    }

#ifdef DEBUG_DUMP_DECODER_STREAMS
    m_debugPacketCount = -1;
#endif

    // care a bit about maximum delay when opening and autodetecting the stream:
    // two effects:
    //   * av_open_input_{stream,file}   has some autodetection which tries to get
    //                                   some quite high score, which may require loading
    //                                   e.g. 256kB (WDR5) of data... that takes time...
    //                                   However, less data (~8kB == 1 sec @ 64kBit) should be sufficient
    //                                   This time can be tuned by m_av_pFormatCtx->probesize
    //   * av_find_stream_info           is also looking for some extra data before starting playback...
    //                                   this time can be tuned by m_av_pFormatCtx->max_analyze_duration


    //av_log_set_level(255);
    m_av_pFormatCtx = avformat_alloc_context();
    m_av_pFormatCtx->probesize = m_maxProbeSize;

    // FIXME: divergence between mpeg.org and libav.org
    // ffmpeg.org:  current:    max_analyze_duration2
    //              deprecated: max_analyze_duration
    //              LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(55, 43, 100)
    //              commit: 5482780a3b6ef0a8934cf29aa7e2f1ef7ccb701e
    // libav.org:   current/master:    max_analyze_duration
    //              no change in any branch/release, at least up to
    //              LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(56, 6, 0)
    //
    // As long as libav does not switch to max_analyze_duration2, and as
    // long as we cannot distinguish libav.org and libffmpeg, we need to
    // stay with max_analyze_duration
    m_av_pFormatCtx->max_analyze_duration  = m_maxAnalyzeTime * AV_TIME_BASE;

// #if  LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(55, 43, 100) // ffmpeg commit 5482780a3b6ef0a8934cf29aa7e2f1ef7ccb701e
//     m_av_pFormatCtx->max_analyze_duration2 = m_maxAnalyzeTime * AV_TIME_BASE;
// #else
//     m_av_pFormatCtx->max_analyze_duration  = m_maxAnalyzeTime * AV_TIME_BASE;
// #endif

    m_av_pFormatCtx_opened = false;
#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    m_is_mms_stream        = false;
#endif
    // if a format has been specified, set up the proper structures



#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD

    initIOCallbacks(m_streamInputBuffer, &InternetRadioDecoder_readInputBuffer);

    AVInputFormat   *iformat = getInputFormat("", warningsNotErrors);

    if (iformat) { // format setup / detection worked well ==> open stream
        open_av_input(iformat, m_inputUrl.pathOrUrl(), warningsNotErrors, /* use_io_context = */ true);
    }
#else // INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD defined below


    // load asf stream
    KUrl  streamUrl = stream;
    if (streamUrl.protocol().startsWith("mms")) {
        m_is_mms_stream = true;
        m_mms_stream = mmsx_connect(NULL, NULL, stream.toUtf8().constData(), 1);
        if (!m_mms_stream) {
            if (warningsNotErrors) {
                log(ThreadLogging::LogWarning, i18n("cannot open MMS stream %1", stream));
            } else {
                m_error = true;
                log(ThreadLogging::LogError, i18n("cannot open MMS stream %1", stream));
            }
            closeAVStream();
        } else {
            initIOCallbacks(m_mms_stream, &mms_read_packet);
        }

        AVInputFormat   *iformat = getInputFormat("asf", warningsNotErrors);

        if (iformat && m_mms_stream) {
            open_av_input(iformat, stream, warningsNotErrors, /* use_io_context = */ true);
        }
    }
    // else: here stream is not mms/mmsx
    else {
        AVInputFormat   *iformat = getInputFormat("", warningsNotErrors);

//         IErrorLogClient::staticLogDebug("InternetRadioDecoder::openAVStream: av_open_input_file start");
        open_av_input(iformat, stream, warningsNotErrors, /* use_io_context = */ false);
//         IErrorLogClient::staticLogDebug("InternetRadioDecoder::openAVStream: av_open_input_file done");
    }

//     IErrorLogClient::staticLogDebug("InternetRadioDecoder::openAVStream: av_find_stream_info start");
#endif // INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD




    if (!m_av_pFormatCtx_opened) {
        return;
    }

    if (!retrieveStreamInformation(stream, warningsNotErrors)) {
        return;
    }

    if (!openCodec(stream, warningsNotErrors)) {
        return;
    }
    m_decoderOpened = true;
}



void InternetRadioDecoder::updateSoundFormat()
{
    if (m_av_pFormatCtx &&
        m_av_audioStream                 >= 0                &&
        (int)m_av_pFormatCtx->nb_streams >  m_av_audioStream &&
        m_av_pFormatCtx->streams[m_av_audioStream]      &&
        m_av_pFormatCtx->streams[m_av_audioStream]->codec
    ) {
        int  rate = m_av_pFormatCtx->streams[m_av_audioStream]->codec->sample_rate;
        int  ch   = m_av_pFormatCtx->streams[m_av_audioStream]->codec->channels;
        int  fmt  = m_av_pFormatCtx->streams[m_av_audioStream]->codec->sample_fmt;
#if defined(HAVE_LIBAVRESAMPLE) || defined (HAVE_LIBSWRESAMPLE)
        fmt       = AV_SAMPLE_FMT_S16;
#endif
        int  bits     = 0;
        int  issigned = 0;
        bool isplanar = false;
        switch(fmt) {
            case AV_SAMPLE_FMT_U8:
                bits     = 8;
                issigned = false;
                isplanar = false;
                break;
            case AV_SAMPLE_FMT_S16:
                bits     = 16;
                issigned = true;
                isplanar = false;
                break;
            case AV_SAMPLE_FMT_S32:
                bits     = 32;
                issigned = true;
                isplanar = false;
                break;
#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(50, 15, 1) // checked: first in ffmpeg >= 0.9
            case AV_SAMPLE_FMT_U8P:
                bits     = 8;
                issigned = false;
                isplanar = true;
                break;
            case AV_SAMPLE_FMT_S16P:
                bits     = 16;
                issigned = true;
                isplanar = true;
                break;
            case AV_SAMPLE_FMT_S32P:
                bits     = 32;
                issigned = true;
                isplanar = true;
                break;
            case AV_SAMPLE_FMT_FLT:
            case AV_SAMPLE_FMT_DBL:
            case AV_SAMPLE_FMT_FLTP:
            case AV_SAMPLE_FMT_DBLP:
                m_error = true;
                log(ThreadLogging::LogError, i18n("Not yet implemented: libav sample format id %1", fmt));
                closeAVStream();
                return;
#endif
            default:
                m_error = true;
                log(ThreadLogging::LogError, i18n("Cannot use libav sample format id %1", fmt));
                closeAVStream();
                return;
        }
        m_soundFormat = SoundFormat(rate, ch, bits, issigned, isplanar);
    }
}


void InternetRadioDecoder::freeAVIOContext()
{
#if  LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(52, 110, 0) // checked: avformat_open_input in ffmpeg >= 0.7
    if (m_av_byteio_contextPtr) {
        if (m_av_byteio_contextPtr->buffer) {
            av_free(m_av_byteio_contextPtr->buffer);
            m_av_byteio_contextPtr->buffer = NULL;
        }
        av_free(m_av_byteio_contextPtr);
        m_av_byteio_contextPtr = NULL;
    }
#else
    if (m_av_byteio_context.buffer) {
        av_free(m_av_byteio_context.buffer);
        m_av_byteio_context.buffer = NULL;
    }
#endif
    if (m_av_pFormatCtx) {
        m_av_pFormatCtx->pb = NULL;
    }
}


void InternetRadioDecoder::closeAVStream()
{
    if (m_av_aCodecCtx) {
        avcodec_close(m_av_aCodecCtx);
    }
#ifdef HAVE_LIBAVRESAMPLE
    if (m_resample_context) {
        avresample_close(m_resample_context);
        avresample_free(&m_resample_context);
        m_resample_context = NULL;
    }
#elif defined HAVE_LIBSWRESAMPLE
    if (m_resample_context) {
        swr_free(&m_resample_context);
        m_resample_context = NULL;
    }
#endif

    freeAVIOContext();

    // if stream was not opened but if the context exists, then free it here
    // otherwise the avformat_close_input function calls will do the job
    if (!m_av_pFormatCtx_opened && m_av_pFormatCtx) {
        av_free(m_av_pFormatCtx);
        m_av_pFormatCtx        = NULL;
        m_av_pFormatCtx_opened = false;
    }

#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    if (m_is_mms_stream) {
#endif
        if (m_av_pFormatCtx) {
#if  LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53, 24, 2) // checked: avformat_close_input in ffmpeg >= 0.9.1
            avformat_close_input(&m_av_pFormatCtx);
#else
            av_close_input_stream(m_av_pFormatCtx);
#endif
        }
#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
        if (m_mms_stream) {
            mmsx_close(m_mms_stream);
        }
    } else if (m_av_pFormatCtx) {
#if  LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53, 24, 2) // checked: avformat_close_input in ffmpeg >= 0.9.1
        avformat_close_input(&m_av_pFormatCtx);
#else
        av_close_input_file(m_av_pFormatCtx);
#endif
    }
    m_mms_stream     = NULL;
#endif

    m_av_pFormatCtx        = NULL;
    m_av_pFormatCtx_opened = NULL;
    m_av_audioStream       = -1;
    m_av_aCodecCtx         = NULL;
    m_av_aCodec            = NULL;
    m_decoderOpened        = false;
}


#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD

static int InternetRadioDecoder_readInputBuffer(void *opaque, uint8_t *buffer, int max_size)
{
#ifdef DEBUG_DUMP_DECODER_STREAMS
    static FILE   *debugFH = NULL;
    static size_t  pos     = 0;
    if (!debugFH) {
        debugFH = fopen("/tmp/kradio-decoder-thread-input-stream.bin", "w");
        printf("InternetRadioDecoder_readInputBuffer: opened input-stream debug file\n");
    }
//     if (pos >= 0x808000) {
//         printf("dummy\n");
//     }
#endif

    StreamInputBuffer *x = static_cast<StreamInputBuffer*>(opaque);
    bool               err = false;
    QByteArray tmp = x->readInputBuffer(1024, max_size, /* consume */ true, err); // at least a kB
    if (!err) {
        const unsigned char *rxBuf   = (const unsigned char*)tmp.constData();
        size_t               bufsize = tmp.size();
        memcpy(buffer, rxBuf, bufsize);

#ifdef DEBUG_DUMP_DECODER_STREAMS
        printf ("input stream read @ %zi (0x%08zx), rxBuf = %p, returned %zi bytes (0x%08zx), err = %i, buf=%p, maxSize = 0x%08x\n", pos, pos, rxBuf, bufsize, bufsize, err, buffer, max_size);
        printf ("  first 16 bytes: ");
        for (int i = 0; i < 16; ++i) {
            printf("%02x ", buffer[i]);
        }
        printf("\n"); fflush(stdout);
        printf ("  last 17 bytes:  ");
        for (int i = -17; i < 0; ++i) {
            printf("%02x ", buffer[bufsize + i]);
        }
        printf("\n"); fflush(stdout);
        fwrite(tmp.constData(), bufsize, 1, debugFH); fflush(debugFH);
        pos += tmp.size();
#endif
        return tmp.size();
    } else {
        return -1;
    }
}

#endif









DecoderThread::DecoderThread(QObject *parent,
                             const InternetRadioStation &rs,
#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
                             const KUrl::List           &playlist,
#else
                             const KUrl                 &currentStreamUrl,
                             StreamReader               *streamReader,
//                              StreamInputBuffer          *streamInputBuffer,
#endif
                             int   input_buffer_size,
                             int   max_output_buffers,
                             int   max_output_buffer_chunk_size,
                             int   max_probe_size_bytes,
                             float max_analyze_secs,
                             int   max_retries
                            )
    : QThread(parent),
      m_station(rs),
      m_max_buffers(max_output_buffers),
      m_max_singleBufferSize(max_output_buffer_chunk_size),
      m_max_probe_size_bytes(max_probe_size_bytes),
      m_max_analyze_secs(max_analyze_secs),
      m_max_retries(max_retries),
      m_decoder(NULL)
#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
      , m_playlist(playlist)
#else
      , m_currentStreamUrl (currentStreamUrl)
//       , m_streamInputBuffer(streamInputBuffer)
#endif
{
#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    m_streamInputBuffer = new StreamInputBuffer(input_buffer_size);
    QObject::connect(streamReader,        SIGNAL(sigStreamData(QByteArray)), m_streamInputBuffer, SLOT(slotWriteInputBuffer(QByteArray)));
    QObject::connect(m_streamInputBuffer, SIGNAL(sigInputBufferFull()),      streamReader,        SLOT(slotStreamPause()));
    QObject::connect(m_streamInputBuffer, SIGNAL(sigInputBufferNotFull()),   streamReader,        SLOT(slotStreamContinue()));
    KIO::MetaData md = streamReader->getConnectionMetaData();
    if (md.contains("content-type")) {
        m_contentType = md["content-type"];
    }
#endif
    setTerminationEnabled(true);
}


DecoderThread::~DecoderThread()
{
    if (m_decoder) {
        delete m_decoder;
        m_decoder = NULL;
    }
    m_streamInputBuffer->resetBuffer();
    m_streamInputBuffer->deleteLater();
    m_streamInputBuffer = NULL;
    IErrorLogClient::staticLogDebug("DecoderThread::~DecoderThread()");
}


void DecoderThread::run()
{
    setTerminationEnabled(true);
    m_decoder = new InternetRadioDecoder(parent(),
                                         m_station,
#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
                                         m_playlist,
#else
                                         m_currentStreamUrl,
                                         m_streamInputBuffer,
                                         m_contentType,
#endif
                                         m_max_buffers,
                                         m_max_singleBufferSize,
                                         m_max_probe_size_bytes,
                                         m_max_analyze_secs,
                                         m_max_retries
                                        );
    exec();
    m_streamInputBuffer->resetBuffer();
    exit();
}



