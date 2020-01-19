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

#ifndef KRADIO_MMSX_HANDLER_H
#define KRADIO_MMSX_HANDLER_H

#include <QSharedPointer>
#include <QtCore/QUrl>

#include "stream_reader.h"

class MMSXHandlerThread;

class MMSXHandler : public StreamReader
{
Q_OBJECT
public:
    MMSXHandler();
    ~MMSXHandler();

    void                        startStreamDownload(QUrl url, const QString &metaDataEncoding) override;
    void                        stopStreamDownload() override;

    KIO::MetaData               getConnectionMetaData() const override { return m_connectionMetaData; }

public slots:
    virtual void                slotStreamContinue() override {} // currently not supported
    virtual void                slotStreamPause()    override {}

public slots:

    void    proxyError   (QUrl url)                                      { emit sigError(url);               }
    void    proxyFinished(QUrl url)                                      { emit sigFinished(url);            }
    void    proxyStarted (QUrl url)                                      { emit sigStarted(url);             }
    void    proxyConnectionEstablished(QUrl url, KIO::MetaData metaData) { m_connectionMetaData = metaData; emit sigConnectionEstablished(url, metaData); }

    void    proxyUrlChanged(QUrl url)                                    { emit sigUrlChanged(url);          }
    void    proxyContentType(QString contentType)                        { emit sigContentType(contentType); }

    void    proxyStreamData    (QByteArray data)                         { emit sigStreamData(data);         }
    void    proxyMetaDataUpdate(KIO::MetaData metaData)                  { m_connectionMetaData = metaData; emit sigMetaDataUpdate(metaData); }


protected:

    QUrl                        m_streamUrl;
    KIO::MetaData               m_connectionMetaData;
    MMSXHandlerThread          *m_mmsxThread;
};


#endif
