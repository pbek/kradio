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


MMSXHandlerThread::MMSXHandlerThread(KUrl url, MMSXHandler *parent)
  : m_url(url),
    m_parent(parent)
{
}


void MMSXHandlerThread::run()
{
    MMSXWrapper mmsx(m_url);
    m_mmsx = &mmsx;
    QObject::connect(&mmsx, SIGNAL(sigError(KUrl)),                               m_parent, SLOT(proxyError(KUrl)),                               Qt::QueuedConnection);
    QObject::connect(&mmsx, SIGNAL(sigFinished(KUrl)),                            m_parent, SLOT(proxyFinished(KUrl)),                            Qt::QueuedConnection);
    QObject::connect(&mmsx, SIGNAL(sigStarted(KUrl)),                             m_parent, SLOT(proxyStarted(KUrl)),                             Qt::QueuedConnection);
    QObject::connect(&mmsx, SIGNAL(sigConnectionEstablished(KUrl,KIO::MetaData)), m_parent, SLOT(proxyConnectionEstablished(KUrl,KIO::MetaData)), Qt::QueuedConnection);
    QObject::connect(&mmsx, SIGNAL(sigUrlChanged(KUrl)),                          m_parent, SLOT(proxyUrlChanged(KUrl)),                          Qt::QueuedConnection);
    QObject::connect(&mmsx, SIGNAL(sigContentType(QString)),                      m_parent, SLOT(proxyContentType(QString)),                      Qt::QueuedConnection);
    QObject::connect(&mmsx, SIGNAL(sigStreamData(QByteArray)),                    m_parent, SLOT(proxyStreamData(QByteArray)),                    Qt::QueuedConnection);
    QObject::connect(&mmsx, SIGNAL(sigMetaDataUpdate(KIO::MetaData)),             m_parent, SLOT(proxyMetaDataUpdate(KIO::MetaData)),             Qt::QueuedConnection);

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


MMSXWrapper::MMSXWrapper(KUrl url)
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
    m_mms_stream = mmsx_connect(NULL, NULL, m_url.pathOrUrl().toUtf8(), 1);
    if (!m_mms_stream) {
        emit sigError(m_url);
        return;
    }
    emit sigContentType("audio/asf");
    emit sigConnectionEstablished(m_url, KIO::MetaData());
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
    emit sigFinished(m_url);
}
