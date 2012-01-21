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
    #ifdef HAVE_FFMPEG
        #include <libavformat/avformat.h>
    #endif
    #ifdef HAVE_FFMPEG_OLD
        #include <ffmpeg/avformat.h>
    #endif
}

#include "libav-global.h"

QSharedPointer<LibAVGlobal> LibAVGlobal::m_instance(NULL);


LibAVGlobal::LibAVGlobal()
{
    av_register_all();
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53, 24, 0)
    avformat_network_init();
#endif
}


LibAVGlobal::~LibAVGlobal()
{
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53, 24, 0)
    avformat_network_deinit();
#endif
}


LibAVGlobal *LibAVGlobal::instance()
{
    if (!m_instance) {
        m_instance = QSharedPointer<LibAVGlobal>(new LibAVGlobal());
    }
    return m_instance.data();
}

