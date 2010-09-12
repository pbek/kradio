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

extern "C" {
    #define this
    #include <libmms/mmsx.h>
    #undef this
}


class DecoderThread : public QThread
{
public:
    DecoderThread(QObject *parent, const InternetRadioStation &rs, int max_buffers);

    virtual ~DecoderThread();

    void                  run();

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
protected:
    void                  pushBuffer(const DataBuffer &);


protected:

    void                  loadPlaylist();

    void                  addErrorString  (const QString &s);
    void                  addWarningString(const QString &s);
    void                  addDebugString  (const QString &s);


    void                  loadPlaylistM3U(bool errorIfEmpty = true);
    void                  loadPlaylistLSC(bool errorIfEmpty = true);
    void                  loadPlaylistPLS(bool errorIfEmpty = true);
    void                  loadPlaylistASX(bool errorIfEmpty = true);


    bool                  decoderOpened() const { return m_decoderOpened; }
    void                  openAVStream(const QString &stream, bool warningsNotErros = false);
    void                  closeAVStream();



    bool                  m_decoderOpened;
    AVFormatContext      *m_av_pFormatCtx;
    int                   m_av_audioStream;
    AVCodecContext       *m_av_aCodecCtx;
    AVCodec              *m_av_aCodec;
    ByteIOContext         m_av_byteio_context;

    mmsx_t               *m_mms_stream;
    char                  m_mms_buffer[32768];


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

    KUrl                  m_inputURL;
    QList<KUrl>           m_playListURLs;
    KUrl                  m_playURL;

    QList<DataBuffer>     m_buffers;
    QSemaphore            m_bufferAccessLock;
    QSemaphore            m_bufferCountSemaphore;
};


#endif
