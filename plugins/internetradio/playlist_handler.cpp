/***************************************************************************
                          playlist_handler.cpp  -  description
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

#include "playlist_handler.h"
#include <errorlog_interfaces.h>

#include <klocale.h>
#include <ktemporaryfile.h>
#include <kencodingprober.h>
#include <kconfiggroup.h>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QTextCodec>

#include <math.h>

PlaylistHandler::PlaylistHandler()
  : m_currentStreamIdx        (-1),
    m_maxStreamRetries        (2),
    m_currentStreamRetriesLeft(0),
    m_randStreamIdxOffset     (0),
    m_error                   (false),
    m_playlistJob             (NULL)
{
}


PlaylistHandler::~PlaylistHandler()
{
    if (m_playlistJob) {
        stopPlaylistDownload();
    }
}


void PlaylistHandler::resetError()
{
    m_error = false;
}

void PlaylistHandler::setPlayListUrl(const InternetRadioStation &s, int maxStreamRetries)
{
    m_currentStation   = s;
    m_maxStreamRetries = maxStreamRetries;
}


void PlaylistHandler::startPlaylistDownload()
{
    resetError();
    loadPlaylistStartJob();
}


void PlaylistHandler::stopPlaylistDownload()
{
    loadPlaylistStopJob();
}


void  PlaylistHandler::selectNextStream(bool allowRetrySameString, bool errorIfEOL)
{
    if (--m_currentStreamRetriesLeft < 0 || !allowRetrySameString) {
        ++m_currentStreamIdx;
        m_currentStreamRetriesLeft = m_maxStreamRetries;
    }
    if (m_currentStreamIdx < m_currentPlaylist.size()) {
        int     realIdx    = (m_currentStreamIdx + m_randStreamIdxOffset) % m_currentPlaylist.size();
        m_currentStreamUrl = m_currentPlaylist[realIdx];
        emit sigStreamSelected(m_currentStreamUrl);
    } else {
        emit sigEOL();
        if (errorIfEOL) {
            setError(i18n("Failed to start any stream of %1").arg(m_currentStation.longName()));
        }
    }
}


void PlaylistHandler::setError(QString errorMsg)
{
    IErrorLogClient::staticLogError(errorMsg);
    stopPlaylistDownload();
    m_error = true;
    emit sigError(errorMsg);
}


void PlaylistHandler::loadPlaylistStopJob()
{
    if (m_playlistJob) {
        QObject::disconnect(m_playlistJob, SIGNAL(data  (KIO::Job *, const QByteArray &)), this, SLOT(slotPlaylistData(KIO::Job *, const QByteArray &)));
        QObject::disconnect(m_playlistJob, SIGNAL(result(KJob *)),                         this, SLOT(slotPlaylistLoadDone(KJob *)));
        m_playlistJob->kill();
        m_playlistJob = NULL;
    }
}


void PlaylistHandler::loadPlaylistStartJob()
{
    loadPlaylistStopJob();
    // if the a new station is selected while the previous hasn't been loaded yet
    m_playlistData   .clear();
    m_currentPlaylist.clear();
    if (getPlaylistClass().length()) {
        m_playlistJob = KIO::get(m_currentStation.url(), KIO::NoReload, KIO::HideProgressInfo);
        if (m_playlistJob) {
            QObject::connect(m_playlistJob, SIGNAL(data  (KIO::Job *, const QByteArray &)), this, SLOT(slotPlaylistData(KIO::Job *, const QByteArray &)));
            QObject::connect(m_playlistJob, SIGNAL(result(KJob *)),                         this, SLOT(slotPlaylistLoadDone(KJob *)));
            m_playlistJob->start();
            if (m_playlistJob->error()) {
                setError(i18n("Failed to load playlist %1: %2").arg(m_currentStation.url().pathOrUrl()).arg(m_playlistJob->errorString()));
            }
        } else {
            setError(i18n("Failed to start playlist download of %1: KIO::get returned NULL pointer").arg(m_currentStation.url().pathOrUrl()));
        }
    } else {
        interpretePlaylistData(QByteArray());
    }
}


void PlaylistHandler::slotPlaylistData(KIO::Job *job, const QByteArray &data)
{
    if (job == m_playlistJob) {
        m_playlistData.append(data);
    }
}


void PlaylistHandler::slotPlaylistLoadDone(KJob *job)
{
    if (job == m_playlistJob) {
        if (m_playlistJob->error()) {
            setError(i18n("Failed to load playlist %1: %2").arg(m_currentStation.url().pathOrUrl()).arg(m_playlistJob->errorString()));
        } else {
            interpretePlaylistData(m_playlistData);
        }
        m_playlistJob = NULL;
    }
    job->deleteLater();
}


QString PlaylistHandler::getPlaylistClass()
{
    QString plscls = m_currentStation.playlistClass();
    QString path   = m_currentStation.url().path();
    if        (plscls == "lsc" || (plscls == "auto" && path.endsWith(".lsc"))) {
            return "lsc";
    } else if (plscls == "m3u" || (plscls == "auto" && path.endsWith(".m3u"))) {
            return "m3u";
    } else if (plscls == "asx" || (plscls == "auto" && path.endsWith(".asx"))) {
            return "asx";
    } else if (plscls == "pls" || (plscls == "auto" && path.endsWith(".pls"))) {
            return "pls";
    } else {
            return "";
    }
}


void PlaylistHandler::interpretePlaylistData(const QByteArray &a)
{
//     IErrorLogClient::staticLogDebug("PlaylistHandler::interpretePlaylist");
    QString plscls = getPlaylistClass();
    m_currentPlaylist.clear();
    if        (plscls == "lsc") {
            interpretePlaylistLSC(a);
    } else if (plscls == "m3u") {
            interpretePlaylistM3U(a);
    } else if (plscls == "asx") {
            interpretePlaylistASX(a);
    } else if (plscls == "pls") {
            interpretePlaylistPLS(a);
    } else {
        m_currentPlaylist.append(m_currentStation.url());
    }
    if (!m_currentPlaylist.size()) {
        setError(i18n("%1 does not contain any usable radio stream", m_currentStation.url().pathOrUrl()));
    } else {
        playlistSuccessfullyLoaded();
    }

/*    logDebug("Playlist:");
    foreach (KUrl url, m_currentPlaylist) {
        logDebug(url.pathOrUrl());
    }*/
}


