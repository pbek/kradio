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

#ifdef KRADIO_ENABLE_FIXMES
    #warning "Port drag&drop to QT4"
#endif
#include <Qt3Support/Q3DragObject>

#include <kdemacros.h>

class KDE_EXPORT StationDragObject : public Q3StoredDrag
{
public:
    StationDragObject(const QStringList &stationIDs, QWidget *dragSource = NULL, const char * name = NULL);
    StationDragObject(QWidget *dragSource = NULL, const char * name = NULL);
    virtual ~StationDragObject();

    const char *format(int i = 0) const;

    void setStations(const QStringList &stationIDs);

    static bool canDecode (const QMimeSource *e);
    static bool decode (const QMimeSource *e, QStringList &stationIDs);
};

#endif

