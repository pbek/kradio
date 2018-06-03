/***************************************************************************
                          libav-global.cpp
                             -------------------
    begin                : Thu Jan 21 2012
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

extern "C" {
    #include <libavformat/avformat.h>
}

#include "libav-global.h"

QSharedPointer<LibAVGlobal> LibAVGlobal::m_instance(NULL);


LibAVGlobal::LibAVGlobal()
{
    av_register_all();
    avformat_network_init();
}


LibAVGlobal::~LibAVGlobal()
{
    avformat_network_deinit();
}


LibAVGlobal *LibAVGlobal::instance()
{
    if (!m_instance) {
        m_instance = QSharedPointer<LibAVGlobal>(new LibAVGlobal());
    }
    return m_instance.data();
}

