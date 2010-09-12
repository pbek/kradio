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

#include <QtGui/QApplication>
#include <QtCore/QXmlStreamReader>

#include <kio/jobclasses.h>
#include <kio/netaccess.h>
#include <kio/job.h>
#include <kconfig.h>
#include <kconfiggroup.h>

/*#include "soundstream_decoding_step_event.h"*/
#include "soundstream_decoding_terminate_event.h"
#include "decoder_thread.h"
#include "errorlog_interfaces.h"


DecoderThread::DecoderThread(QObject *parent, const InternetRadioStation &rs, int max_buffers)
:   m_decoderOpened (false),
    m_av_pFormatCtx (NULL),
    m_av_audioStream(-1),
    m_av_aCodecCtx  (NULL),
    m_av_aCodec     (NULL),

    m_mms_stream    (NULL),

    m_parent        (parent),
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
    m_inputURL      (m_RadioStation.url()),
    m_bufferAccessLock(1),
    m_bufferCountSemaphore(max_buffers)
{
}

DecoderThread::~DecoderThread()
{
    flushBuffers();
    closeAVStream();
}


void DecoderThread::loadPlaylist()
{
    QString plscls = m_RadioStation.playlistClass();
    if (plscls == "auto") {
        if (m_inputURL.path().endsWith(".lsc")) {
            loadPlaylistLSC();
        }
        else if (m_inputURL.path().endsWith(".m3u")) {
            loadPlaylistM3U();
        }
        else if (m_inputURL.path().endsWith(".pls")) {
            loadPlaylistPLS();
        }
        else if (m_inputURL.path().endsWith(".asx")) {
            loadPlaylistASX();
        } else {
            m_playListURLs.append(m_inputURL);
        }
    } else if (plscls == "lsc") {
            loadPlaylistLSC();
    } else if (plscls == "m3u") {
            loadPlaylistM3U();
    } else if (plscls == "asx") {
            loadPlaylistASX();
    } else if (plscls == "pls") {
            loadPlaylistPLS();
    } else {
        m_playListURLs.append(m_inputURL);
    }
}


void DecoderThread::loadPlaylistLSC(bool errorIfEmpty)
{
    loadPlaylistM3U(false);
    if (!m_playListURLs.size()) {
        loadPlaylistASX(errorIfEmpty);
    }
}

void DecoderThread::loadPlaylistM3U(bool errorIfEmpty)
{
    IErrorLogClient::staticLogDebug(i18n("downloading playlist %1", m_inputURL.pathOrUrl()));
    QByteArray        playlistData;
    KIO::TransferJob *job = KIO::get(m_inputURL);

    m_playListURLs.clear();

    if (KIO::NetAccess::synchronousRun(job, NULL, &playlistData)) {
        QStringList lines = QString(playlistData).split("\n");
        QString line;
        foreach (line, lines) {
            QString t = line.trimmed();
            if (t.startsWith("http://") || t.startsWith("https://") || t.startsWith("mms://")) {
                m_playListURLs.append(t);
            }
        }
        if (errorIfEmpty && !m_playListURLs.size()) {
            addErrorString(i18n("%1 does not contain any usable radio stream", m_inputURL.pathOrUrl()));
        }
    } else {
        addErrorString(i18n("failed to download %1", m_inputURL.pathOrUrl()));
    }
}


