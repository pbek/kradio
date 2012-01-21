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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>
#include <unistd.h>

#include <QtGui/QApplication>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QAbstractEventDispatcher>

#include <kio/jobclasses.h>
#include <kio/netaccess.h>
#include <kio/job.h>
#include <kconfig.h>
#include <kconfiggroup.h>

/*#include "soundstream_decoding_step_event.h"*/
#include "soundstream_decoding_terminate_event.h"
#include "decoder_thread.h"
#include "errorlog_interfaces.h"

#include "libav-global.h"

InternetRadioDecoder::InternetRadioDecoder(QObject *event_parent,
                                           const InternetRadioStation &rs,
#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
                                           const KUrl::List &playlist,
#endif
                                           int max_buffers,
                                           int max_probe_size_bytes, float max_analyze_secs)
:   m_decoderOpened (false),
    m_av_pFormatCtx (NULL),
    m_av_pFormatCtx_opened(false),
    m_av_audioStream(-1),
    m_av_aCodecCtx  (NULL),
    m_av_aCodec     (NULL),

#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    m_is_mms_stream (false),
    m_mms_stream    (NULL),
#endif

    m_parent        (event_parent),
    m_RadioStation  (rs),

    m_error         (false),
    m_errorString   (QString()),
    m_warning       (false),
    m_warningString (QString()),
    m_debug         (false),
    m_debugString   (QString()),
    m_modErrWarnSemaphore(1),
    m_done          (false),

    m_decodedSize   (0),

#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    m_playListURLs  (playlist),
#endif
/*    m_inputURL      (m_RadioStation.url()),*/
    m_bufferCountSemaphore(max_buffers),
#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    m_inputBufferMaxSize(65536),
    m_inputBufferSize(0),
#endif
    m_maxProbeSize  (max_probe_size_bytes > 1024 ? max_probe_size_bytes : 4096),
    m_maxAnalyzeTime(max_analyze_secs > 0.01     ? max_analyze_secs     : 0.5)
{
//     IErrorLogClient::staticLogDebug(QString().sprintf("InternetRadioDecoder::InternetRadioDecoder: this->thread() = %012p",             thread()));
//     IErrorLogClient::staticLogDebug(QString().sprintf("InternetRadioDecoder::InternetRadioDecoder: dispatcher for this thread = %012p", QAbstractEventDispatcher::instance()));

#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    QObject::connect(this, SIGNAL(sigInputBufferNotFull()), m_parent, SLOT(slotStreamContinue()));
#endif
    QObject::connect(this, SIGNAL(sigSelfTrigger()), this, SLOT(run()), Qt::QueuedConnection);
    emit sigSelfTrigger();
}

InternetRadioDecoder::~InternetRadioDecoder()
{
    flushBuffers();
    closeAVStream();
}




