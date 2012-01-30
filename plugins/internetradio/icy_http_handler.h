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

#include "stream_reader.h"

class IcyHttpHandler : public StreamReader
{
Q_OBJECT
public:
    IcyHttpHandler();
    ~IcyHttpHandler();

    // FIXME: parent class with standardized interface
    // FIXME: parent class which cares about playlist handling

    void                        startStreamDownload(KUrl url);
    void                        stopStreamDownload();

    KIO::MetaData               getConnectionMetaData() const { return m_connectionMetaData; }

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
    KIO::MetaData               m_connectionMetaData;
};


#endif
