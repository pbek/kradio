/***************************************************************************
                          decoder_thread.h  -  description
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

#ifndef KRADIO_DECODER_THREAD_H
#define KRADIO_DECODER_THREAD_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtCore/QSemaphore>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

#include <kurl.h>

#include "soundformat.h"
#include "sound_metadata.h"
#include "thread-logging.h"
#include "internetradiostation.h"

#include "databuffer.h"
#include "stream_input_buffer.h"
#include "icy_http_handler.h"

extern "C" {
#ifdef HAVE_FFMPEG
    #include <libavformat/avformat.h>
    #include <libavutil/dict.h>
#endif
#ifdef HAVE_FFMPEG_OLD
    #include <ffmpeg/avformat.h>
#endif
}

#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
extern "C" {
    #define this
    #include <libmms/mmsx.h>
    #undef this
}
#endif


// if this macro is defined, the encoded input stream and decoded PCM audio stream
// as well as some log information about chunk sizes and positions in the streams
// are dumped to statically named files in /tmp
//
// #define DEBUG_DUMP_DECODER_STREAMS


class InternetRadioDecoder : public QObject,
                             public ThreadLogging
{
Q_OBJECT
public:
    InternetRadioDecoder(QObject                    *event_parent,
                         const InternetRadioStation &station,
#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
                         const KUrl::List           &playlist,
#else
                         const KUrl                 &currentStreamUrl,
                         StreamInputBuffer          *input_buffer,
                         QString                     contentType,
#endif
                         int   max_buffers,
                         int   max_singleBufferSize,
                         int   max_probe_size_bytes,
                         float max_analyze_secs);

    virtual ~InternetRadioDecoder();

    void                  setDone();
    bool                  isDone() const { return m_done; }

    bool                  initDone() const    { return m_decoderOpened; }

    // sound format is only valid if initDone returns true
    void                  updateSoundFormat();
    const SoundFormat    &soundFormat() const { return m_soundFormat; }

    // output buffers
    int                   availableBuffers();
    DataBuffer           &getFirstBuffer();
    void                  popFirstBuffer();
    void                  flushBuffers();

protected:
    // output buffer
    void                  pushBuffer(const char *data, size_t dataSize, const SoundMetaData &md, const SoundFormat &sf);

signals:
    void                  sigSelfTrigger();

protected slots:

    void                  run();

protected:

    bool                  decoderOpened() const { return m_decoderOpened; }
    void                  openAVStream(const QString &stream, bool warningsNotErros = false);
    void                  closeAVStream();
    void                  freeAVIOContext();

    bool                  readFrame(AVPacket &pkt);
    bool                  decodePacket(AVPacket &pkt, int &decoded_input_size);

#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    void                  selectStreamFromPlaylist();
#endif


    bool                  m_decoderOpened;
    AVFormatContext      *m_av_pFormatCtx;
    bool                  m_av_pFormatCtx_opened;
    int                   m_av_audioStream;
    AVCodecContext       *m_av_aCodecCtx;
    AVCodec              *m_av_aCodec;
#if  LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(52, 110, 0) // checked: avformat_open_input in ffmpeg >= 0.7
    AVIOContext          *m_av_byteio_contextPtr;
#else
    ByteIOContext         m_av_byteio_context;
#endif
#if  LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 42, 0) // checked: avcodec_decode_audio4 in ffmpeg >= 0.9
    AVFrame              *m_decoded_frame;
#endif

#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    bool                  m_is_mms_stream;
    mmsx_t               *m_mms_stream;
#endif


    QObject              *m_parent;
    InternetRadioStation  m_RadioStation;

    bool                  m_error;
    bool                  m_done;

    SoundFormat           m_soundFormat;
    quint64               m_decodedSize;
    quint64               m_encodedSize;
    time_t                m_startTime;

#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    KUrl::List            m_playListURLs;
    KUrl                  m_playURL;
#endif

    // output buffers
    QList<DataBuffer>     m_buffers;
    QMutex                m_bufferAccessLock;
    QSemaphore            m_bufferCountSemaphore;
    size_t                m_maxBufferCount;
    size_t                m_maxSingleBufferSize;

#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    KUrl                  m_inputUrl;
    StreamInputBuffer    *m_streamInputBuffer;
    QString               m_contentType;
#endif

    int                   m_maxProbeSize;    // in bytes,   see openAVStream
    float                 m_maxAnalyzeTime;  // in seconds, see openAVStream

#ifdef DEBUG_DUMP_DECODER_STREAMS
    FILE  *m_debugCodedStream;
    FILE  *m_debugDecodedStream;
    FILE  *m_debugMetaStream;
#endif
};











class DecoderThread : public QThread
{
Q_OBJECT
public:
    DecoderThread(QObject                    *parent,
                  const InternetRadioStation &station,
#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
                  const KUrl::List           &playlist,
#else
                  const KUrl                 &currentStreamUrl,
                  StreamReader               *streamReader,
//                   StreamInputBuffer          *input_buffer,
#endif
                  int   max_buffers,
                  int   max_singleBufferSize,
                  int   max_probe_size_bytes,
                  float max_analyze_secs
                 );
    virtual ~DecoderThread();

    void                  run();

    InternetRadioDecoder *decoder() const { return m_decoder; }

protected:

    InternetRadioStation  m_station;
    int                   m_max_buffers;
    int                   m_max_singleBufferSize;
    int                   m_max_probe_size_bytes;
    float                 m_max_analyze_secs;

    InternetRadioDecoder *m_decoder;
#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    KUrl::List            m_playlist;
#else
    KUrl                  m_currentStreamUrl;
    StreamInputBuffer    *m_streamInputBuffer;
    QString               m_contentType;
#endif
};



#endif
