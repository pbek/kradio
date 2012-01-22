/***************************************************************************
                          icy_http_handler.cpp
                             -------------------
    begin                : Thu Jan 21 2012
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

#include "icy_http_handler.h"

#include <kio/global.h>
#include <QtNetwork/QHttpResponseHeader>
#include "errorlog_interfaces.h"

// avoid ever-growing metadata buffers
#define METADATA_MAX_SIZE       (1024 * 1024)


IcyHttpHandler::IcyHttpHandler(const KUrl &url)
  : m_url               (url),
    m_transferJob       (NULL),
    m_httpHeaderAnalyzed(false),
    m_ICYMetaInt        (0),
    m_dataRest          (0),
    m_metaRest          (0)
{
    setupJob();
}


IcyHttpHandler::~IcyHttpHandler()
{
    if (m_transferJob) {
        delete m_transferJob;
    }
}


void IcyHttpHandler::setupJob()
{
    m_transferJob = KIO::get(m_url, KIO::NoReload, KIO::HideProgressInfo);
    m_transferJob->addMetaData("customHTTPHeader", "Icy-MetaData:1");
    m_transferJob->addMetaData("PropagateHttpHeader", "true");

    connect(m_transferJob, SIGNAL(data(KIO::Job*,QByteArray)), SLOT(jobDataAvailable(KIO::Job*,QByteArray)));
}


void IcyHttpHandler::start()
{
    m_transferJob->start();
}


void IcyHttpHandler::jobDataAvailable(KIO::Job *job, QByteArray data)
{
    if (!m_httpHeaderAnalyzed) {
        m_httpHeaderAnalyzed = true;
        KIO::MetaData md = job->metaData();
        foreach(QString k, md.keys()) {
            QString v = md[k];
            IErrorLogClient::staticLogDebug(QString("%1 = %2").arg(k).arg(v));
            if (k == "HTTP-Headers") {
                QHttpResponseHeader hdr(v);
                QString icyMetaIntKey = "ICY-metaint";
                if (hdr.hasKey(icyMetaIntKey)) {
                    m_ICYMetaInt = hdr.value(icyMetaIntKey).toInt();
                    IErrorLogClient::staticLogDebug(QString("found metaint: %1").arg(m_ICYMetaInt));
                    m_dataRest = m_ICYMetaInt;
                }
            }
        }
    }
    while (data.size()) {
        // is it stream data?
        if (m_ICYMetaInt <= 0) {
            size_t chunk = data.size();
            data = data.mid(chunk);
            m_dataRest -= chunk;
        }
        else if (m_dataRest > 0) {
            size_t chunk = qMin(m_dataRest, (size_t)data.size());
            data = data.mid(chunk);
            m_dataRest -= chunk;
        }
        // or is it meta data
        else {
            if (m_metaRest <= 0) {
                m_metaRest = data[0] * 16;
                data = data.mid(1);
                m_metaData.clear();
            }
            size_t chunk = qMin(m_metaRest, (size_t)data.size());
            // avoid that the buffer grows too large in case the transmission is broken by whatever
            if (m_metaData.size() + chunk > METADATA_MAX_SIZE) {
                m_metaData.clear();
            }
            m_metaData.append(data.left(chunk));
            data = data.mid(chunk);
            m_metaRest -= chunk;
            if (m_metaRest <= 0) {
                m_dataRest = m_ICYMetaInt;
                QString metaString = QString::fromUtf8(m_metaData.data());
                if (metaString.size()) {
                    IErrorLogClient::staticLogDebug(QString("meta: %1").arg(metaString));
                }
            }
        }
    }
}