void DecoderThread::loadPlaylistPLS(bool errorIfEmpty)
{
    IErrorLogClient::staticLogDebug(i18n("downloading playlist %1", m_inputURL.pathOrUrl()));

    m_playListURLs.clear();

    QString tmpFile;
    if( KIO::NetAccess::download(m_inputURL, tmpFile, NULL) ) {
        KConfig      cfg(tmpFile);

        // mapping group names to lower case in order to be case insensitive
        QStringList            groups = cfg.groupList();
        QMap<QString, QString> group_lc_map;
        QString                grp;
        foreach(grp, groups) {
            group_lc_map.insert(grp.toLower(), grp);
        }

        KConfigGroup cfggrp = cfg.group(group_lc_map["playlist"]);

        // mapping entry keys to lower case in order to be case insensitive
        QStringList keys = cfggrp.keyList();
        QMap<QString, QString> key_lc_map;
        QString key;
        foreach(key, keys) {
            key_lc_map.insert(key.toLower(), key);
        }

        unsigned int entries = cfggrp.readEntry(key_lc_map["numberofentries"], 0);
        if (entries) {
            for (unsigned int i = 0; i < entries; ++i) {
                QString url = cfggrp.readEntry(key_lc_map[QString("file%1").arg(i)], QString());
                if (url.length()) {
                    m_playListURLs.append(url);
                }
            }
        }

        if (errorIfEmpty && !m_playListURLs.size()) {
            addErrorString(i18n("%1 does not contain any usable radio stream", m_inputURL.pathOrUrl()));
        }
        KIO::NetAccess::removeTempFile(tmpFile);
    } else {
        addErrorString(i18n("failed to download %1", m_inputURL.pathOrUrl()));
    }
}


void DecoderThread::loadPlaylistASX(bool errorIfEmpty)
{
    QByteArray        xmlData;
    KIO::TransferJob *job = KIO::get(m_inputURL);

    m_playListURLs.clear();

    if (KIO::NetAccess::synchronousRun(job, NULL, &xmlData)) {
        QXmlStreamReader reader(xmlData);

        bool inEntry = false;

        while (!reader.atEnd() && (reader.error() == QXmlStreamReader::NoError)) {
            reader.readNext();
            if (reader.isStartElement()) {
                QStringRef name = reader.name();
                if (name.toString().toLower() == "entry") {
                    inEntry = true;
                }
                else if (name.toString().toLower() == "ref" && inEntry) {
                    QXmlStreamAttributes attrs = reader.attributes();
                    QXmlStreamAttribute  attr;
                    foreach(attr, attrs) {
                        if(attr.name().toString().toLower() == "href") {
                            m_playListURLs.append(attr.value().toString());
                        }
                    }
                }
            }
            else if (reader.isEndElement()) {
                QStringRef name = reader.name();
                if (name == "entry") {
                    inEntry = false;
                }
            }
        }

        if (reader.error() != QXmlStreamReader::NoError) {
            addErrorString(i18n("error while reading asx file", reader.error()));
        }

        if (errorIfEmpty && !m_playListURLs.size()) {
            addErrorString(i18n("%1 does not contain any usable radio stream", m_inputURL.pathOrUrl()));
        }

    } else {
        addErrorString(i18n("failed to download %1", m_inputURL.pathOrUrl()));
    }
}


void DecoderThread::run()
{
    loadPlaylist();

    
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
        addErrorString(i18n("Could not open any input stream of %1.", m_inputURL.pathOrUrl()));
    }

    AVPacket pkt;

    time_t  start_time      = time(NULL);

    while (!m_error && !m_done && m_decoderOpened && (av_read_frame(m_av_pFormatCtx, &pkt) >= 0)) {

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
                    addWarningString(i18n("%1: error decoding packet. Discarding packet\n").arg(m_playURL.pathOrUrl()));
                    break;
                }
                else if (generated_output_bytes > 0) {
                    time_t cur_time = time(NULL);
                    SoundMetaData  md(m_decodedSize, cur_time - start_time, cur_time, i18n("internal stream, not stored (%1)", m_playURL.pathOrUrl()));

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
//     printf ("posting event\n");
    QApplication::postEvent(m_parent, new SoundStreamEncodingTerminatedEvent(m_soundFormat));

//     printf ("closing stream\n");
    closeAVStream();
    exit();
}


void DecoderThread::pushBuffer(const DataBuffer &buf)
{
    m_bufferCountSemaphore.acquire();
    m_bufferAccessLock.acquire();
    m_buffers.push_back(buf);
    m_bufferAccessLock.release();
}


int DecoderThread::availableBuffers()
{
    int n = 0;
    m_bufferAccessLock.acquire();
    n = m_buffers.size();
    m_bufferAccessLock.release();
    return n;
}

DataBuffer &DecoderThread::getFirstBuffer()
{
    DataBuffer *tmp;
    m_bufferAccessLock.acquire();
    tmp = &(m_buffers.first());
    m_bufferAccessLock.release();
    return *tmp;
}

