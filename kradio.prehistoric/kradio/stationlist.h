/***************************************************************************
                          stationlist.h  -  description
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

#ifndef STATIONLIST_H
#define STATIONLIST_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "stationlistmetadata.h"

// qt includes
#include <qptrlist.h>

// forward declarations
class RadioStation;
class KURL;

/**
 * Contains a list of stations
 * @author Klas Kalass
 */

class StationList  {
public:
    StationList();
    ~StationList();

    // all stations, with full access
    QPtrList<RadioStation> & all() {return m_all;};
    QPtrList<RadioStation> const & all() const {return m_all;};
    // the meta data for this station List, with full access
    StationListMetaData & metaData() {return m_metaData;};
    StationListMetaData const & metaData() const {return m_metaData;};

    /**
     * returns a station from the list that has the same URL
     */
    RadioStation* matchingRadioStation(KURL const &url);
    /**
     * returns a station from the list that has the same frequency
     */
    RadioStation* matchingRadioStation(float frequency);

    /**
     * merges  the other list into this one.
     */
    void merge(StationList * other);

protected:
    QPtrList<RadioStation> m_all;
    StationListMetaData m_metaData;

};

#endif
