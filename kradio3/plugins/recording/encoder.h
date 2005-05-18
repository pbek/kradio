/***************************************************************************
                          encoder.h  -  description
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

#ifndef KRADIO_RECORDING_ENCODER_H
#define KRADIO_RECORDING_ENCODER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <sndfile.h>
#ifdef HAVE_LAME_LAME_H
    #include <lame/lame.h>
#endif
#if defined(HAVE_VORBIS_VORBISENC_H) && defined(HAVE_OGG_OGG_H)
    #include <vorbis/vorbisenc.h>
#endif

#include <qobject.h>
#include <qstring.h>
#include <qmutex.h>
#include <qsemaphore.h>
#include <qthread.h>
#include <qtimer.h>


class BufferSoundMetaData : public SoundMetaData
{
public:
    BufferSoundMetaData()
        : SoundMetaData(0, 0, 0, KURL()), m_BufferPosition(0) {}
    BufferSoundMetaData(const SoundMetaData &md, unsigned bufferpos)
        : SoundMetaData(md), m_BufferPosition(bufferpos) {}
    BufferSoundMetaData(Q_INT64 pos, time_t rel, time_t abs, const KURL &url, unsigned bufferpos)
        : SoundMetaData(pos, rel, abs, url), m_BufferPosition(bufferpos) {}

    unsigned bufferPosition() const { return m_BufferPosition; }

protected:
    unsigned m_BufferPosition;
};


class RecordingEncoding : public QThread
{
public:
    RecordingEncoding(QObject *parent, SoundStreamID id, const RecordingConfig &cfg, const RadioStation *rs, const QString &filename);
    RecordingEncoding::~RecordingEncoding();

    void run();

    char              *lockInputBuffer(unsigned int &bufferSize);    // bytes we whish to write, returns number of bytes available
    void               unlockInputBuffer(unsigned int bufferSize, const SoundMetaData &md);   // bytes we actually wrote

    bool               error() const { return m_error; }
    const QString     &errorString() const { return m_errorString; }

    void               setDone();
    bool               IsDone() { return m_done; }

    bool               openOutput(const QString &outputFile);
    void               closeOutput();

    Q_UINT64           encodedSize() const { return m_encodedSize; }

    const RecordingConfig &config() const { return m_config; }

protected:
    void               encode_pcm(const char *_buffer, unsigned buffer_size, char *&export_buffer, unsigned &export_buffer_size);
    void               encode_mp3(const char *_buffer, unsigned buffer_size, char *&export_buffer, unsigned &export_buffer_size);
    void               encode_ogg(const char *_buffer, unsigned buffer_size, char *&export_buffer, unsigned &export_buffer_size);

    bool               openOutput_pcm(const QString &outputFile);
    bool               openOutput_mp3(const QString &outputFile);
    bool               openOutput_ogg(const QString &outputFile);

    void               closeOutput_pcm();
    void               closeOutput_mp3();
    void               closeOutput_ogg();

    QObject           *m_parent;
    RecordingConfig    m_config;
    RadioStation      *m_RadioStation;
    SoundStreamID      m_SoundStreamID;

    QMutex             m_bufferInputLock;
    QSemaphore         m_inputAvailableLock;
    bool               m_error;
    QString            m_errorString;
    bool               m_done;

    char             **m_buffersInput;
    unsigned int      *m_buffersInputFill,
                       m_currentInputBuffer;
    QPtrList<BufferSoundMetaData>
                     **m_buffersMetaData;
    Q_UINT64           m_encodedSize;

    time_t             m_InputStartTime;
    Q_UINT64           m_InputStartPosition;

    KURL               m_outputURL;
    SNDFILE           *m_output;

#ifdef HAVE_LAME_LAME_H
    unsigned char     *m_MP3Buffer;
    unsigned int       m_MP3BufferSize;
    FILE              *m_MP3Output;
    char              *m_ID3Tags;
    lame_global_flags *m_LAMEFlags;
    short int         *m_MP3LBuffer,
                      *m_MP3RBuffer;
#endif

#if defined(HAVE_VORBIS_VORBISENC_H) && defined(HAVE_OGG_OGG_H)
    FILE              *m_OggOutput;
    char              *m_OggExportBuffer;
    unsigned           m_OggExportBufferSize;
    ogg_stream_state   m_OggStream;
    vorbis_dsp_state   m_VorbisDSP;
    vorbis_block       m_VorbisBlock;
    vorbis_info        m_VorbisInfo;
#endif
};


#endif
