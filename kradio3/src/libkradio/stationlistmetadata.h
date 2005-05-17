/***************************************************************************
                          stationlistmetadata.h  -  description
                             -------------------
    begin                : Sat March 29 2003
    copyright            : (C) 2003 by Klas Kalass
    email                : klas@kde.org
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

#ifndef STATIONLISTMETADATA_H
#define STATIONLISTMETADATA_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qstring.h>
#include <qdatetime.h>

/**
 * Meta Data about a stationlist
 * @author  Martin Witte
 */

class StationListMetaData  {
public:
    QString    maintainer;
    QDateTime  lastChange;
    QString    country;
    QString    city;
    QString    media;
    QString    comment;
};

#endif
