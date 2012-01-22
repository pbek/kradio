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

#include <math.h>

#include "icy_http_handler.h"
#include "errorlog_interfaces.h"
#include <internetradiostation.h>

// avoid ever-growing metadata buffers
#define METADATA_MAX_SIZE       (1024 * 1024)


IcyHttpHandler::IcyHttpHandler(StreamInputBuffer *buffer)
  : m_httpHeaderAnalyzed      (false),
    m_ICYMetaInt              (0),
    m_dataRest                (0),
    m_metaRest                (0),
    m_currentStreamIdx        (-1),
    m_currentStreamRetriesMax (2),
    m_currentStreamRetriesLeft(-1),
    m_randStreamIdxOffset     (-1),
    m_error                   (false),
    m_active                  (false),
    m_streamJob               (NULL),
    m_inputBuffer             (buffer)
{
    QObject::connect(m_inputBuffer, SIGNAL(sigInputBufferNotFull()), this, SLOT(slotStreamContinue()), Qt::QueuedConnection);
}


IcyHttpHandler::~IcyHttpHandler()
{
    stopStreamDownload();
}


void IcyHttpHandler::setupStreamJob(const KUrl &url)
{
    // stop old job if present
    stopCurrentStreamDownload();

    // start download job
    m_currentStreamUrl = url;
    logDebug(i18n("opening stream %1", m_currentStreamUrl.pathOrUrl()));

    m_streamJob = KIO::get(m_currentStreamUrl, KIO::NoReload, KIO::HideProgressInfo);
    if (m_streamJob) {
        m_streamJob->addMetaData("customHTTPHeader", "Icy-MetaData:1");
        m_streamJob->addMetaData("PropagateHttpHeader", "true");

        QObject::connect(m_streamJob, SIGNAL(data  (KIO::Job *, const QByteArray &)), this, SLOT(slotStreamData(KIO::Job *, const QByteArray &)));
        QObject::connect(m_streamJob, SIGNAL(result(KJob *)),                         this, SLOT(slotStreamDone(KJob *)));
    } else {
        logError(i18n("Failed to start stream download of %1: KIO::get returned NULL pointer").arg(m_currentStreamUrl.pathOrUrl()));
        emit sigErrorStream(m_currentStreamUrl);
        stopCurrentStreamDownload();
    }
}


void IcyHttpHandler::startCurrentStreamJob()
{
    m_httpHeaderAnalyzed = false;
    m_ICYMetaInt         = 0;
    m_dataRest           = 0;
    m_metaRest           = 0;

    m_inputBuffer->resetBuffer();

    m_streamJob->start();
    emit sigStartedStream(m_currentStreamUrl);

    if (m_streamJob->error()) {
        logError(i18n("Failed to start stream download of %1: %2").arg(m_currentStreamUrl.pathOrUrl()).arg(m_streamJob->errorString()));
        emit sigErrorStream(m_currentStreamUrl);
        stopCurrentStreamDownload();
    }
}


void IcyHttpHandler::startStreamDownload(const KUrl::List &urls, const InternetRadioStation &rs, int max_retries)
{
    // just in case, stop current downloads
    stopStreamDownload();

    m_currentPlaylist          = urls;
    m_currentStation           = rs;
    m_currentStreamRetriesMax  = max_retries;
    
    m_randStreamIdxOffset      = rint((m_currentPlaylist.size() - 1) * (float)rand() / (float)RAND_MAX);
    m_currentStreamIdx         = 0;
    m_currentStreamRetriesLeft = m_currentStreamRetriesMax;

    m_active                   = true;
    m_error                    = false;

    emit sigStartedPlaylist(m_currentStation.url());

    tryNextStream();
}


void IcyHttpHandler::tryNextStream()
{
    do {
        if (--m_currentStreamRetriesLeft < 0) {
            ++m_currentStreamIdx;
            m_currentStreamRetriesLeft = m_currentStreamRetriesMax;
        }
        if (m_active && m_currentStreamIdx < m_currentPlaylist.size()) {
            setupStreamJob(m_currentPlaylist[(m_currentStreamIdx + m_randStreamIdxOffset) % m_currentPlaylist.size()]);
            startCurrentStreamJob();
        } else {
            logError(i18n("Failed to start any stream of %1").arg(m_currentStation.longName()));
            stopStreamDownload();

            m_active = false;
            m_error  = true;
            emit sigErrorPlaylist(m_currentStation.url());
        }
    } while (m_active && !m_streamJob);
}


void IcyHttpHandler::stopCurrentStreamDownload()
{
    if (m_streamJob) {
        QObject::disconnect(m_streamJob, SIGNAL(data  (KIO::Job *, const QByteArray &)), this, SLOT(slotStreamData(KIO::Job *, const QByteArray &)));
        QObject::disconnect(m_streamJob, SIGNAL(result(KJob *)),                         this, SLOT(slotStreamDone(KJob *)));
        m_streamJob->kill(); // stop and delete
        m_streamJob = NULL;

        m_inputBuffer->resetBuffer();
        emit sigFinishedStream(m_currentStreamUrl);
    }
}


void IcyHttpHandler::stopStreamDownload()
{
    stopCurrentStreamDownload();
    if (m_active) {
        m_active = false;
        emit sigFinishedPlaylist(m_currentStation.url());
    }
}


void IcyHttpHandler::analyzeHttpHeader(KIO::Job *job)
{
    m_httpHeaderAnalyzed = true;
    KIO::MetaData md = job->metaData();
    foreach(QString k, md.keys()) {
        QString v = md[k];
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
    }
}


void IcyHttpHandler::handleStreamData(const QByteArray &data)
{
//         logDebug(QString("stream data: %1 bytes").arg(data.size()));
    if (m_inputBuffer) {
        bool isfull = false;
        m_inputBuffer->writeInputBuffer(data, isfull, m_currentStreamUrl);
        if (isfull) {
            m_streamJob->suspend();
        }
    }
}


void IcyHttpHandler::handleMetaData(const QByteArray &data, bool complete)
{
    // avoid that the buffer grows too large in case the transmission is broken by whatever
    if (m_metaData.size() + data.size() > METADATA_MAX_SIZE) {
        m_metaData.clear();
    }
    m_metaData.append(data);
    if (complete) {
        QString metaString = QString::fromUtf8(m_metaData.data());
        if (metaString.size()) {
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
        while (data.size()) {
            // is it stream data?
            if (m_ICYMetaInt <= 0 || m_dataRest > 0) {
                size_t chunk = m_ICYMetaInt > 0 ? qMin(m_dataRest, (size_t)data.size()) : data.size();
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
    }
}


void IcyHttpHandler::slotStreamDone(KJob *job)
{
    if (m_streamJob == job) {
        bool local_err = false;
        if (m_streamJob->error()) {
            logError(i18n("Failed to load stream data for %1: %2").arg(m_currentStreamUrl.pathOrUrl()).arg(m_streamJob->errorString()));
            local_err = true;
            emit sigErrorStream(m_currentStreamUrl);
        }
        if (local_err) {
            stopCurrentStreamDownload();
            m_currentStreamRetriesLeft = 0;
            tryNextStream();
        } else {
            stopStreamDownload();
        }
        m_inputBuffer->resetBuffer();
    }
    job->deleteLater();
}



