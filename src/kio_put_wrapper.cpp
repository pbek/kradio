/***************************************************************************
                     kio_put_wrapper.cpp  -  description
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

#include "kio_put_wrapper.h"

kio_put_wrapper_t::
kio_put_wrapper_t(const QUrl & url, const QByteArray &data, const KIO::JobFlags jobFlags)
  : m_url            (url),
    m_data           (data),
    m_ok             (false),
    m_error          (0),
    m_errorString    (""),
    m_txJob          (nullptr),
    m_jobFlags       (jobFlags),
    m_dataTransferred(0)
{} // CTOR
    
    
kio_put_wrapper_t::
~kio_put_wrapper_t()
{
    if (m_txJob) {
        m_txJob->deleteLater();
        m_txJob = nullptr;
    }
} // DTOR
    
    
bool
kio_put_wrapper_t::
exec()
{
    m_txJob = KIO::put(m_url, -1, m_jobFlags);
    
    m_dataTransferred = 0;    
    connect(m_txJob, &KIO::TransferJob::dataReq, this, &kio_put_wrapper_t::slotDataReq);
    m_txJob->setTotalSize(m_data.size());
    
    m_ok          = m_txJob->exec();
    m_error       = m_txJob->error();
    m_errorString = m_txJob->errorString();
    
    m_txJob->deleteLater();
    m_txJob = nullptr;
    
    return m_ok;
} // exec


void 
kio_put_wrapper_t::
slotDataReq(KIO::Job */*job*/, QByteArray &data)
{
    const size_t maxChunkSize = 512 * 1024; // KF5 recommendation: < 1 MB
    const size_t sizeLeft     = m_data.size() - m_dataTransferred;
    const size_t sizeChunk    = std::min(sizeLeft, maxChunkSize);
    
    data = m_data.mid(m_dataTransferred, sizeChunk);
    
    m_dataTransferred += sizeChunk;
} // slotData
    
