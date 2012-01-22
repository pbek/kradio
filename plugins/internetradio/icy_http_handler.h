/***************************************************************************
                          icy_http_handler.h  -  description
                             -------------------
    begin                : Thu Jan 22 2012
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

class IcyHttpHandler : public QObject
{
Q_OBJECT
public:
    IcyHttpHandler(const KUrl &url);
    ~IcyHttpHandler();

    // FIXME: store ICY meta data
    // FIXME: if content-type/encoding is given: use it for decoder autodetection

    void                start();

protected slots:
    void                jobDataAvailable(KIO::Job *job, QByteArray data);

protected:
    void                setupJob();

    KUrl                m_url;
    KIO::TransferJob   *m_transferJob;
    bool                m_httpHeaderAnalyzed;
    size_t              m_ICYMetaInt;
    size_t              m_dataRest;
    size_t              m_metaRest;
    QByteArray          m_metaData;
};


#endif
