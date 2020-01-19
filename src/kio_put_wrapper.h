/***************************************************************************
                     kio_put_wrapper.h  -  description
                          -------------------
    begin                : Jan 19, 2020
    copyright            : (C) 2020 by Martin Witte
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

#ifndef KIO_PUT_WRAPPER_H_
#define KIO_PUT_WRAPPER_H_

#include <QtCore/QUrl>
#include <QtCore/QByteArray>
#include <QtCore/QString>

#include <KIO/TransferJob>


#include "kradio-def.h"

/**
  *@author Martin Witte
  */

class KRADIO5_EXPORT kio_put_wrapper_t : public QObject
{
Q_OBJECT
protected:
    
    QUrl                m_url;
    const QByteArray  & m_data;
    bool                m_ok;
    int                 m_error;
    QString             m_errorString;
    KIO::TransferJob  * m_txJob;
    
    size_t              m_dataTransferred;
    
public:
    
    kio_put_wrapper_t(const QUrl & url, const QByteArray & data);
    kio_put_wrapper_t(const kio_put_wrapper_t &) = delete;
    
    ~kio_put_wrapper_t();
    
    
    bool
    ok() const {
        return m_ok;
    }
    
    
    int
    error() const {
        return m_error;
    }   
    
    const QString &
    errorString() const {
        return m_errorString;
    }    
    
    const QByteArray &
    data() const {
        return m_data;
    }    
    
    
    bool
    exec();


protected Q_SLOTS:
    
    void 
    slotDataReq(KIO::Job *job, QByteArray &data);
    
}; // kio_put_wrapper_t


#endif //  KIO_PUT_WRAPPER_H_
