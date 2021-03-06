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

#include <QObject>
#include <QString>
#include <QThread>
#include <QSemaphore>
#include <QMutex>
#include <QMutexLocker>

#include <QtCore/QUrl>

#include "soundformat.h"
#include "sound_metadata.h"
#include "thread-logging.h"
#include "internetradiostation.h"

#include "databuffer.h"
#include "stream_input_buffer.h"
#include "icy_http_handler.h"

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavutil/dict.h>
#ifdef HAVE_LIBAVRESAMPLE
    #include <libavresample/avresample.h>
#elif defined(HAVE_LIBSWRESAMPLE)
    #include <libswresample/swresample.h>
#endif
}

#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
extern "C" {
    #define this
    #include <libmms/mmsx.h>
    #undef this
}
#endif


/*! if this macro is defined, the encoded input stream and decoded PCM audio stream
    as well as some log information about chunk sizes and positions in the streams
    are dumped to statically named files in /tmp
*/
// #define DEBUG_DUMP_DECODER_STREAMS

/*! Enable DEBUGGING: Log Buffer States
 */
// #define DEBUG_LOG_BUFFER_STATE

/*! Enable DEBUGGING: Do not send PCM data to the audio
 *  interface (fast decoding)
 */
// #define DEBUG_DISCARD_DECODED_PCM_DATA


/*! Stop execution at some hard coded known packed number for easier decoding problem
 *  analysis
 */
#ifdef DEBUG_DUMP_DECODER_STREAMS
//     #define DEBUG_EXIT_ON_EXAMPLE_ERROR_PACKET
#endif



#define DEFAULT_MMS_BUFFER_SIZE         65536



class InternetRadioDecoder : public QObject,
                             public ThreadLogging
{
Q_OBJECT
public:
    InternetRadioDecoder(QObject                    *event_parent,
                         const InternetRadioStation &station,
#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
                         const QUrl::List           &playlist,
#else
                         const QUrl                 &currentStreamUrl,
                         StreamInputBuffer          *input_buffer,
                         QString                     contentType,
#endif
                         int   max_output_buffers,
                         int   max_output_buffer_chunk_size,
                         int   max_probe_size_bytes,
                         float max_analyze_secs,
                         int   max_retries
                        );

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
    void                  initIOCallbacks(void *opaque, int(*read_packet_func)(void *, uint8_t *, int ));
    void                  open_av_input(AVInputFormat *iformat, const QString &stream, bool warningsNotErrors, bool use_io_context);
    AVInputFormat *       getInputFormat(const QString &fallbackFormat, bool warningsNotErrors);
    bool                  retrieveStreamInformation(const QString &stream, bool warningsNotErrors);
    bool                  openCodec(const QString &stream, bool warningsNotErrors);

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
    AVIOContext          *m_av_byteio_contextPtr;
    AVFrame              *m_decoded_frame;
#ifdef HAVE_LIBAVRESAMPLE
    AVAudioResampleContext  *m_resample_context;
#elif defined HAVE_LIBSWRESAMPLE
    SwrContext              *m_resample_context;
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
    QUrl::List            m_playListURLs;
    QUrl                  m_playURL;
#endif

    // output buffers
    QList<DataBuffer>     m_buffers;
    QMutex                m_bufferAccessLock;
    QSemaphore            m_bufferCountSemaphore;
    size_t                m_maxBufferCount;
    size_t                m_maxSingleBufferSize;

#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    QUrl                  m_inputUrl;
    StreamInputBuffer    *m_streamInputBuffer;
    QString               m_contentType;
#endif

    int                   m_maxProbeSize;    // in bytes,   see openAVStream
    float                 m_maxAnalyzeTime;  // in seconds, see openAVStream
    int                   m_maxRetries;

    QString               m_i18nInternalStream;

#ifdef DEBUG_DUMP_DECODER_STREAMS
    FILE  *m_debugCodedStream;
    FILE  *m_debugDecodedStream;
    FILE  *m_debugMetaStream;

    size_t m_debugPacketCount;
#endif
};











class DecoderThread : public QThread
{
Q_OBJECT
public:
    DecoderThread(QObject                    *parent,
                  const InternetRadioStation &station,
#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
                  const QUrl::List           &playlist,
#else
                  const QUrl                 &currentStreamUrl,
                  StreamReader               *streamReader,
//                   StreamInputBuffer          *input_buffer,
#endif
                  int   input_buffer_size,
                  int   max_output_buffers,
                  int   max_output_buffer_chunk_size,
                  int   max_probe_size_bytes,
                  float max_analyze_secs,
                  int   max_retries
                 );
    virtual ~DecoderThread();

    void                  run() override;

    InternetRadioDecoder *decoder() const { return m_decoder; }

protected:

    InternetRadioStation  m_station;
    int                   m_max_buffers;
    int                   m_max_singleBufferSize;
    int                   m_max_probe_size_bytes;
    float                 m_max_analyze_secs;
    int                   m_max_retries;

    InternetRadioDecoder *m_decoder;
#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    QUrl::List            m_playlist;
#else
    QUrl                  m_currentStreamUrl;
    StreamInputBuffer    *m_streamInputBuffer;
    QString               m_contentType;
#endif
};



#endif
