/***************************************************************************
                          mmsx_handler_thread.h  -  description
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

#ifndef KRADIO_MMSX_HANDLER_THREAD_H
#define KRADIO_MMSX_HANDLER_THREAD_H

#include <QtCore/QUrl>
#include <QtCore/QThread>

#include "stream_reader.h"


extern "C" {
    #define this
    #include <libmms/mmsx.h>
    #undef this
}

class  MMSXHandler;
class  MMSXWrapper;

class MMSXHandlerThread : public QThread
{
public:
    MMSXHandlerThread(QUrl url, MMSXHandler *parent);

    void  run() override;
    void  stop();

protected:

    QUrl         m_url;
    MMSXHandler *m_parent;
    MMSXWrapper *m_mmsx;
};


class MMSXWrapper : public QObject
{
Q_OBJECT
public:
    MMSXWrapper(QUrl url);

    void run();
    void stop();

signals:
    void  sigError   (QUrl url);
    void  sigFinished(QUrl url);
    void  sigStarted (QUrl url);
    void  sigConnectionEstablished(QUrl url, KIO::MetaData metaData);

    void  sigUrlChanged(QUrl url);
    void  sigContentType(QString contentType);

    void  sigStreamData    (QByteArray data);
    void  sigMetaDataUpdate(KIO::MetaData metadata);

protected:
    QUrl           m_url;
    mmsx_t        *m_mms_stream;
    bool           m_stopRequested;
    bool           m_error;
    KIO::MetaData  m_metaData;
};



#endif
