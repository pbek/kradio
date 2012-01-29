/***************************************************************************
                          icy_http_handler.h  -  description
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

#ifndef KRADIO_ICY_HTTP_HANDLER_H
#define KRADIO_ICY_HTTP_HANDLER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QtCore/QSharedPointer>
#include <kurl.h>
#include <kio/job.h>
#include <kio/jobclasses.h>

#include "errorlog_interfaces.h"
#include "internetradiostation.h"
#include "stream_input_buffer.h"

class IcyHttpHandler : public QObject
{
Q_OBJECT
public:
    IcyHttpHandler();
    ~IcyHttpHandler();

    // FIXME: store ICY meta data
    // FIXME: if content-type/encoding is given: use it for decoder autodetection

    // FIXME: parent class with standardized interface
    // FIXME: parent class which cares about playlist handling

    void                        startStreamDownload(KUrl url);
    void                        stopStreamDownload();

signals:
    void                        sigError(KUrl url);
    void                        sigFinished(KUrl url);
    void                        sigStarted(KUrl url);

    void                        sigUrlChanged(KUrl url);

    void                        sigStreamData(QByteArray data);
    void                        sigMetaDataUpdate(QMap<QString, QString> metadata);

protected slots:
    void                        slotStreamData(KIO::Job *job, QByteArray data);
    void                        slotStreamDone(KJob *job);
    void                        slotStreamContinue();
    void                        slotStreamPause();

protected:
    void                        setupStreamJob(const KUrl &url);
    void                        startStreamJob();

    void                        analyzeHttpHeader(KIO::Job *job);
    void                        handleStreamData(const QByteArray &data);
    void                        handleMetaData(const QByteArray &data, bool complete);

protected:
    bool                        m_httpHeaderAnalyzed;
    size_t                      m_ICYMetaInt;
    size_t                      m_dataRest;
    size_t                      m_metaRest;
    QByteArray                  m_metaData;

    KUrl                        m_streamUrl;
    KIO::TransferJob           *m_streamJob;
};


#endif
