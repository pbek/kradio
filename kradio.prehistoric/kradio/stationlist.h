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

/*

   Why an own Station List ?

   RadioStations are used everywhere. But who is responsible for them?
   Especially after a list merge?

   A very simple solution should be a StationList with "deep copies". Though
   this is not very efficient, we can assume, that copy operations do not
   take place very often and thus are not critical.


   Why don't we use QValueList then?

   We are using polymorphic radio stations, thus we cannot use a template
   using instances of a base class and copying them with a copy constructor.
   But as each derived class has its own copy() function, we are able to create
   exact copies from a pointer with the type of our base class "RadioStation".

*/
   

class RawStationList : public QPtrList<RadioStation>
{
public:

	typedef QPtrListIterator<RadioStation>  Iterator;
	typedef QPtrList<RadioStation>          BaseClass;

public:
	RawStationList ();
	RawStationList (const RawStationList &sl);
	~RawStationList ();

	// overwrite all insert-methods in order to change
	// multiple insertion of same station_id into an update

	bool insert  (uint index, const RadioStation *item);
	bool insert  (const RadioStation *item);
	void inSort  (const RadioStation *item);
	void prepend (const RadioStation *item);
	void append  (const RadioStation *item);
	bool replace (uint index, const RadioStation *item);

	// simplify stationIDSearch
	
    const RadioStation &  stationWithID(const QString &sid) const;
          RadioStation &  stationWithID(const QString &sid);

    int                   idxWithID(const QString &sid) const;
          
protected:

	QPtrCollection::Item newItem (QPtrCollection::Item s);
	void                 deleteItem (QPtrCollection::Item s);

	int compareItems (QPtrCollection::Item a, QPtrCollection::Item b);
};




/**
 * Contains a list of stations, including meta data
 * @author Klas Kalass, Ernst Martin Witte
 */

class StationList  {
public:
    StationList();
    StationList(const StationList &sl);
    ~StationList();

    // some usefull "proxy" functions

    int                   count() const { return m_all.count(); }
    const RadioStation &  at(int idx) const;
          RadioStation &  at(int idx);

    const RadioStation &  stationWithID(const QString &sid) const;
          RadioStation &  stationWithID(const QString &sid);

    // all stations, with full access
    RawStationList &       all()       { return m_all; }
    RawStationList const & all() const { return m_all; }

    // the meta data for this station List, with full access
    StationListMetaData &       metaData()       { return m_metaData; }
    StationListMetaData const & metaData() const { return m_metaData; }

    // we do not need a special matchingStation/find/... method because
    // it is already implemented in RawStationList

    /**
     * merges  the other list into this one. creates copies from the stations.
     */
    void merge(const StationList &other);

    // assignment

    StationList &operator = (const StationList &sl);


    // xml in/out

    bool    readXML (const QString &dat, bool enableMessageBox = true);
    bool    readXML (const KURL &url, bool enableMessageBox = true);
    
    QString writeXML () const;
    bool    writeXML (const KURL &url, bool enableMessageBox = true) const;

protected:
    RawStationList        m_all;
    StationListMetaData   m_metaData;

};


extern const StationList emptyStationList;

#endif
