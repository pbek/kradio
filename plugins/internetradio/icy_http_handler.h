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

class IcyHttpHandler : public QObject, public IErrorLogClient
{
Q_OBJECT
public:
    IcyHttpHandler(StreamInputBuffer *buffer);
    ~IcyHttpHandler();

    // FIXME: store ICY meta data
    // FIXME: if content-type/encoding is given: use it for decoder autodetection

    // FIXME: parent class with standardized interface
    // FIXME: parent class which cares about playlist handling

    void                        startStreamDownload(const KUrl::List &urls, const InternetRadioStation &rs, int max_retries);
    void                        stopStreamDownload();

signals:
    // sigs emitted for each stream
    void                        sigErrorStream(const KUrl &url);
    void                        sigFinishedStream(const KUrl &url);
    void                        sigStartedStream(const KUrl &url);
    // sigs emitted only for the playlist
    void                        sigErrorPlaylist(const KUrl &url);  // only emitted when all playlist entries fail
    void                        sigFinishedPlaylist(const KUrl &url);
    void                        sigStartedPlaylist(const KUrl &url);

protected slots:
    void                        slotStreamData(KIO::Job *job, QByteArray data);
    void                        slotStreamDone(KJob *job);
    void                        slotStreamContinue();

protected:
    void                        setupStreamJob(const KUrl &url);
    void                        startCurrentStreamJob();
    void                        tryNextStream();
    void                        stopCurrentStreamDownload();

    void                        analyzeHttpHeader(KIO::Job *job);
    void                        handleStreamData(const QByteArray &data);
    void                        handleMetaData(const QByteArray &data, bool complete);

protected:
    bool                        m_httpHeaderAnalyzed;
    size_t                      m_ICYMetaInt;
    size_t                      m_dataRest;
    size_t                      m_metaRest;
    QByteArray                  m_metaData;


    InternetRadioStation        m_currentStation;
    KUrl::List                  m_currentPlaylist;
    KUrl                        m_currentStreamUrl;
    int                         m_currentStreamIdx;
    int                         m_currentStreamRetriesMax;
    int                         m_currentStreamRetriesLeft;
    int                         m_randStreamIdxOffset;

    bool                        m_error;
    bool                        m_active;

    KIO::TransferJob           *m_streamJob;

    StreamInputBuffer          *m_inputBuffer;
};


#endif