void InternetRadioDecoder::run()
{
//     IErrorLogClient::staticLogDebug(QString().sprintf("InternetRadioDecoder::run: this->thread() = %012p",             thread()));
//     IErrorLogClient::staticLogDebug(QString().sprintf("InternetRadioDecoder::run: dispatcher for this thread = %012p", QAbstractEventDispatcher::instance(this)));

//     loadPlaylist();

    while (!m_error && !m_done) {

#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
        int retries = 2;
        unsigned int start_play_idx = (unsigned int) rint((m_playListURLs.size()-1) * (float)rand() / (float)RAND_MAX);
        int n = m_playListURLs.size();
        for (int i = 0; !m_decoderOpened && i < n; ++i) {
            m_playURL = m_playListURLs[(start_play_idx + i) % n];
            for (int j = 0; j < retries && !m_decoderOpened; ++j) {
                addDebugString(i18n("opening stream %1", m_playURL.pathOrUrl()));
                openAVStream(m_playURL.pathOrUrl(), true);
            }
            if (!m_decoderOpened) {
                addWarningString(i18n("failed to open %1. Trying next stream in play list.", m_playURL.pathOrUrl()));
            }
        }
        if (!m_decoderOpened) {
            addErrorString(i18n("Could not open any input stream of %1.", m_RadioStation.url().pathOrUrl()));
        }
#else
        openAVStream(m_inputUrl.pathOrUrl(), true);
#endif

        AVPacket pkt;

        time_t  start_time      = time(NULL);

        while (!m_error && !m_done && m_decoderOpened) {

            int frame_read_res = av_read_frame(m_av_pFormatCtx, &pkt);
            if (frame_read_res < 0) {
                if (frame_read_res == (int)AVERROR_EOF || (m_av_pFormatCtx->pb && m_av_pFormatCtx->pb->eof_reached)) {
                    m_done = true;
                    break;
                }
                if (m_av_pFormatCtx->pb && m_av_pFormatCtx->pb->error) {
                    m_error = true;
                    break;
                }
                usleep(50000);
                continue;
            }

            if (!m_done && pkt.stream_index == m_av_audioStream) {

                uint8_t *audio_pkt_data = pkt.data;
                int      audio_pkt_size = pkt.size;

                while (!m_error && !m_done && m_decoderOpened && (audio_pkt_size > 0)) {
                    char output_buf[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
                    int  generated_output_bytes = sizeof(output_buf);

    #if  LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(52, 34, 0)
                    int  processed_input_bytes = avcodec_decode_audio3(m_av_aCodecCtx,
                                                                    (int16_t *)output_buf,
                                                                    &generated_output_bytes,
                                                                    &pkt);
    #else
                    int  processed_input_bytes = avcodec_decode_audio2(m_av_aCodecCtx,
                                                                    (int16_t *)output_buf,
                                                                    &generated_output_bytes,
                                                                    audio_pkt_data,
                                                                    audio_pkt_size);
    #endif
                    if (processed_input_bytes < 0) {
                        /* if error, skip frame */
                        addWarningString(i18n("%1: error decoding packet. Discarding packet\n")
#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
                                         .arg(m_playURL.pathOrUrl())
#else
                                         .arg(m_inputUrl.pathOrUrl())
#endif
                                        );
                        break;
                    }
                    else if (generated_output_bytes > 0) {
                        time_t cur_time = time(NULL);
                        SoundMetaData  md(m_decodedSize,
                                          cur_time - start_time,
                                          cur_time,
                                          i18n("internal stream, not stored (%1)",
#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
                                               m_playURL.pathOrUrl()
#else
                                               m_inputUrl.pathOrUrl()
#endif
                                              )
                                         );

                        DataBuffer buf(output_buf, generated_output_bytes, md, m_soundFormat);

    //                     printf ("free buffers: %i\n", m_bufferCountSemaphore.available());
    //                     printf ("storing decoded data ...");
                        pushBuffer(buf);
    //                     printf (" done\n");

    /*                    printf ("semaphore available: %i\n", m_runSemaphore.available());
                        m_runSemaphore.acquire(1);
                        if (!m_done) {
                            SoundStreamDecodingStepEvent *e = new SoundStreamDecodingStepEvent(m_soundFormat, output_buf, generated_output_bytes, md);
                            QApplication::postEvent(m_parent, e);
                        }*/
                        m_decodedSize += generated_output_bytes;
                    }

                    audio_pkt_data += processed_input_bytes;
                    audio_pkt_size -= processed_input_bytes;

                }
            }

            av_free_packet(&pkt);
        //         printf ("waiting for next packet\n");
        }

        //     printf ("closing stream\n");
        closeAVStream();
    }

//     printf ("posting event\n");

    thread()->exit();
}


void InternetRadioDecoder::pushBuffer(const DataBuffer &buf)
{
    if (m_done) {
        return;
    }
    m_bufferCountSemaphore.acquire();
    QMutexLocker lock(&m_bufferAccessLock);
    m_buffers.push_back(buf);
}


int InternetRadioDecoder::availableBuffers()
{
    QMutexLocker lock(&m_bufferAccessLock);
    int n = 0;
    n = m_buffers.size();
    return n;
}

DataBuffer &InternetRadioDecoder::getFirstBuffer()
{
    QMutexLocker lock(&m_bufferAccessLock);
    DataBuffer *tmp;
    tmp = &(m_buffers.first());
    return *tmp;
}

void InternetRadioDecoder::popFirstBuffer()
{
    QMutexLocker lock(&m_bufferAccessLock);
    DataBuffer &buf = m_buffers.first();
    buf.freeData();
    m_buffers.pop_front();
    m_bufferCountSemaphore.release();
}

void InternetRadioDecoder::flushBuffers()
{
    QMutexLocker lock(&m_bufferAccessLock);
    while (m_buffers.size()) {
        DataBuffer &buf = m_buffers.first();
        buf.freeData();
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
    m_inputBufferSize.release(m_maxProbeSize);
#endif
}

QString InternetRadioDecoder::errorString(bool resetError)
{
    m_modErrWarnSemaphore.acquire();
    QString ret = QString(m_errorString.constData(), m_errorString.length()); // deep copy
    if (resetError) {
        m_error       = false;
        m_errorString = "";
    }
    m_modErrWarnSemaphore.release();
    return ret;
}

QString InternetRadioDecoder::warningString(bool resetWarning)
{
    m_modErrWarnSemaphore.acquire();
    QString ret = QString(m_warningString.constData(), m_warningString.length()); // deep copy
    if (resetWarning) {
        m_warning       = false;
        m_warningString = "";
    }
    m_modErrWarnSemaphore.release();
    return ret;
}

QString InternetRadioDecoder::debugString(bool resetWarning)
{
    m_modErrWarnSemaphore.acquire();
    QString ret = QString(m_debugString.constData(), m_debugString.length()); // deep copy
    if (resetWarning) {
        m_debug       = false;
        m_debugString = "";
    }
    m_modErrWarnSemaphore.release();
    return ret;
}

void InternetRadioDecoder::addErrorString(const QString &s)
{
    m_modErrWarnSemaphore.acquire();
    m_error = true;
    if (m_errorString.length()) {
        m_errorString += " \n";
    }
    m_errorString += s;
    m_modErrWarnSemaphore.release();
}

void InternetRadioDecoder::addWarningString(const QString &s)
{
    m_modErrWarnSemaphore.acquire();
    m_warning = true;
    if (m_warningString.length()) {
        m_warningString += " \n";
    }
    m_warningString += s;
    m_modErrWarnSemaphore.release();
}

void InternetRadioDecoder::addDebugString(const QString &s)
{
    m_modErrWarnSemaphore.acquire();
    m_debug = true;
    if (m_debugString.length()) {
        m_debugString += " \n";
    }
    m_debugString += s;
    m_modErrWarnSemaphore.release();
}

#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
static int mms_read_packet(void *opaque, uint8_t *buf, int buf_size)
{
    return mmsx_read(NULL, (mmsx_t*)opaque, (char*)buf, buf_size);
}
#else
static int InternetRadioDecoder_readInputBuffer(void *opaque, uint8_t *buffer, int max_size);
#endif

void InternetRadioDecoder::openAVStream(const QString &stream, bool warningsNotErrors)
{
    if (m_decoderOpened) {
        return;
    }

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

    m_av_pFormatCtx->max_analyze_duration = m_maxAnalyzeTime * AV_TIME_BASE;

    AVFormatParameters av_params;
    memset(&av_params, 0, sizeof(av_params));
    av_params.prealloced_context = 1;

    m_av_pFormatCtx_opened = false;
#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    m_is_mms_stream        = false;
#endif
    // if a format has been specified, set up the proper structures

    LibAVGlobal::ensureInitDone();
    AVInputFormat   *iformat = av_find_input_format(m_RadioStation.decoderClass().toLocal8Bit());

#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    init_put_byte(&m_av_byteio_context,
                    (unsigned char*)m_mms_buffer,
                    sizeof(m_mms_buffer),
                    false,
                    this,
                    &InternetRadioDecoder_readInputBuffer,
                    NULL,
                    NULL
                 );

    // run autodetection if format is not known
    if (!iformat) {

        m_inputBufferSize.acquire(m_maxProbeSize);

        AVProbeData pd = { stream.toLocal8Bit(), (unsigned char*)m_inputBuffer.data(), m_inputBuffer.size() };

        int score = 0;
        iformat = av_probe_input_format2(&pd, 1, &score);
        if (!iformat) {
            if (warningsNotErrors) {
                addWarningString(i18n("Autodetect of stream type failed for %1").arg(stream));
            } else {
                addErrorString(i18n("Autodetect of stream type failed for %1").arg(stream));
            }
            closeAVStream();
            return;
        } else {
            addDebugString(i18n("Autodetected format: %1 (score %2)", iformat->long_name, score));
        }
    }

    if (av_open_input_stream(&m_av_pFormatCtx, &m_av_byteio_context, stream.toUtf8(), iformat, &av_params) != 0) {
        if (warningsNotErrors) {
            addWarningString(i18n("Could not open Stream %1").arg(stream));
        } else {
            addErrorString(i18n("Could not open Stream %1").arg(stream));
        }
        closeAVStream();
        return;
    }
    m_av_pFormatCtx_opened = true;
#else


    // load asf stream
    if (stream.startsWith("mms://") || stream.startsWith("mmsx://")) {
        m_is_mms_stream = true;
        m_mms_stream = mmsx_connect(NULL, NULL, stream.toUtf8(), 1);
        if (!m_mms_stream) {
            if (warningsNotErrors) {
                addWarningString(i18n("cannot open MMS stream %1", stream));
            } else {
                addErrorString(i18n("cannot open MMS stream %1", stream));
            }
            closeAVStream();
            return;
        }
        init_put_byte(&m_av_byteio_context,
                      (unsigned char*)m_mms_buffer,
                      sizeof(m_mms_buffer),
                      false,
                      m_mms_stream,
                      &mms_read_packet,
                      NULL,
                      NULL
                     );

        if (!iformat) {
            iformat = av_find_input_format("asf");
        }

        if (av_open_input_stream(&m_av_pFormatCtx, &m_av_byteio_context, stream.toUtf8(), iformat, &av_params) != 0) {
            if (warningsNotErrors) {
                addWarningString(i18n("Could not open Stream %1").arg(stream));
            } else {
                addErrorString(i18n("Could not open Stream %1").arg(stream));
            }
            closeAVStream();
            return; // Couldn't open file
        }
        m_av_pFormatCtx_opened = true;
    }
    else {
//         IErrorLogClient::staticLogDebug("InternetRadioDecoder::openAVStream: av_open_input_file start");
        if (av_open_input_file(&m_av_pFormatCtx, stream.toUtf8(), iformat, 0, &av_params) != 0) {
            if (warningsNotErrors) {
                addWarningString(i18n("Could not open Stream %1").arg(stream));
            } else {
                addErrorString(i18n("Could not open Stream %1").arg(stream));
            }
            closeAVStream();
            return; // Couldn't open file
        }
        m_av_pFormatCtx_opened = true;
//         IErrorLogClient::staticLogDebug("InternetRadioDecoder::openAVStream: av_open_input_file done");
    }

//     IErrorLogClient::staticLogDebug("InternetRadioDecoder::openAVStream: av_find_stream_info start");
#endif

    // Retrieve stream information
    if (av_find_stream_info(m_av_pFormatCtx) < 0) {
        if (warningsNotErrors) {
            addWarningString(i18n("Could not find stream information in %1").arg(stream));
        } else {
            addErrorString(i18n("Could not find stream information in %1").arg(stream));
        }
        closeAVStream();
        return; // Couldn't find stream information
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


    m_av_audioStream = -1;
    for (unsigned int i = 0; i < m_av_pFormatCtx->nb_streams; i++) {
//         if (m_av_pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO && m_av_audioStream < 0) {
#if LIBAVCODEC_VERSION_MAJOR < 53
        if (m_av_pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO) { // take last stream
#else
        if (m_av_pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) { // take last stream
#endif
            m_av_audioStream = i;
            // break; // do not break here: wait for last stream - currently only way to get some multi-stream sources running with the first stream unusable .
        }
    }

    if (m_av_audioStream == -1) {
        if (warningsNotErrors) {
            addWarningString(i18n("Could not find an audio stream in %1").arg(stream));
        } else {
            addErrorString(i18n("Could not find an audio stream in %1").arg(stream));
        }
        closeAVStream();
        return;
    }


    m_av_aCodecCtx = m_av_pFormatCtx->streams[m_av_audioStream]->codec;


    m_soundFormat = SoundFormat(m_av_aCodecCtx->sample_rate, m_av_aCodecCtx->channels, 16, true);


    m_av_aCodec = avcodec_find_decoder(m_av_aCodecCtx->codec_id);
    if (!m_av_aCodec) {
        if (warningsNotErrors) {
            addWarningString (i18n("Could not find a codec for %1").arg(stream));
        } else {
            addErrorString (i18n("Could not find a codec for %1").arg(stream));
        }
        closeAVStream();
        return;
    }

    if (avcodec_open(m_av_aCodecCtx, m_av_aCodec) < 0) {
        if (warningsNotErrors) {
            addWarningString(i18n("Opening codec for %1 failed").arg(stream));
        } else {
            addErrorString(i18n("Opening codec for %1 failed").arg(stream));
        }
        closeAVStream();
        return;
    }

    m_decoderOpened = true;
}


void InternetRadioDecoder::closeAVStream()
{
    if (m_av_aCodecCtx) {
        avcodec_close(m_av_aCodecCtx);
    }

    if (!m_av_pFormatCtx_opened) {
        av_free(m_av_pFormatCtx);
        m_av_pFormatCtx = NULL;
    }

#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    if (m_is_mms_stream) {
#endif
        if (m_av_pFormatCtx) {
            av_close_input_stream(m_av_pFormatCtx);
        }
#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
        if (m_mms_stream) {
            mmsx_close(m_mms_stream);
        }
    } else if (m_av_pFormatCtx) {
        av_close_input_file(m_av_pFormatCtx);
    }
    m_mms_stream     = NULL;
#endif

    m_av_pFormatCtx  = NULL;
    m_av_audioStream = -1;
    m_av_aCodecCtx   = NULL;
    m_av_aCodec      = NULL;
    m_decoderOpened  = false;
}


#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
static int InternetRadioDecoder_readInputBuffer(void *opaque, uint8_t *buffer, int max_size)
{
    InternetRadioDecoder *x = static_cast<InternetRadioDecoder*>(opaque);
    QByteArray tmp = x->readInputBuffer(max_size);
    memcpy(buffer, tmp.constData(), tmp.size());
    return tmp.size();
}


// blocking function if buffer is empty!
QByteArray InternetRadioDecoder::readInputBuffer(size_t maxSize)
{
    bool       isfull = false;
    QByteArray retval;

    if (m_done) {
        return QByteArray();
    }

    // block until at least 1 byte is readable
    m_inputBufferSize.acquire(1);

    {   QMutexLocker  lock(&m_inputBufferAccessLock);

        QByteArray shared = m_inputBuffer.left(maxSize);
        retval = QByteArray(shared.data(), shared.size()); // force deep copy for threading reasons
        m_inputBuffer.remove(0, retval.size());
        isfull = (size_t)m_inputBuffer.size() >= m_inputBufferMaxSize;
    }

    // keep track of the bytes read
    if (retval.size() > 0) {
        m_inputBufferSize.acquire(retval.size() - 1);
    }

    if (!isfull) {
        emit sigInputBufferNotFull();
    }
    return retval;
}


void InternetRadioDecoder::writeInputBuffer(const QByteArray &data, bool &isFull, const KUrl &inputUrl)
{
    QMutexLocker  lock(&m_inputBufferAccessLock);

    m_inputBuffer.append(data.data(), data.size()); // force deep copy
    isFull     = (size_t)m_inputBuffer.size() >= m_inputBufferMaxSize;
    m_inputUrl = inputUrl;
    m_inputBufferSize.release(data.size());
}
#endif









DecoderThread::DecoderThread(QObject *parent,
                             const InternetRadioStation &rs,
#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
                             const KUrl::List &playlist,
#endif
                             int max_buffers,
                             int max_probe_size_bytes, float max_analyze_secs)
    : QThread(parent),
      m_station(rs),
      m_max_buffers(max_buffers),
      m_max_probe_size_bytes(max_probe_size_bytes),
      m_max_analyze_secs(max_analyze_secs),
      m_decoder(NULL)
#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
      , m_playlist(playlist)
#endif
{
    setTerminationEnabled(true);
}


DecoderThread::~DecoderThread()
{
    if (m_decoder) {
        delete m_decoder;
        m_decoder = NULL;
    }
    IErrorLogClient::staticLogDebug("DecoderThread::~DecoderThread()");
}

void DecoderThread::run()
{
    setTerminationEnabled(true);
    m_decoder = new InternetRadioDecoder(parent(),
                                         m_station,
#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
                                         m_playlist,
#endif
                                         m_max_buffers,
                                         m_max_probe_size_bytes, m_max_analyze_secs);
//     IErrorLogClient::staticLogDebug("DecoderThread::exec()");
    exec();
//     QApplication::postEvent(parent(), new SoundStreamEncodingTerminatedEvent(m_decoder->soundFormat()));
    exit();
}



