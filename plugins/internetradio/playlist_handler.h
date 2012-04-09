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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <kurl.h>

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
    KUrl        currentStreamUrl() const { return m_currentStreamUrl; }

signals:
    void        sigPlaylistLoaded(KUrl::List  streamList);
    void        sigStreamSelected(KUrl stream);
    void        sigEOL();
    void        sigError(QString errorMsg);

public slots:
    void        setPlayListUrl(const InternetRadioStation &s, int maxStreamRetries);
    void        startPlaylistDownload();
    void        stopPlaylistDownload();
    void        selectNextStream(bool allowRetrySameString, bool errorIfEOL, bool isRetry = true);

    void        setMaxRetries(int maxRetries);

    void        resetError();

protected slots:
    void        slotPlaylistData(KIO::Job *job, const QByteArray &data);
    void        slotPlaylistLoadDone(KJob *job);

protected:
    void        loadPlaylistStartJob();
    void        loadPlaylistStopJob();
    void        playlistSuccessfullyLoaded();
    QString     getPlaylistClass();
    void        interpretePlaylistData(const QByteArray &a);
    void        interpretePlaylistLSC(const QByteArray &a);
    void        interpretePlaylistM3U(const QByteArray &playlistData);
    void        interpretePlaylistPLS(const QByteArray &playlistData);
    void        interpretePlaylistASX(const QByteArray &xmlData);


    void        setError(QString errorMsg);

protected:
    InternetRadioStation        m_currentStation;
    KUrl::List                  m_currentPlaylist;
    KUrl                        m_currentStreamUrl;
    int                         m_currentStreamIdx;
    int                         m_maxStreamRetries;
    int                         m_currentStreamRetriesLeft;
    int                         m_randStreamIdxOffset;

    bool                        m_error;

    QByteArray                  m_playlistData;
    KIO::TransferJob           *m_playlistJob;
};


#endif

