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

#include <kglobal.h>

struct LibAVGlobalStruct
{
    LibAVGlobalStruct()
    {
        av_register_all();
        avformat_network_init();
    }

    ~LibAVGlobalStruct()
    {
        avformat_network_deinit();
    }
};

K_GLOBAL_STATIC(LibAVGlobalStruct, libavinit)

namespace LibAVGlobal {

void ensureInitDone()
{
    Q_UNUSED(*libavinit);
}

}

