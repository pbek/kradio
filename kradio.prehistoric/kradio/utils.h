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

using namespace std;

#include <kconfig.h>
#include <klocale.h>
#include <qstring.h>
#include <list>
#include <vector>
#define __USE_ISOC99 1
#include <math.h>

typedef list<QString>				StringList;
typedef StringList::iterator		iStirngList;
typedef StringList::const_iterator	ciStringList;

typedef vector<int>					IntVector;
typedef IntVector::iterator			iIntVector;
typedef IntVector::const_iterator	ciIntVector;


extern const char *mixerChannelLabels[];
extern const char *mixerChannelNames[];


#endif
