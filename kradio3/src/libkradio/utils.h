/***************************************************************************
                          utils.h  -  description
                             -------------------
    begin                : Sun Feb 3 2002
    copyright            : (C) 2002 by Martin Witte / Frank Schwanz
    email                : witte@kawo1.rwth-aachen.de / schwanz@fh-brandenburg.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRADIO_UTILS_H
#define KRADIO_UTILS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

using namespace std;

#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>
#include <qstring.h>
#define __USE_ISOC99 1
#include <math.h>

extern const char *mixerChannelLabels[];
extern const char *mixerChannelNames[];

extern QString XMLEscape (const QString &s);
QString xmlOpenTag  (const QString &tag, bool newline = true);
QString xmlTag      (const QString &tag, const QString &s, bool newline = true);
QString xmlTag      (const QString &tag, int i,            bool newline = true);
QString xmlTag      (const QString &tag, float f,          bool newline = true);
QString xmlCloseTag (const QString &tag, bool newline = true);

template<class T1, class T2> inline T1 min (T1 a, T2 b) { return a < b ? a : b; }
template<class T1, class T2> inline T1 max (T1 a, T2 b) { return a < b ? b : a; }

#endif
