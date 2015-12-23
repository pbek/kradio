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

#include <QSharedPointer>
#include <kurl.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kencodingprober.h>

#include "stream_reader.h"

#include <stdio.h>

// if this macro is defined, the full icy stream and the demuxed data and
// metadata streams as well as some log information about chunk sizes and positions
// in the streams are dumped to statically named files in /tmp
//
// #define DEBUG_DUMP_ICY_STREAMS

class IcyHttpHandler : public StreamReader
{
Q_OBJECT
public:
    IcyHttpHandler();
    ~IcyHttpHandler();

    void                        startStreamDownload(KUrl url, const QString &metaDataEncoding);
    void                        stopStreamDownload() { stopStreamDownload(true); }

    KIO::MetaData               getConnectionMetaData() const { return m_connectionMetaData; }

public slots:
    void                        slotStreamContinue();
    void                        slotStreamPause();

protected slots:
    void                        slotStreamData(KIO::Job *job, QByteArray data);
    void                        slotStreamDone(KJob *job);

protected:
    void                        stopStreamDownload(bool emitSigFinished);
    void                        setupStreamJob(const KUrl &url, const QString &metaDataEncoding);
    void                        startStreamJob();

    QMap<QString, QString>      splitExtractHttpHeaderKeys(QString httpHeader);
    void                        analyzeHttpHeader(KIO::Job *job);
    QByteArray                  analyzeICYHeader(QByteArray packet);
    void                        analyzeHttpHeader(QString httpHeader, KIO::MetaData &metaData);

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

    QString                     m_metaDataEncoding;
    QTextCodec                 *m_metaDataEncodingCodec;
    KEncodingProber             m_metaDataEncodingProber;

#ifdef DEBUG_DUMP_ICY_STREAMS
    FILE                       *m_debugFullStream;
    FILE                       *m_debugMetaStream;
    FILE                       *m_debugDataStream;
    FILE                       *m_debugDecodingLog;
    size_t                      m_debugFullPos;
    size_t                      m_debugDataPos;
    size_t                      m_debugMetaPos;
#endif
};


#endif
