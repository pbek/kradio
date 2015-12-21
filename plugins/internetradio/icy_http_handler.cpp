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


#include <kio/global.h>
#include <klocalizedstring.h>
#include <kio/slaveconfig.h>

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
    m_streamJob               (NULL),
    m_metaDataEncoding        ("auto"),
    m_metaDataEncodingCodec   (NULL)
#ifdef DEBUG_DUMP_ICY_STREAMS
    , m_debugFullStream (NULL)
    , m_debugMetaStream (NULL)
    , m_debugDataStream (NULL)
    , m_debugDecodingLog(NULL)
    , m_debugFullPos    (0)
    , m_debugDataPos    (0)
    , m_debugMetaPos    (0)
#endif
{
}


IcyHttpHandler::~IcyHttpHandler()
{
    stopStreamDownload();
}


void IcyHttpHandler::setupStreamJob(const KUrl &url, const QString &metaDataEncoding)
{
    // stop old job if present
    stopStreamDownload();

    m_metaDataEncoding      = metaDataEncoding;
    m_metaDataEncodingCodec = QTextCodec::codecForName(m_metaDataEncoding.toLatin1());

    // start download job
    m_streamUrl = url;
    IErrorLogClient::staticLogDebug(i18n("Internet Radio Plugin (ICY http handler): opening stream %1", m_streamUrl.pathOrUrl()));

    emit sigUrlChanged(m_streamUrl);

    m_streamJob = KIO::get(m_streamUrl, KIO::NoReload, KIO::HideProgressInfo);
    if (m_streamJob) {
        m_streamJob->addMetaData("customHTTPHeader",     "Icy-MetaData:1");
        m_streamJob->addMetaData("accept",               "*/*");
        m_streamJob->addMetaData("Encodings",            "*");
        m_streamJob->addMetaData("Charsets",             "*");
        m_streamJob->addMetaData("Languages",            "*");
        m_streamJob->addMetaData("UserAgent",            QString("KRadio Internet Radio Plugin, Version %1").arg(KRADIO_VERSION));

        m_streamJob->addMetaData("PropagateHttpHeader", "true");

        QObject::connect(m_streamJob, SIGNAL(data  (KIO::Job *, const QByteArray &)), this, SLOT(slotStreamData(KIO::Job *, const QByteArray &)));
        QObject::connect(m_streamJob, SIGNAL(result(KJob *)),                         this, SLOT(slotStreamDone(KJob *)));
    } else {
        IErrorLogClient::staticLogError(i18n("Internet Radio Plugin (ICY http handler): Failed to start stream download of %1: KIO::get returned NULL pointer", m_streamUrl.pathOrUrl()));
        stopStreamDownload(false);
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
        IErrorLogClient::staticLogError(i18n("Internet Radio Plugin (ICY http handler): Failed to start stream download of %1: %2", m_streamUrl.pathOrUrl(), m_streamJob->errorString()));
        stopStreamDownload(false);
        emit sigError(m_streamUrl);
    }
}


void IcyHttpHandler::startStreamDownload(KUrl url, const QString &metaDataEncoding)
{
    // just in case, stop current downloads
    stopStreamDownload();

    m_streamUrl = url;

    setupStreamJob(m_streamUrl, metaDataEncoding);
    startStreamJob();

#ifdef DEBUG_DUMP_ICY_STREAMS
    m_debugFullStream  = fopen("/tmp/kradio-debug-icy-full-stream",  "w");
    m_debugMetaStream  = fopen("/tmp/kradio-debug-icy-meta-stream",  "w");
    m_debugDataStream  = fopen("/tmp/kradio-debug-icy-data-stream",  "w");
    m_debugDecodingLog = fopen("/tmp/kradio-debug-icy-decoding-log", "w");
    m_debugFullPos     = 0;
    m_debugDataPos     = 0;
    m_debugMetaPos     = 0;
#endif
}


void IcyHttpHandler::stopStreamDownload(bool emitSigFinished)
{
    if (m_streamJob) {
        QObject::disconnect(m_streamJob, SIGNAL(data  (KIO::Job *, const QByteArray &)), this, SLOT(slotStreamData(KIO::Job *, const QByteArray &)));
        QObject::disconnect(m_streamJob, SIGNAL(result(KJob *)),                         this, SLOT(slotStreamDone(KJob *)));
        m_streamJob->kill(); // stop and delete
        m_streamJob = NULL;
        if (emitSigFinished) {
            emit sigFinished(m_streamUrl);
        }
    }
#ifdef DEBUG_DUMP_ICY_STREAMS
    if (m_debugFullStream)  fclose(m_debugFullStream);  m_debugFullStream  = NULL;
    if (m_debugMetaStream)  fclose(m_debugMetaStream);  m_debugMetaStream  = NULL;
    if (m_debugDataStream)  fclose(m_debugDataStream);  m_debugDataStream  = NULL;
    if (m_debugDecodingLog) fclose(m_debugDecodingLog); m_debugDecodingLog = NULL;
#endif
}


