/***************************************************************************
                          station-drag-object.h  -  description
                             -------------------
    begin                : Sun Aug 28 2005
    copyright            : (C) 2005 by Martin Witte
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

#ifndef KRADIO_STATION_DRAG_OBJECT_H
#define KRADIO_STATION_DRAG_OBJECT_H

#include <QStringList>

#include <kdemacros.h>

class QMimeData;

class KDE_EXPORT StationDragObject
{
public:
    static void encode(QMimeData *data, const QStringList &stationIDs);

    static bool canDecode(const QMimeData *data);
    static bool decode(const QMimeData *data, QStringList &stationIDs);
};

#endif

