/***************************************************************************
                          libav-global.h  -  description
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

#ifndef KRADIO_LIBAV_GLOBAL_H
#define KRADIO_LIBAV_GLOBAL_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QtCore/QSharedPointer>

class LibAVGlobal
{
protected:
    LibAVGlobal();
    ~LibAVGlobal();

public:
    static LibAVGlobal *instance();

    static void         ensureInitDone() { instance(); }

protected:

    static  QSharedPointer<LibAVGlobal> m_instance;
};


#endif