QMap<QString, QString> IcyHttpHandler::splitExtractHttpHeaderKeys(QString httpHeader)
{
    // urg... QHttpResponseHeader is deprecated and does not parse ICY headers correctly.
    // let's do it ourselves
    QStringList  lines = httpHeader.split(QRegExp("\\r?\\n"));

    // fix HTTP header line continuations
    QStringList  consolidatedLines;
    foreach(QString line, lines) {
        if (line.startsWith(" ")) {
            consolidatedLines.last() += line;
        } else {
            consolidatedLines.append(line);
        }
    }

    // extract keys form header lines
    QMap<QString, QString> headerMap;
    foreach(QString line, consolidatedLines) {
        QRegExp  colonRegExp("\\s*:\\s*");
        int colonIdx = colonRegExp.indexIn(line);
        if (colonRegExp.matchedLength() > 0 && colonIdx > 0 && colonIdx < line.length()) {
            QString  key = line.left(colonIdx);
            QString  val = line.mid (colonIdx + colonRegExp.matchedLength());
            headerMap.insert(key, val);
        }
    }
    return headerMap;
}


void IcyHttpHandler::analyzeHttpHeader(QString httpHeader, KIO::MetaData &metaData)
{
    QMap<QString, QString> headerMap      = splitExtractHttpHeaderKeys(httpHeader);
    QString                icyMetaIntKey  = "icy-metaint";
    QString                contentTypeKey = "content-type";

    foreach(QString key, headerMap.keys()) {
        if (key.toLower() == icyMetaIntKey) {
            m_ICYMetaInt = headerMap.value(key).toInt();
            m_dataRest   = m_ICYMetaInt;
            IErrorLogClient::staticLogDebug(QString("Internet Radio Plugin (ICY http handler):     found metaint: %1").arg(m_ICYMetaInt));
        }
        if (key.toLower() == contentTypeKey) {
            emit sigContentType(headerMap.value(key));
        }
    }
    foreach(QString key, headerMap.keys()) {
        metaData.insert(key, headerMap.value(key));
    }
}


// returns packet minus header
QByteArray IcyHttpHandler::analyzeICYHeader(QByteArray packet)
{
    m_httpHeaderAnalyzed = true;
    QString              headerStr     = packet;
    int                  headerLen     = headerStr.indexOf("\r\n\r\n");
    QByteArray           remainingData = packet.mid(headerLen + 4);
    m_connectionMetaData.clear();
    analyzeHttpHeader(headerStr.left(headerLen + 4), m_connectionMetaData);
    emit sigConnectionEstablished(m_streamUrl, m_connectionMetaData);
    return remainingData;
}


