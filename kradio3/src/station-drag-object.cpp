/***************************************************************************
                          station-drag-object.cpp  -  description
                             -------------------
    begin                : Sun Aug 28 2005
    copyright            : (C) 2005 by Martin Witte
    email                : witte@kawo1.rwth-aachen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "include/station-drag-object.h"
#include "include/errorlog-interfaces.h"
#include <klocale.h>

#define STATION_LIST_MIME_TYPE "multimedia/kradio-stationids"

StationDragObject::StationDragObject(const QStringList &stationIDs, QWidget *dragSource, const char * name)
 : QStoredDrag(STATION_LIST_MIME_TYPE, dragSource, name)
{
    setStations(stationIDs);
}

StationDragObject::StationDragObject(QWidget *dragSource, const char * name)
 : QStoredDrag(STATION_LIST_MIME_TYPE, dragSource, name)
{
}


StationDragObject::~StationDragObject()
{
}

const char *StationDragObject::format(int i) const
{
    if (i == 0)
        return STATION_LIST_MIME_TYPE;
    else
        return NULL;
}


void StationDragObject::setStations(const QStringList &stationIDs)
{
    QByteArray tmp;
    int pos = 0;
    for (QValueListConstIterator<QString> it=stationIDs.begin(); it != stationIDs.end(); ++it) {
        const QString &s = *it;
        tmp.resize(tmp.size()+s.length() + 1);
        for (unsigned int k = 0; k < s.length(); ++k) {
            tmp[pos++] = s[k].latin1();
        }
        tmp[pos++] = 0;
    }
    setEncodedData(tmp);
}


bool StationDragObject::canDecode (const QMimeSource *e)
{
    IErrorLogClient::staticLogDebug(e->format(0));
    bool retval = (e && e->format(0) == QString(STATION_LIST_MIME_TYPE));
    if (retval)
        IErrorLogClient::staticLogDebug(i18n("canDecode = true"));
    return retval;
}


bool StationDragObject::decode (const QMimeSource *e, QStringList &stationIDs)
{
    stationIDs.clear();
    if (canDecode(e)) {
        const QByteArray &tmp = e->encodedData(e->format(0));
        QString str = "";
        for (unsigned int pos = 0; pos < tmp.size(); ++pos) {
            if (tmp[pos]) {
                str.append(tmp[pos]);
            } else {
                stationIDs.append(str);
                str = "";
            }
        }
    }
    return true;
}


