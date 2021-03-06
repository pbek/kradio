/***************************************************************************
                          playlist_handler.h  -  description
                             -------------------
    begin                : Sun Jan 29 2012
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

#ifndef KRADIO_PLAYLIST_HANDLER_H
#define KRADIO_PLAYLIST_HANDLER_H

#include <QtCore/QUrl>

#include <kio/job.h>
#include <kio/jobclasses.h>

#include "internetradiostation.h"


class PlaylistHandler : public QObject
{
Q_OBJECT
public:
    PlaylistHandler();
    ~PlaylistHandler();

    bool        hadError()         const { return m_error; }
    QUrl        currentStreamUrl() const { return m_currentStreamUrl; }

signals:
    void        sigPlaylistLoaded(const QList<QUrl> &streamList);
    void        sigStreamSelected(QUrl stream);
    void        sigEOL();
    void        sigError(QString errorMsg);

public slots:
    void        setPlayListUrl(const InternetRadioStation &s, int maxStreamRetries);
    void        startPlaylistDownload();
    void        stopPlaylistDownload();
    void        selectNextStream(bool allowRetrySameString, bool errorIfEOL, bool isRetry = true);

    void        resetError();

protected slots:
    void        slotPlaylistData(KIO::Job *job, const QByteArray &data);
    void        slotPlaylistLoadDone(KJob *job);

protected:
    void        loadPlaylistStartJob();
    void        loadPlaylistStopJob();
    void        playlistSuccessfullyLoaded();
    QString     getPlaylistClassFromContentType(const QString &curPlsCls);
    QString     getPlaylistClassFromURL        (const QString &curPlsCls);
    QString     getPlaylistClass();
    void        interpretePlaylistData(const QByteArray &a);
    void        interpretePlaylistLSC(const QByteArray &a);
    void        interpretePlaylistM3U(const QByteArray &playlistData);
    void        interpretePlaylistPLS(const QByteArray &playlistData, bool probe = false);
    void        interpretePlaylistWMV(const QByteArray &playlistData, bool probe = false);
    void        interpretePlaylistASX(const QByteArray &xmlData,      bool probe = false);
    void        interpretePlaylistXSPF(const QByteArray &xmlData,     bool probe = false);
    bool        isTextual(const QByteArray &playlistData);


    void        setError(QString errorMsg);

protected:
    InternetRadioStation        m_currentStation;
    QList<QUrl>                 m_currentPlaylist;
    QUrl                        m_currentStreamUrl;
    int                         m_currentStreamIdx;
    int                         m_maxStreamRetries;
    int                         m_currentStreamRetriesLeft;
    int                         m_randStreamIdxOffset;

    bool                        m_error;

    QByteArray                  m_playlistData;
    QString                     m_contentType;
    KIO::TransferJob           *m_playlistJob;
};


#endif

