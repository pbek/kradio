/***************************************************************************
                     kio_get_wrapper.cpp  -  description
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

#include "kio_get_wrapper.h"

kio_get_wrapper_t::
kio_get_wrapper_t(const QUrl & url, size_t maxSize)
  : m_url        (url),
    m_maxSize    (maxSize),
    m_ok         (false),
    m_error      (0),
    m_errorString(""),
    m_rxJob      (nullptr)
{} // CTOR
    
    
kio_get_wrapper_t::
~kio_get_wrapper_t()
{
    if (m_rxJob) {
        m_rxJob->deleteLater();
        m_rxJob = nullptr;
    }
} // DTOR
    
    
bool
kio_get_wrapper_t::
exec()
{
    m_rxJob = KIO::get(m_url);
    
    connect(m_rxJob, &KIO::TransferJob::data, this, &kio_get_wrapper_t::slotData);
    
    m_ok          = m_rxJob->exec();
    m_error       = m_rxJob->error();
    m_errorString = m_rxJob->errorString();
    
    m_rxJob->deleteLater();
    m_rxJob = nullptr;
    
    return m_ok;
} // exec


void 
kio_get_wrapper_t::
slotData(KIO::Job */*job*/, const QByteArray &data)
{
    const size_t oldSize      = m_data.size();
    const size_t incSize      = data  .size();
    const size_t maxIncSize   = m_maxSize - oldSize;
    const size_t tunedIncSize = std::min(incSize, maxIncSize);
    
    m_data.append(data.left(tunedIncSize));
    
    if (size_t(m_data.size()) >= m_maxSize) {
        m_rxJob->kill();
    }
} // slotData
    