void DecoderThread::popFirstBuffer()
{
    m_bufferAccessLock.acquire();
    DataBuffer &buf = m_buffers.first();
    buf.freeData();
    m_buffers.pop_front();
    m_bufferAccessLock.release();
    m_bufferCountSemaphore.release();
}

void DecoderThread::flushBuffers()
{
    m_bufferAccessLock.acquire();
    while (m_buffers.size()) {
        DataBuffer &buf = m_buffers.first();
        buf.freeData();
        m_buffers.pop_front();
        m_bufferCountSemaphore.release();
    }
    m_bufferAccessLock.release();
}

void DecoderThread::setDone()
{
    m_done = true;
}

QString DecoderThread::errorString(bool resetError)
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

QString DecoderThread::warningString(bool resetWarning)
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

QString DecoderThread::debugString(bool resetWarning)
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

void DecoderThread::addErrorString(const QString &s)
{
    m_modErrWarnSemaphore.acquire();
    m_error = true;
    if (m_errorString.length()) {
        m_errorString += " \n";
    }
    m_errorString += s;
    m_modErrWarnSemaphore.release();
}

void DecoderThread::addWarningString(const QString &s)
{
    m_modErrWarnSemaphore.acquire();
    m_warning = true;
    if (m_warningString.length()) {
        m_warningString += " \n";
    }
    m_warningString += s;
    m_modErrWarnSemaphore.release();
}

void DecoderThread::addDebugString(const QString &s)
{
    m_modErrWarnSemaphore.acquire();
    m_debug = true;
    if (m_debugString.length()) {
        m_debugString += " \n";
    }
    m_debugString += s;
    m_modErrWarnSemaphore.release();
}

static int mms_read_packet(void *opaque, uint8_t *buf, int buf_size)
{
    return mmsx_read(NULL, (mmsx_t*)opaque, (char*)buf, buf_size);
}

void DecoderThread::openAVStream(const QString &stream, bool warningsNotErrors)
{
    if (m_decoderOpened) {
        return;
    }

    av_register_all();
    AVInputFormat   *iformat = av_find_input_format(m_RadioStation.decoderClass().toLocal8Bit());

    // load asf stream
    if (stream.startsWith("mms://")) {
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

        if (av_open_input_stream(&m_av_pFormatCtx, &m_av_byteio_context, stream.toUtf8(), iformat, NULL) != 0) {
            if (warningsNotErrors) {
                addWarningString(i18n("Could not open Stream %1").arg(stream));
            } else {
                addErrorString(i18n("Could not open Stream %1").arg(stream));
            }
            closeAVStream();
            return; // Couldn't open file
        }
    }
    else {

        if (av_open_input_file(&m_av_pFormatCtx, stream.toUtf8(), iformat, 0, NULL) != 0) {
            if (warningsNotErrors) {
                addWarningString(i18n("Could not open Stream %1").arg(stream));
            } else {
                addErrorString(i18n("Could not open Stream %1").arg(stream));
            }
            closeAVStream();
            return; // Couldn't open file
        }
    }

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


    m_av_audioStream = -1;
    for (unsigned int i = 0; i < m_av_pFormatCtx->nb_streams; i++) {
//         if (m_av_pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO && m_av_audioStream < 0) {
        if (m_av_pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO) { // take last stream
            m_av_audioStream = i;
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


void DecoderThread::closeAVStream()
{
    if (m_av_aCodecCtx) {
        avcodec_close(m_av_aCodecCtx);
    }

    if (m_mms_stream) {
        if (m_av_pFormatCtx) {
            av_close_input_stream(m_av_pFormatCtx);
        }
        mmsx_close(m_mms_stream);
    } else if (m_av_pFormatCtx) {
        av_close_input_file(m_av_pFormatCtx);
    }

    m_mms_stream     = NULL;
    m_av_pFormatCtx  = NULL;
    m_av_audioStream = -1;
    m_av_aCodecCtx   = NULL;
    m_av_aCodec      = NULL;
    m_decoderOpened  = false;
}


