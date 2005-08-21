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


#include <qobject.h>
#include <qstring.h>
#include <qthread.h>

#include "../../src/radio-stations/radiostation.h"
#include "../../src/libkradio/multibuffer.h"
#include "../../src/libkradio/sound_metadata.h"
#include "../../src/libkradio/soundstreamid.h"
#include "recording-config.h"

class BufferSoundMetaData : public SoundMetaData
{
public:
    BufferSoundMetaData()
        : SoundMetaData(0, 0, 0, KURL()), m_BufferPosition(0) {}
    BufferSoundMetaData(const SoundMetaData &md, size_t bufferpos)
        : SoundMetaData(md), m_BufferPosition(bufferpos) {}
    BufferSoundMetaData(Q_INT64 pos, time_t rel, time_t abs, const KURL &url, size_t bufferpos)
        : SoundMetaData(pos, rel, abs, url), m_BufferPosition(bufferpos) {}

    size_t bufferPosition() const { return m_BufferPosition; }

protected:
    size_t m_BufferPosition;
};


class RecordingEncoding : public QThread
{
public:
    RecordingEncoding(QObject *parent, SoundStreamID id, const RecordingConfig &cfg, const RadioStation *rs, const QString &filename);
    virtual ~RecordingEncoding();

    void run();

    char              *lockInputBuffer(size_t &bufferSize);    // bytes we whish to write, returns number of bytes available
    void               unlockInputBuffer(size_t bufferSize, const SoundMetaData &md);   // bytes we actually wrote

    bool               error() const { return m_error; }
    const QString     &errorString() const { return m_errorString; }

    void               setDone();
    bool               IsDone() { return m_done; }

    virtual bool       openOutput(const QString &outputFile) = 0;
    virtual void       closeOutput() = 0;

    Q_UINT64           encodedSize() const { return m_encodedSize; }

    const RecordingConfig &config() const { return m_config; }

protected:
    virtual void       encode(const char *_buffer, size_t buffer_size, char *&export_buffer, size_t &export_buffer_size) = 0;

    QObject           *m_parent;
    RecordingConfig    m_config;
    RadioStation      *m_RadioStation;
    SoundStreamID      m_SoundStreamID;

    bool               m_error;
    QString            m_errorString;
    bool               m_done;

    MultiBuffer        m_InputBuffers;
    QPtrList<BufferSoundMetaData>
                     **m_buffersMetaData;
    Q_UINT64           m_encodedSize;

    time_t             m_InputStartTime;
    Q_UINT64           m_InputStartPosition;

    KURL               m_outputURL;
};


#endif
