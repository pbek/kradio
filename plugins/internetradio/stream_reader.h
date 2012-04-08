/***************************************************************************
                          stream_reader.h  -  description
                             -------------------
    begin                : Mon Jan 30 2012
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

#ifndef KRADIO_STREAM_READER_H
#define KRADIO_STREAM_READER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QtCore/QSharedPointer>
#include <kurl.h>

#include <kio/jobclasses.h>


class StreamReader : public QObject
{
Q_OBJECT
public:
    StreamReader();
    virtual ~StreamReader();

    virtual void                startStreamDownload(KUrl url, const QString &metaDataEncoding) = 0;
    virtual void                stopStreamDownload()          = 0;

    virtual KIO::MetaData       getConnectionMetaData() const = 0;

public slots:
    virtual void                slotStreamContinue() = 0;
    virtual void                slotStreamPause()    = 0;

signals:
    void                        sigError   (KUrl url);
    void                        sigFinished(KUrl url);
    void                        sigStarted (KUrl url);
    void                        sigConnectionEstablished(KUrl url, KIO::MetaData metaData);

    void                        sigUrlChanged(KUrl url);
    void                        sigContentType(QString contentType);

    void                        sigStreamData    (QByteArray data);
    void                        sigMetaDataUpdate(KIO::MetaData metadata);
};

Q_DECLARE_METATYPE(KIO::MetaData)

#endif
