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
#include "internetradiostation.h"

#include "databuffer.h"

extern "C" {
#ifdef HAVE_FFMPEG
    #include <libavformat/avformat.h>
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





class InternetRadioDecoder : public QObject
{
Q_OBJECT
public:
    InternetRadioDecoder(QObject *event_parent, 
                         const InternetRadioStation &station, 
#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
                         const KUrl::List &playlist, 
#endif
                         int max_buffers, 
                         int max_probe_size_bytes, float max_analyze_secs);

    virtual ~InternetRadioDecoder();

    bool                  error()         const { return m_error; }
    QString               errorString(bool resetError = true);
    bool                  warning()       const { return m_warning; }
    QString               warningString(bool resetWarning = true);
    bool                  debug()         const { return m_debug; }
    QString               debugString(bool resetDebug = true);

    void                  setDone();
    bool                  isDone() const { return m_done; }

    bool                  initDone() const    { return m_decoderOpened; }

    // sound format is only valid if initDone returns true
    const SoundFormat    &soundFormat() const { return m_soundFormat; }


    int                   availableBuffers();
    DataBuffer           &getFirstBuffer();
    void                  popFirstBuffer();
    void                  flushBuffers();

#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    void                  writeInputBuffer(const QByteArray &data, bool &isFull, const KUrl &inputUrl);
    QByteArray            readInputBuffer(size_t maxSize);
#endif

protected:
    void                  pushBuffer(const DataBuffer &);

signals:
    void                  sigSelfTrigger();
#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    void                  sigInputBufferNotFull();
#endif

protected slots:

    void                  run();

protected:
    void                  addErrorString  (const QString &s);
    void                  addWarningString(const QString &s);
    void                  addDebugString  (const QString &s);


    bool                  decoderOpened() const { return m_decoderOpened; }
    void                  openAVStream(const QString &stream, bool warningsNotErros = false);
    void                  closeAVStream();
    void                  freeAVIOContext();


    bool                  m_decoderOpened;
    AVFormatContext      *m_av_pFormatCtx;
    bool                  m_av_pFormatCtx_opened;
    int                   m_av_audioStream;
    AVCodecContext       *m_av_aCodecCtx;
    AVCodec              *m_av_aCodec;
#if  LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53, 24, 0)
    AVIOContext          *m_av_byteio_contextPtr;
#else
    ByteIOContext         m_av_byteio_context;
#endif

#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    bool                  m_is_mms_stream;
    mmsx_t               *m_mms_stream;
#endif
//     char                  m_mms_buffer[32768];


    QObject              *m_parent;
    InternetRadioStation  m_RadioStation;

    bool                  m_error;
    QString               m_errorString;
    bool                  m_warning;
    QString               m_warningString;
    bool                  m_debug;
    QString               m_debugString;
    mutable
    QSemaphore            m_modErrWarnSemaphore;

    bool                  m_done;

    SoundFormat           m_soundFormat;
    quint64               m_decodedSize;

#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    KUrl::List            m_playListURLs;
    KUrl                  m_playURL;
#endif

    QList<DataBuffer>     m_buffers;
    QMutex                m_bufferAccessLock;
    QSemaphore            m_bufferCountSemaphore;

#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    size_t                m_inputBufferMaxSize;
    QByteArray            m_inputBuffer;
    QMutex                m_inputBufferAccessLock;
    QSemaphore            m_inputBufferSize;
    KUrl                  m_inputUrl;
#endif

    int                   m_maxProbeSize;    // in bytes,   see openAVStream
    float                 m_maxAnalyzeTime;  // in seconds, see openAVStream
};











class DecoderThread : public QThread
{
Q_OBJECT
public:
    DecoderThread(QObject *parent,
                  const InternetRadioStation &station,
#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
                  const KUrl::List &playlist,
#endif
                  int max_buffers,
                  int max_probe_size_bytes,
                  float max_analyze_secs
                 );
    virtual ~DecoderThread();

    void                  run();

    InternetRadioDecoder *decoder() const { return m_decoder; }

protected:

    InternetRadioStation  m_station;
    int                   m_max_buffers;
    int                   m_max_probe_size_bytes;
    float                 m_max_analyze_secs;

    InternetRadioDecoder *m_decoder;
#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    KUrl::List            m_playlist;
#endif
};



#endif
