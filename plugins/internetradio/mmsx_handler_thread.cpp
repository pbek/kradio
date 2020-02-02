/***************************************************************************
                          mmsx_handler_thread.cpp  -  description
                             -------------------
    begin                : Sun Jan 22 2012
    copyright            : (C) 2012 by Martin Witte
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

#include "mmsx_handler_thread.h"
#include "mmsx_handler.h"
#include <unistd.h>

extern "C" {
    #define this
    #include <libmms/mmsx.h>
    #undef this
}


MMSXHandlerThread::MMSXHandlerThread(QUrl url, MMSXHandler *parent)
  : m_url(url),
    m_parent(parent)
{
}


void MMSXHandlerThread::run()
{
    MMSXWrapper mmsx(m_url);
    m_mmsx = &mmsx;
    QObject::connect(&mmsx, &MMSXWrapper::sigError,                 m_parent, &MMSXHandler::proxyError,                 Qt::QueuedConnection);
    QObject::connect(&mmsx, &MMSXWrapper::sigFinished,              m_parent, &MMSXHandler::proxyFinished,              Qt::QueuedConnection);
    QObject::connect(&mmsx, &MMSXWrapper::sigStarted,               m_parent, &MMSXHandler::proxyStarted,               Qt::QueuedConnection);
    QObject::connect(&mmsx, &MMSXWrapper::sigConnectionEstablished, m_parent, &MMSXHandler::proxyConnectionEstablished, Qt::QueuedConnection);
    QObject::connect(&mmsx, &MMSXWrapper::sigUrlChanged,            m_parent, &MMSXHandler::proxyUrlChanged,            Qt::QueuedConnection);
    QObject::connect(&mmsx, &MMSXWrapper::sigContentType,           m_parent, &MMSXHandler::proxyContentType,           Qt::QueuedConnection);
    QObject::connect(&mmsx, &MMSXWrapper::sigStreamData,            m_parent, &MMSXHandler::proxyStreamData,            Qt::QueuedConnection);
    QObject::connect(&mmsx, &MMSXWrapper::sigMetaDataUpdate,        m_parent, &MMSXHandler::proxyMetaDataUpdate,        Qt::QueuedConnection);

    mmsx.run();

    m_mmsx = NULL;
    exit();
}


void MMSXHandlerThread::stop()
{
    if (m_mmsx) {
        m_mmsx->stop();
        m_mmsx->disconnect(m_parent);
    }
}


MMSXWrapper::MMSXWrapper(QUrl url)
  : m_url(url),
    m_mms_stream(NULL),
    m_stopRequested(false),
    m_error(false)
{
}


void MMSXWrapper::stop()
{
    m_stopRequested = true;
    emit sigFinished(m_url);
}


void MMSXWrapper::run()
{
    const int buffer_size = 1024 * 4; // FIXME: make configurable
    char buffer[buffer_size];

    emit sigUrlChanged(m_url);
    m_mms_stream = mmsx_connect(NULL, NULL, m_url.toString().toUtf8().constData(), 1);
    if (!m_mms_stream) {
        m_error = true;
        emit sigError(m_url);
        return;
    }
    m_metaData.clear();
    m_metaData.insert("content-type", "audio/asf");
    emit sigContentType          (m_metaData["content-type"]);
    emit sigConnectionEstablished(m_url, m_metaData);
    emit sigMetaDataUpdate       (m_metaData);
    while(!m_stopRequested && !m_error) {
        int n = mmsx_read(NULL, m_mms_stream, buffer, buffer_size);
        if (n < 0) {
            m_error = true;
            emit sigError(m_url);
            break;
        }
        if (n > 0) {
            emit sigStreamData(QByteArray(buffer, n));
        } else {
            usleep(50000);
        }
    }
    if (m_mms_stream) {
        mmsx_close(m_mms_stream);
    }
    if (!m_error) {
        emit sigFinished(m_url);
    }
}
