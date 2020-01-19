/***************************************************************************
                          mmsx_handler.cpp  -  description
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

#include "mmsx_handler.h"
#include "mmsx_handler_thread.h"

MMSXHandler::MMSXHandler()
 :  m_mmsxThread(NULL)
{
}


MMSXHandler::~MMSXHandler()
{
}


void MMSXHandler::startStreamDownload(QUrl url, const QString &/*metaDataEncoding*/)
{
    stopStreamDownload();
    m_streamUrl  = url;
    m_mmsxThread = new MMSXHandlerThread(url, this);
    m_mmsxThread->start();
}


void MMSXHandler::stopStreamDownload()
{
    if (m_mmsxThread) {
        m_mmsxThread->stop();
        m_mmsxThread->quit();
        // delete m_mmsxThread; // the thread will delete itself, but only when it really finished.
        m_mmsxThread = NULL;
    }
}

