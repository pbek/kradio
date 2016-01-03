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

#include <klocalizedstring.h>
#include <ktemporaryfile.h>
#include <kencodingprober.h>
#include <kconfiggroup.h>
#include <kconfig.h>
#include <QXmlStreamReader>
#include <QTextCodec>

#include <math.h>


#define MAX_PLAYLIST_PROBE_SIZE 8192

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


void  PlaylistHandler::selectNextStream(bool allowRetrySameString, bool errorIfEOL, bool isRetry)
{
    m_currentStreamRetriesLeft -= isRetry;
    if (m_currentStreamRetriesLeft < 0 || !allowRetrySameString) {
        ++m_currentStreamIdx;
        if (!isRetry) {
            m_currentStreamIdx %= m_currentPlaylist.size();
        }
        m_currentStreamRetriesLeft = m_maxStreamRetries;
    }
    if (m_currentStreamIdx < m_currentPlaylist.size()) {
        int     realIdx    = (m_currentStreamIdx + m_randStreamIdxOffset) % m_currentPlaylist.size();
        m_currentStreamUrl = m_currentPlaylist[realIdx];
        emit sigStreamSelected(m_currentStreamUrl);
    } else {
        emit sigEOL();
        if (errorIfEOL) {
            setError(i18n("Failed to start any stream of %1", m_currentStation.longName()));
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
    m_contentType    .clear();
    m_playlistData   .clear();
    m_currentPlaylist.clear();
    IErrorLogClient::staticLogDebug(QString::fromLatin1("Internet Radio Plugin (Playlist handler): loading playlist %1").arg(m_currentStation.url().url()));
    QString protocol = m_currentStation.url().protocol();
    // protocol mms / mmsx can only be plain streams and cannot be interpreted by KDE
    // also if playlist class is set to "" (none) we skip the playlist download
    if (!protocol.startsWith("mms") && m_currentStation.playlistClass() != "") {
        m_playlistJob = KIO::get(m_currentStation.url(), KIO::NoReload, KIO::HideProgressInfo);
        if (m_playlistJob) {
            QObject::connect(m_playlistJob, SIGNAL(data  (KIO::Job *, const QByteArray &)), this, SLOT(slotPlaylistData(KIO::Job *, const QByteArray &)));
            QObject::connect(m_playlistJob, SIGNAL(result(KJob *)),                         this, SLOT(slotPlaylistLoadDone(KJob *)));
            m_playlistJob->start();
            if (m_playlistJob->error()) {
                setError(i18n("Failed to load playlist %1: %2", m_currentStation.url().pathOrUrl(), m_playlistJob->errorString()));
            }
        } else {
            setError(i18n("Failed to start playlist download of %1: KIO::get returned NULL pointer", m_currentStation.url().pathOrUrl()));
        }
    } else {
        interpretePlaylistData(QByteArray());
    }
}


void PlaylistHandler::slotPlaylistData(KIO::Job *job, const QByteArray &data)
{
    if (job == m_playlistJob) {
        m_playlistData.append(data);

        // if we do autodetection, load max 4kB
        if (m_currentStation.playlistClass() == "auto") {
            if (m_playlistData.size() >= MAX_PLAYLIST_PROBE_SIZE) {
                slotPlaylistLoadDone(m_playlistJob);
            }
        }
    }
}


void PlaylistHandler::slotPlaylistLoadDone(KJob *job)
{
    bool local_error = false;
    if (job == m_playlistJob) {
        if (m_playlistData.size() == 0 && m_playlistJob->error()) {
            setError(i18n("Failed to load playlist %1: %2", m_currentStation.url().pathOrUrl(), m_playlistJob->errorString()));
            local_error = true;
        } else {
            KIO::MetaData md = m_playlistJob->metaData();
            if (md.contains("responsecode")) {
                int http_response_code = md["responsecode"].toInt();
                if ((http_response_code < 200 || http_response_code >= 300) && http_response_code != 304 && http_response_code != 0) {  // skip 304 NOT MODIFIED http response codes
                    setError(i18n("Internet Radio Plugin (Playlist handler): HTTP error %1 for stream %2", http_response_code, m_currentStation.url().pathOrUrl()));
                    local_error = true;
                }
            }
            foreach(QString k, md.keys()) {
                QString v = md[k];
                IErrorLogClient::staticLogDebug(QString("Internet Radio Plugin (Playlist handler):      %1 = %2").arg(k).arg(v));
                if (k == "HTTP-Headers") {
//                     analyzeHttpHeader(v, m_connectionMetaData);
                }
                else if (k == "content-type") {
                    m_contentType = v;
                }
            }
        }
        m_playlistJob = NULL;

        if (!local_error) {
            interpretePlaylistData(m_playlistData);
        }
    }
    job->deleteLater();
}


QString PlaylistHandler::getPlaylistClassFromContentType(const QString &curPlsCls)
{
    QString plscls = curPlsCls;
    if (plscls == "auto" && m_contentType.length()) {
        if        (m_contentType == "audio/x-scpls") {
            plscls = "pls";
        } else if (m_contentType == "application/xspf+xml") {
            plscls = "xspf";
        }
    }
    return plscls;
}


QString PlaylistHandler::getPlaylistClassFromURL(const QString &curPlsCls)
{
    QString plscls = curPlsCls;
    if (plscls == "auto") {
        QString path   = m_currentStation.url().path();
        if        (path.endsWith(".lsc")) {
            plscls = "lsc";
        } else if (path.endsWith(".m3u")) {
            plscls = "m3u";
        } else if (path.endsWith(".asx")) {
            plscls = "asx";
        } else if (path.endsWith(".pls")) {
            plscls = "pls";
        } else if (path.endsWith(".xspf")) {
            plscls = "xspf";
        }
    }
    return plscls;
}


QString PlaylistHandler::getPlaylistClass()
{
    QString plscls = m_currentStation.playlistClass();
    plscls = getPlaylistClassFromURL        (plscls);
    plscls = getPlaylistClassFromContentType(plscls);
    IErrorLogClient::staticLogDebug(QString("Internet Radio Plugin (Playlist handler): playlist type \"%1\" detected").arg(plscls));
    return plscls;
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
    } else if (plscls == "xspf") {
        interpretePlaylistXSPF(a);
    } else if (plscls == "wmv") {
        interpretePlaylistWMV(a);
    } else if (plscls == "auto") {
        // if it is still auto, we have to probe
        if (!m_currentPlaylist.size()) {
            interpretePlaylistASX(a, /* probe */ true);
        }
        if (!m_currentPlaylist.size()) {
            interpretePlaylistPLS(a, /* probe */ true);
        }
        if (!m_currentPlaylist.size()) {
            interpretePlaylistWMV(a, /* probe */ true);
        }
        if (!m_currentPlaylist.size()) {
            interpretePlaylistXSPF(a, /* probe */ true);
        }
        // if still empty: no known playlist, thus try as plain stream
        if (!m_currentPlaylist.size()) {
            m_currentPlaylist.append(m_currentStation.url());
        }
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
    m_randStreamIdxOffset      = 0; //rint((m_currentPlaylist.size() - 1) * (float)rand() / (float)RAND_MAX);
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


bool PlaylistHandler::isTextual(const QByteArray &playlistData)
{
    for (int i = 0; i < playlistData.size(); ++i) {
        unsigned char b = playlistData[i];
        switch(b) {
            case 0x00:
            case 0x01:
            case 0x02:
            case 0x03:
            case 0x04:
            case 0x05:
            case 0x06:
            case 0x07:
            case 0x08:
//             case 0x09: // tab
//             case 0x0a: // \n
            case 0x0b:
            case 0x0c:
//             case 0x0d: // \r
            case 0x0e:
            case 0x0f:
            case 0x10:
            case 0x11:
            case 0x12:
            case 0x13:
            case 0x14:
            case 0x15:
            case 0x16:
            case 0x17:
            case 0x18:
            case 0x19:
            case 0x1a:
//             case 0x1b: // esc
            case 0x1c:
            case 0x1d:
            case 0x1e:
            case 0x1f:
                return false;
                break;
            default:
                break;
        }
    }
    return true;
}


void PlaylistHandler::interpretePlaylistWMV(const QByteArray &playlistData, bool /*probe*/)
{
    // simple check if it is text and if it contains some required syntax parts
    if (!isTextual(playlistData)) {
        return;
    }
    QByteArray tmp = playlistData.toLower();
    if (!tmp.contains("[reference]")) {
        return;
    }

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

    KConfig      cfg(tmpFile.fileName(), KConfig::SimpleConfig);

    // mapping group names to lower case in order to be case insensitive
    QStringList            groups = cfg.groupList();
    QMap<QString, QString> group_lc_map;
    QString                grp;
    foreach(grp, groups) {
        group_lc_map.insert(grp.toLower(), grp);
    }

    KConfigGroup cfggrp = cfg.group(group_lc_map["reference"]);

    // mapping entry keys to lower case in order to be case insensitive
    QStringList keys = cfggrp.keyList();
    QMap<QString, QString> key_lc_map;
    QString key;
    foreach(key, keys) {
        key_lc_map.insert(key.toLower(), key);
    }

    unsigned int entries = keys.size();
    if (entries) {
        // some pls files start with offset 1, some at offset 0... thus we need to start at index 0 and stop at index #entries
        for (unsigned int i = 0; i <= entries; ++i) {
            QString url = cfggrp.readEntry(key_lc_map[QString("ref%1").arg(i)], QString());
            if (url.length()) {
                KUrl tmp = url;
                tmp.setProtocol("mms");
                m_currentPlaylist.append(tmp.url());
            }
        }
    }
}



void PlaylistHandler::interpretePlaylistPLS(const QByteArray &playlistData, bool /*probe*/)
{
    // simple check if it is text and if it contains some required syntax parts
    if (!isTextual(playlistData)) {
        return;
    }
    QByteArray tmp = playlistData.toLower();
    if (!tmp.contains("[playlist]")) {
        return;
    }

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

    KConfig      cfg(tmpFile.fileName(), KConfig::SimpleConfig);

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
        // some pls files start with offset 1, some at offset 0... thus we need to start at index 0 and stop at index #entries
        for (unsigned int i = 0; i <= entries; ++i) {
            QString url = cfggrp.readEntry(key_lc_map[QString("file%1").arg(i)], QString());
            if (url.length()) {
                m_currentPlaylist.append(url);
            }
        }
    }
}


void PlaylistHandler::interpretePlaylistASX(const QByteArray &rawData, bool probe)
{

    KEncodingProber prober;
    prober.feed(rawData);

    QXmlStreamReader reader(QTextCodec::codecForName(prober.encoding())->toUnicode(rawData));

    bool inEntry = false;
    bool inASX   = false;

    while (!reader.atEnd() && (reader.error() == QXmlStreamReader::NoError)) {
        reader.readNext();
        if (reader.isStartElement()) {
            QStringRef name   = reader.name();
            QString    nameLC = name.toString().toLower();
            if (nameLC == "entry") {
                inEntry = true;
            }
            else if (nameLC == "asx") {
                inASX = true;
            }
            else if (nameLC == "ref" && inEntry && inASX) {
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
            QStringRef name   = reader.name();
            QString    nameLC = name.toString().toLower();
            if (nameLC == "entry") {
                inEntry = false;
            }
            else if (nameLC == "asx") {
                inASX = false;
            }
        }
    }

    if (!probe && reader.error() != QXmlStreamReader::NoError) {
        setError(i18n("error while reading asx file: %1", reader.error()));
    }
}

void PlaylistHandler::interpretePlaylistXSPF(const QByteArray &rawData, bool probe)
{
    QXmlStreamReader reader(rawData);

    bool inPlaylist = false;
    bool inTrackList = false;
    bool inTrack = false;
    bool inLocation = false;

    while (!reader.atEnd() && (reader.error() == QXmlStreamReader::NoError)) {
        const QXmlStreamReader::TokenType token = reader.readNext();
        QStringRef name;
        switch (token) {
        case QXmlStreamReader::StartElement:
            name = reader.name();
            if (inTrack && name.compare(QLatin1String("location")) == 0) {
                inLocation = true;
            } else if (inTrackList && name.compare(QLatin1String("track")) == 0) {
                inTrack = true;
            } else if (inPlaylist && name.compare(QLatin1String("trackList")) == 0) {
                inTrackList = true;
            } else if (name.compare(QLatin1String("playlist")) == 0) {
                inPlaylist = true;
            }
            break;
        case QXmlStreamReader::EndElement:
            name = reader.name();
            if (name.compare(QLatin1String("location")) == 0) {
                inLocation = false;
            } else if (name.compare(QLatin1String("track")) == 0) {
                inTrack = false;
            } else if (name.compare(QLatin1String("trackList")) == 0) {
                inTrackList = false;
            } else if (name.compare(QLatin1String("playlist")) == 0) {
                inPlaylist = false;
            }
            break;
        case QXmlStreamReader::Characters:
            if (inLocation) {
                m_currentPlaylist.append(reader.text().toString());
            }
            break;
        default:
            break;
        }
    }

    if (!probe && reader.error() != QXmlStreamReader::NoError) {
        setError(i18n("error while reading XSPF file: %1", reader.error()));
    }
}




