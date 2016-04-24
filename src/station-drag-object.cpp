/***************************************************************************
                          station-drag-object.cpp  -  description
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

#include "station-drag-object.h"
#include "errorlog_interfaces.h"
#include <QDataStream>
#include <QMimeData>

#define STATION_LIST_MIME_TYPE "multimedia/kradio-stationids"

void StationDragObject::encode(QMimeData *data, const QStringList &stationIDs)
{
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    stream << stationIDs;
    data->setData(STATION_LIST_MIME_TYPE, encodedData);
}


bool StationDragObject::canDecode(const QMimeData *data)
{
    IErrorLogClient::staticLogDebug(data->data(STATION_LIST_MIME_TYPE));
    bool retval = (data && data->hasFormat(STATION_LIST_MIME_TYPE));
    if (retval)
        IErrorLogClient::staticLogDebug("canDecode = true");
    return retval;
}


bool StationDragObject::decode(const QMimeData *data, QStringList &stationIDs)
{
    stationIDs.clear();
    if (canDecode(data)) {
        QByteArray encodedData = data->data(STATION_LIST_MIME_TYPE);
        QDataStream stream(&encodedData, QIODevice::ReadOnly);
        stream >> stationIDs;
    }
    return true;
}