void PlaylistHandler::playlistSuccessfullyLoaded()
{
    m_randStreamIdxOffset      = rint((m_currentPlaylist.size() - 1) * (float)rand() / (float)RAND_MAX);
    m_currentStreamIdx         = 0;
    m_currentStreamRetriesLeft = m_maxStreamRetries;
    emit sigPlaylistLoaded(m_currentPlaylist);
}


void PlaylistHandler::interpretePlaylistLSC(const QByteArray &a)
{
    interpretePlaylistM3U(a);
    if (!m_currentPlaylist.size()) {
        interpretePlaylistASX(a);
    }
}

void PlaylistHandler::interpretePlaylistM3U(const QByteArray &playlistData)
{
    QStringList lines = QString(playlistData).split("\n");
    foreach (QString line, lines) {
        QString t = line.trimmed();
        if (t.length() > 5 && !t.startsWith("#")) {
            m_currentPlaylist.append(t);
        }
    }
}


void PlaylistHandler::interpretePlaylistPLS(const QByteArray &playlistData)
{
    KTemporaryFile tmpFile;
    tmpFile.setAutoRemove(true);
    if (!tmpFile.open()) {
        setError(i18n("failed to create temporary file to store playlist data"));
        return;
    }
    if (tmpFile.write(playlistData) != playlistData.size()) {
        setError(i18n("failed to write temporary file to store playlist data"));
        return;
    }
    tmpFile.close();

    KConfig      cfg(tmpFile.fileName());

    // mapping group names to lower case in order to be case insensitive
    QStringList            groups = cfg.groupList();
    QMap<QString, QString> group_lc_map;
    QString                grp;
    foreach(grp, groups) {
        group_lc_map.insert(grp.toLower(), grp);
    }

    KConfigGroup cfggrp = cfg.group(group_lc_map["playlist"]);

    // mapping entry keys to lower case in order to be case insensitive
    QStringList keys = cfggrp.keyList();
    QMap<QString, QString> key_lc_map;
    QString key;
    foreach(key, keys) {
        key_lc_map.insert(key.toLower(), key);
    }

    unsigned int entries = cfggrp.readEntry(key_lc_map["numberofentries"], 0);
    if (entries) {
        for (unsigned int i = 0; i < entries; ++i) {
            QString url = cfggrp.readEntry(key_lc_map[QString("file%1").arg(i)], QString());
            if (url.length()) {
                m_currentPlaylist.append(url);
            }
        }
    }
}


void PlaylistHandler::interpretePlaylistASX(const QByteArray &rawData)
{

    KEncodingProber prober;
    prober.feed(rawData);

    QXmlStreamReader reader(QTextCodec::codecForName(prober.encoding())->toUnicode(rawData));

    bool inEntry = false;

    while (!reader.atEnd() && (reader.error() == QXmlStreamReader::NoError)) {
        reader.readNext();
        if (reader.isStartElement()) {
            QStringRef name = reader.name();
            if (name.toString().toLower() == "entry") {
                inEntry = true;
            }
            else if (name.toString().toLower() == "ref" && inEntry) {
                QXmlStreamAttributes attrs = reader.attributes();
                QXmlStreamAttribute  attr;
                foreach(attr, attrs) {
                    if(attr.name().toString().toLower() == "href") {
                        m_currentPlaylist.append(attr.value().toString());
                    }
                }
            }
        }
        else if (reader.isEndElement()) {
            QStringRef name = reader.name();
            if (name == "entry") {
                inEntry = false;
            }
        }
    }

    if (reader.error() != QXmlStreamReader::NoError) {
        setError(i18n("error while reading asx file: ", reader.error()));
    }
}




