/***************************************************************************
                          stationlist.cpp  -  description
                             -------------------
    begin                : Sat March 29 2003
    copyright            : (C) 2003 by Klas Kalass
    email                : klas@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "stationlist.h"
#include "radiostation.h"

#include <kurl.h>

StationList::StationList()
{
}

StationList::~StationList()
{
}

RadioStation* StationList::matchingRadioStation(KURL const &url)
{
    RadioStation *station = m_all.first();
    while (station && !station->urlMatch(url))
        station = m_all.next();
    return station;
}

RadioStation* StationList::matchingRadioStation(float frequency)
{
    RadioStation *station = m_all.first();
    while (station && !station->frequencyMatch(frequency))
        station = m_all.next();
    return station;
}

void StationList::merge(StationList  * other)
{
    if (!other)
        return;

    // merge meta information
    StationListMetaData const & metaData = other->metaData();
    m_metaData.comment += "\n"+i18n("Contains merged Data: ")+"\n";
    if (!metaData.lastChange.isValid())
        m_metaData.comment = "  "+i18n("Last Changed: ")+metaData.lastChange.toString()+"\n";
    if (!metaData.maintainer.isEmpty())
        m_metaData.comment = "  "+i18n("Maintainer: ")+metaData.maintainer+"\n";
    if (!metaData.country.isEmpty())
        m_metaData.comment = "  "+i18n("Country: ")+metaData.country+"\n";
    if (!metaData.city.isEmpty())
        m_metaData.comment = "  "+i18n("City: ")+metaData.city+"\n";
    if (!metaData.media.isEmpty())
        m_metaData.comment = "  "+i18n("Media: ")+metaData.media+"\n";
    if (!metaData.comment.isEmpty())
        m_metaData.comment = "  "+i18n("Comment: ")+metaData.comment+"\n";

    // merge stations
    RadioStation *station = other->all().first();
    while (station) {
        m_all.append(station);
        station = other->all().next();
    }
}