void IcyHttpHandler::analyzeHttpHeader(KIO::Job *job)
{
    m_httpHeaderAnalyzed = true;
    m_connectionMetaData = job->metaData();
    foreach(QString k, m_connectionMetaData.keys()) {
        QString v = m_connectionMetaData[k];
        IErrorLogClient::staticLogDebug(QString("Internet Radio Plugin (ICY http handler):      %1 = %2").arg(k).arg(v));
        if (k == "HTTP-Headers") {
            analyzeHttpHeader(v, m_connectionMetaData);
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
#ifdef DEBUG_DUMP_ICY_STREAMS
    fwrite(data.constData(), data.size(), 1, m_debugDataStream); fflush(m_debugDataStream);
    fprintf(m_debugDecodingLog, "   data received: size = %zi @ pos %zi\n", (size_t)data.size(), m_debugDataPos); fflush(m_debugDecodingLog);
    m_debugDataPos += data.size();
#endif
    emit sigStreamData(data);
}


void IcyHttpHandler::handleMetaData(const QByteArray &data, bool complete)
{
    // avoid that the buffer grows too large in case the transmission is broken by whatever
    if (m_metaData.size() + data.size() > METADATA_MAX_SIZE) {
        m_metaData.clear();
    }
    m_metaData.append(QByteArray(data.data(), data.size()));

#ifdef DEBUG_DUMP_ICY_STREAMS
    fwrite(data.constData(), data.size(), 1, m_debugMetaStream);  fflush(m_debugMetaStream);
    fprintf(m_debugDecodingLog, "   meta data received: size = %zi @ pos %zi\n", (size_t)data.size(), m_debugMetaPos); fflush(m_debugDecodingLog);
    m_debugMetaPos += data.size();
#endif

    if (complete) {
        if (m_metaData.size()) {
            // truncate tailing zeros, seems that the codecs also translate zero chars
            int     zpos = m_metaData.indexOf('\000');
            if (zpos >= 0) {
                m_metaData = m_metaData.left(zpos);
            }
            QString         tmpString;
            if (m_metaDataEncoding == "auto" || !m_metaDataEncodingCodec) {
                m_metaDataEncodingProber.feed(m_metaData);
    //             printf ("confidence = %f\n", prober.confidence());
                if (m_metaDataEncodingProber.confidence() > 0.8) {
                    tmpString = QTextCodec::codecForName(m_metaDataEncodingProber.encoding())->toUnicode(m_metaData);
                } else {
                    tmpString = QString::fromUtf8(m_metaData.data());
                }
            } else {
                tmpString = m_metaDataEncodingCodec->toUnicode(m_metaData);
            }
            // enforce deep copy
            QString metaString(tmpString.constData(), tmpString.size());

            QString charcode = "";
            for (int i = 0; i < metaString.length(); ++i) {
                charcode += QString::number(metaString[i].toAscii(), 16) + " ";
            }
            IErrorLogClient::staticLogDebug(QString("Internet Radio Plugin (ICY http handler):     meta: %1 (len %2), %3").arg(metaString).arg(metaString.length()).arg(charcode));

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
                QString  escapeRegExp = "\\\\.|''";
                QString  termRegExp   = useQuotes ? "'\\s*;" : ";";
                QRegExp  findRegExp("(" + escapeRegExp + "|" + termRegExp + ")");
                // step by step replace escapes and generate variable value
                QString  value;
                while (curIdx < metaString.size()) {
                    int findIdx  = findRegExp.indexIn(metaString, curIdx);
                    int matchLen = findRegExp.matchedLength();
                    if (findIdx >= 0) {
                        QString tmp = metaString.mid(findIdx, matchLen);
                        if (QRegExp(escapeRegExp).exactMatch(tmp)) {
                            curIdx += matchLen;
                        } else {
                            value      = metaString.left(findIdx);
                            metaString = metaString.mid(findIdx + matchLen).trimmed();
                            break;
                        }
                    } else {
                        value      = metaString;
                        metaString = QString::null;
                        break;
                    }
                }
                metaData.insert(key, value);
#ifdef DEBUG
                IErrorLogClient::staticLogDebug(QString("Internet Radio Plugin (ICY http handler):     Metadata Key: %1 = %2").arg(key).arg(value));
#endif
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
            QRegExp  ICYHeaderRegExp("^ICY\\s+\\d+\\s+\\w+[\r\n]");
            if (QString(data).indexOf(ICYHeaderRegExp) == 0) {
                data = analyzeICYHeader(data);
            } else {
                analyzeHttpHeader(job);
            }
        }

#ifdef DEBUG_DUMP_ICY_STREAMS
        fwrite(data.constData(), data.size(), 1, m_debugFullStream); fflush(m_debugFullStream);
        fprintf(m_debugDecodingLog, "full stream data received: size = %zi @ pos %zi\n", (size_t)data.size(), m_debugFullPos); fflush(m_debugDecodingLog);
        m_debugFullPos += data.size();
#endif

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
                QByteArray mdata(data.constData(), chunk);
                           m_metaRest   -= chunk;
                bool       metaComplete = m_metaRest <= 0;
                           data         = data.mid(chunk);
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
//         printf ("stream CONTINUED\n");
    }
}


void IcyHttpHandler::slotStreamPause()
{
    if (m_streamJob) {
        m_streamJob->suspend();
//         printf ("stream PAUSED\n");
    }
}



void IcyHttpHandler::slotStreamDone(KJob *job)
{
    if (m_streamJob == job) {
        bool local_err = false;
        if (m_streamJob->error()) {
            IErrorLogClient::staticLogError(i18n("Internet Radio Plugin (ICY http handler): Failed to load stream data for %1: %2", m_streamUrl.pathOrUrl(), m_streamJob->errorString()));
            local_err = true;
        }

        KIO::MetaData md = m_streamJob->metaData();
        if (md.contains("HTTP-Headers") && md.contains("responsecode")) {
            int http_response_code = md["responsecode"].toInt();
            if ((http_response_code < 200 || http_response_code >= 300) && http_response_code != 304) {  // skip 304 NOT MODIFIED http response codes
                IErrorLogClient::staticLogError(i18n("Internet Radio Plugin (ICY http handler): HTTP error %1 for stream %2", http_response_code, m_streamUrl.pathOrUrl()));
                local_err = true;
            }
        }
        stopStreamDownload(!local_err);
        if (local_err) {
            emit sigError(m_streamUrl);
        }
    }
    job->deleteLater();
}



