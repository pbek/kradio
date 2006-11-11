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

#include "../../src/include/radiostation.h"
#include "../../src/include/errorlog-interfaces.h"
#include "../../src/include/aboutwidget.h"

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

RecordingEncoding::RecordingEncoding(QObject *parent,            SoundStreamID ssid,
                                     const RecordingConfig &cfg, const RadioStation *rs,
                                     const QString &filename)
    :
      m_parent(parent),
      m_config(cfg),
      m_RadioStation(rs ? rs->copy() : NULL),
      m_SoundStreamID(ssid),
      m_error(false),
      m_errorString(QString::null),
      m_done(false),
      m_InputBuffers(m_config.m_EncodeBufferCount < 3 ? 3 : m_config.m_EncodeBufferCount,
                     m_config.m_EncodeBufferSize < 4096 ? 4096 : m_config.m_EncodeBufferSize),
      m_buffersMetaData(NULL),
      m_encodedSize(0),
      m_InputStartTime(0),
      m_InputStartPosition(0),
      m_outputURL(filename)
{

    if (m_config.m_EncodeBufferCount < 3)
        m_config.m_EncodeBufferCount = 3;
    if (m_config.m_EncodeBufferSize < 4096)
        m_config.m_EncodeBufferSize = 4096;

    m_buffersMetaData  = new QPtrList<BufferSoundMetaData> *[m_config.m_EncodeBufferCount];
    for (size_t i = 0; i < m_config.m_EncodeBufferCount; ++i) {
        m_buffersMetaData [i] = new QPtrList<BufferSoundMetaData>;
        m_buffersMetaData [i]->setAutoDelete(true);
    }
}


RecordingEncoding::~RecordingEncoding()
{
    for (size_t i = 0; i < m_config.m_EncodeBufferCount; ++i) {
        delete m_buffersMetaData[i];
    }
    delete m_buffersMetaData;
    delete m_RadioStation;
}


char *RecordingEncoding::lockInputBuffer(size_t &bufferSize)
{
    if (m_done || m_error)
        return NULL;
    char * retval = m_InputBuffers.lockWriteBuffer(bufferSize);

    m_error |= m_InputBuffers.hasError();
    m_errorString += m_InputBuffers.getErrorString();
    m_InputBuffers.resetError();

    return retval;
}


void  RecordingEncoding::unlockInputBuffer(size_t bufferSize, const SoundMetaData &md)
{
    if (m_done)
        return;
    size_t bufidx  = m_InputBuffers.getCurrentWriteBufferIdx();
    size_t buffill = m_InputBuffers.getWriteBufferFill();
    m_InputBuffers.unlockWriteBuffer(bufferSize);

    if (!m_InputBuffers.hasError()) {
        if (!m_InputStartTime) {
            m_InputStartTime     = md.absoluteTimestamp();
            m_InputStartPosition = md.position();
        }
        BufferSoundMetaData *bmd = new BufferSoundMetaData(
                                md.position() - m_InputStartPosition,
                                md.absoluteTimestamp() - m_InputStartTime,
                                md.absoluteTimestamp(),
                                md.url(),
                                buffill);
        m_buffersMetaData[bufidx]->append(bmd);
    } else {
        m_error = true;
        m_errorString += m_InputBuffers.getErrorString();
        m_InputBuffers.resetError();
    }
}


void RecordingEncoding::setDone()
{
    m_done = true;
    m_InputBuffers.unlockAllWriteBuffers();
}



void RecordingEncoding::run()
{
    BufferSoundMetaData  last_md;

    while (!m_error) {
        char   *buffer      = NULL;
        size_t  buffer_fill = 0;
        if (!m_done) {
            buffer = m_InputBuffers.wait4ReadBuffer(buffer_fill);
        }

        if (!buffer_fill) {
            if (m_done)
                break;
            else
                continue;
        }

        char        *export_buffer = NULL;
        size_t       export_buffer_size = 0;

        Q_UINT64  old_pos  = m_encodedSize;

        encode(buffer, buffer_fill, export_buffer, export_buffer_size);

        SoundStreamEncodingStepEvent *step_event = NULL;

        if (!m_error) {
            last_md = *m_buffersMetaData[m_InputBuffers.getCurrentReadBufferIdx()]->first();
            SoundMetaData  md(old_pos, last_md.relativeTimestamp(), last_md.absoluteTimestamp(), m_outputURL);
            step_event = new SoundStreamEncodingStepEvent(m_SoundStreamID, export_buffer, export_buffer_size, md);
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

