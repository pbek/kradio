/***************************************************************************
                          icy_http_handler.cpp
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


#include <QtNetwork/QHttpResponseHeader>

#include <kio/global.h>
#include <klocale.h>
#include <kencodingprober.h>

#include <QtCore/QTextCodec>

#include <math.h>

#include "icy_http_handler.h"
#include "errorlog_interfaces.h"
#include <internetradiostation.h>

// avoid ever-growing metadata buffers
#define METADATA_MAX_SIZE       (1024 * 1024)


IcyHttpHandler::IcyHttpHandler()
  : m_httpHeaderAnalyzed      (false),
    m_ICYMetaInt              (0),
    m_dataRest                (0),
    m_metaRest                (0),
    m_streamJob               (NULL)
{
}


IcyHttpHandler::~IcyHttpHandler()
{
    stopStreamDownload();
}


void IcyHttpHandler::setupStreamJob(const KUrl &url)
{
    // stop old job if present
    stopStreamDownload();

    // start download job
    m_streamUrl = url;
    IErrorLogClient::staticLogDebug(i18n("opening stream %1", m_streamUrl.pathOrUrl()));

    emit sigUrlChanged(m_streamUrl);

    m_streamJob = KIO::get(m_streamUrl, KIO::NoReload, KIO::HideProgressInfo);
    if (m_streamJob) {
        m_streamJob->addMetaData("customHTTPHeader", "Icy-MetaData:1");
        m_streamJob->addMetaData("PropagateHttpHeader", "true");

        QObject::connect(m_streamJob, SIGNAL(data  (KIO::Job *, const QByteArray &)), this, SLOT(slotStreamData(KIO::Job *, const QByteArray &)));
        QObject::connect(m_streamJob, SIGNAL(result(KJob *)),                         this, SLOT(slotStreamDone(KJob *)));
    } else {
        IErrorLogClient::staticLogError(i18n("Failed to start stream download of %1: KIO::get returned NULL pointer").arg(m_streamUrl.pathOrUrl()));
        stopStreamDownload();
        emit sigError(m_streamUrl);
    }
}


void IcyHttpHandler::startStreamJob()
{
    m_httpHeaderAnalyzed = false;
    m_ICYMetaInt         = 0;
    m_dataRest           = 0;
    m_metaRest           = 0;

//     m_inputBuffer->resetBuffer();

    m_streamJob->start();
    emit sigStarted(m_streamUrl);

    if (m_streamJob->error()) {
        IErrorLogClient::staticLogError(i18n("Failed to start stream download of %1: %2").arg(m_streamUrl.pathOrUrl()).arg(m_streamJob->errorString()));
        stopStreamDownload();
        emit sigError(m_streamUrl);
    }
}


void IcyHttpHandler::startStreamDownload(KUrl url)
{
    // just in case, stop current downloads
    stopStreamDownload();

    m_streamUrl = url;

    setupStreamJob(m_streamUrl);
    startStreamJob();
}


void IcyHttpHandler::stopStreamDownload()
{
    if (m_streamJob) {
        QObject::disconnect(m_streamJob, SIGNAL(data  (KIO::Job *, const QByteArray &)), this, SLOT(slotStreamData(KIO::Job *, const QByteArray &)));
        QObject::disconnect(m_streamJob, SIGNAL(result(KJob *)),                         this, SLOT(slotStreamDone(KJob *)));
        m_streamJob->kill(); // stop and delete
        m_streamJob = NULL;
        emit sigFinished(m_streamUrl);
    }
}


void IcyHttpHandler::analyzeHttpHeader(KIO::Job *job)
{
    m_httpHeaderAnalyzed = true;
    m_connectionMetaData = job->metaData();
    foreach(QString k, m_connectionMetaData.keys()) {
        QString v = m_connectionMetaData[k];
        IErrorLogClient::staticLogDebug(QString("%1 = %2").arg(k).arg(v));
        if (k == "HTTP-Headers") {
            QHttpResponseHeader hdr(v);
            QString             icyMetaIntKey = "ICY-metaint";
            if (hdr.hasKey(icyMetaIntKey)) {
                m_ICYMetaInt = hdr.value(icyMetaIntKey).toInt();
                m_dataRest   = m_ICYMetaInt;
                IErrorLogClient::staticLogDebug(QString("found metaint: %1").arg(m_ICYMetaInt));
            }
        }
        else if (k == "content-type") {
            emit sigContentType(m_connectionMetaData[k]);
        }
    }
    emit sigConnectionEstablished(m_streamUrl, m_connectionMetaData);
}


void IcyHttpHandler::handleStreamData(const QByteArray &data)
{
//    logDebug(QString("stream data: %1 bytes").arg(data.size()));
    emit sigStreamData(data);
}


void IcyHttpHandler::handleMetaData(const QByteArray &data, bool complete)
{
    // avoid that the buffer grows too large in case the transmission is broken by whatever
    if (m_metaData.size() + data.size() > METADATA_MAX_SIZE) {
        m_metaData.clear();
    }
    m_metaData.append(QByteArray(data.data(), data.size()));
    if (complete) {

        QString         metaString = QString::fromUtf8(m_metaData.data());
        if (metaString.size()) {

            KEncodingProber prober;
            prober.setProberType(KEncodingProber::WesternEuropean);
            for (int i = 0; i < 1000; ++i) {
                prober.feed(m_metaData);
            }
            printf ("confidence = %f\n", prober.confidence());
            if (prober.confidence() > 0.2) {
                metaString = QTextCodec::codecForName(prober.encoding())->toUnicode(m_metaData);
            }

            IErrorLogClient::staticLogDebug(QString("meta: %1").arg(metaString));

            // parse meta data
            QMap<QString, QString>  metaData;

            while (metaString.size()) {
                int eqIdx = metaString.indexOf('=');
                if (eqIdx < 0) {
                    break;
                }
                QString key = metaString.left(eqIdx).trimmed();
//                 IErrorLogClient::staticLogDebug(QString("Metadata Key: %1").arg(key));

                metaString  = metaString.mid(eqIdx + 1).trimmed();
                bool useQuotes = metaString.startsWith("'");
                metaString.remove(0,useQuotes);

                int      curIdx  = 0;
                QString  escapeRegExp = "\\.|''";
                QString  termRegExp   = useQuotes ? "'\\s*;" : ";";
                QRegExp  findRegExp("(" + escapeRegExp + "|" + termRegExp + ")");
                while (curIdx < metaString.size()) {
                    int findIdx  = findRegExp.indexIn(metaString, curIdx);
                    int matchLen = findRegExp.matchedLength();
                    if (findIdx >= 0) {
                        QString tmp = metaString.mid(findIdx, matchLen);
                        if (QRegExp(escapeRegExp).exactMatch(tmp)) {
                            curIdx += matchLen;
                        } else {
                            QString value = metaString.left(findIdx);
                            metaData.insert(key, value);
                            metaString = metaString.mid(findIdx + matchLen).trimmed();
                            IErrorLogClient::staticLogDebug(QString("Metadata Key: %1 = %2").arg(key).arg(value));
                            break;
                        }
                    } else {
                        metaData.insert(key, metaString);
                        metaString = QString::null;
                        break;
                    }
                }
            }

            emit sigMetaDataUpdate(metaData);
        }

        // free buffer for next meta data transmission
        m_metaData.clear();
    }
}


void IcyHttpHandler::slotStreamData(KIO::Job *job, QByteArray data)
{
    if (m_streamJob == job) {
        if (!m_httpHeaderAnalyzed) {
            analyzeHttpHeader(job);
        }
//         printf ("received packet: %i bytes\n", data.size());
        while (data.size()) {
            // is it stream data?
            if (m_ICYMetaInt <= 0 || m_dataRest > 0) {
                size_t chunk = m_ICYMetaInt > 0 ? qMin(m_dataRest, (size_t)data.size()) : data.size();
//                 printf ("    stream chunk: %zi bytes\n", chunk);
                handleStreamData(data.left(chunk));
                data = data.mid(chunk);
                m_dataRest -= chunk;
            }
            // or is it meta data
            else {
                if (m_metaRest <= 0) {
                    m_metaRest = data[0] * 16;
                    data = data.mid(1);
                }
                size_t     chunk        = qMin(m_metaRest, (size_t)data.size());
//                 printf ("    meta chunk: %zi + 1 bytes\n", chunk);
                QByteArray mdata        = data.left(chunk);
                           m_metaRest   -= chunk;
                bool       metaComplete = m_metaRest <= 0;
                           data         =  data.mid(chunk);
                if (metaComplete) {
                   m_dataRest   = m_ICYMetaInt;
                }
                handleMetaData(mdata, metaComplete);
            }
        }
    }
}


void IcyHttpHandler::slotStreamContinue()
{
    if (m_streamJob) {
        m_streamJob->resume();
        printf ("stream CONTINUED\n");
    }
}


void IcyHttpHandler::slotStreamPause()
{
    m_streamJob->suspend();
    printf ("stream PAUSED\n");
}



void IcyHttpHandler::slotStreamDone(KJob *job)
{
    if (m_streamJob == job) {
        bool local_err = false;
        if (m_streamJob->error()) {
            IErrorLogClient::staticLogError(i18n("Failed to load stream data for %1: %2").arg(m_streamUrl.pathOrUrl()).arg(m_streamJob->errorString()));
            local_err = true;
        }

        KIO::MetaData md = m_streamJob->metaData();
        if (md.contains("HTTP-Headers")) {
            int http_response_code = md["responsecode"].toInt();
            if (http_response_code < 200 || http_response_code >= 300) {
                IErrorLogClient::staticLogError(i18n("HTTP error %1 for stream %2").arg(http_response_code).arg(m_streamUrl.pathOrUrl()));
                local_err = true;
            }
        }
        stopStreamDownload();
        if (local_err) {
            emit sigError(m_streamUrl);
        }
    }
    job->deleteLater();
}



