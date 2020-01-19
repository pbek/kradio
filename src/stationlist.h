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

#include "stationlistmetadata.h"
#include "errorlog_interfaces.h"

#include <QList>

class RadioStation;
class QUrl;

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

//#warning "FIXME: abolish RawStationList. only use qlist<RadioStation*> as member of StationList"
// class RawStationList : protected QList<RadioStation*>
// {
// public:
// 
//     typedef QList<RadioStation*>::iterator Iterator;
//     typedef QList<RadioStation*>           BaseClass;
// 
// public:
//     RawStationList ();
//     RawStationList (const RawStationList &sl);
//     ~RawStationList ();
// 
//     // inheritance is now protected. all manipulation functions must be declared here as public. Let's see which compiler errors occurr so that we can populate the access/manipulation functions...
// 
// 
//     size_t count() const { return QList<RadioStation*>::count(); }
// 
// //     // overwrite all insert-methods in order to change
// //     // multiple insertion of same station_id into an update
// // 
// //     bool insert  (uint index, const RadioStation *item);
// //     bool insert  (const RadioStation *item);
// //     void inSort  (const RadioStation *item);
// //     void prepend (const RadioStation *item);
// //     void append  (const RadioStation *item);
// //     bool replace (uint index, const RadioStation *item);
// 
//     // simplify stationIDSearch
// 
//     const RadioStation &  stationWithID(const QString &sid) const;
//           RadioStation &  stationWithID(const QString &sid);
// 
//     int                   idxWithID(const QString &sid) const;
// 
//     bool                  operator == (const RawStationList &l) const;
//     bool                  operator != (const RawStationList &l) const { return !operator==(l); }
// 
// protected:
// 
// /*    value_type           newItem (value_type s);
//     void                 deleteItem (value_type s);
// 
//     int compareItems (value_type a, value_type b);*/
// };



class QXmlInputSource;


/**
 * Contains a list of stations, including meta data
 * @author Ernst Martin Witte, Klas Kalass
 */
 
class KRADIO5_EXPORT StationList  {
public:
    StationList();
    StationList(const StationList &sl);
    ~StationList();

    // some usefull "proxy" functions

    int                   count() const { return m_stations.count(); }
    const RadioStation &  at(int idx) const;
          RadioStation &  at(int idx);

    void                  moveStation(int old_idx, int new_idx);

    const RadioStation &  stationWithID(const QString &sid) const;
          RadioStation &  stationWithID(const QString &sid);
    int                   idxWithID(const QString &sid) const;

    // all stations, with full access
//     RawStationList &       all()       { return m_all; }
//     RawStationList const & all() const { return m_all; }

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
    StationList &clearStations();
    StationList &setStations(const StationList &x);
    StationList &addStations(const StationList &x);
    StationList &addStation (const RadioStation &x);
    StationList &removeStationAt(int idx);


    // xml in/out

    bool    readXML (const QXmlInputSource &xmlInp, const IErrorLogClient &logger, bool enableMessageBox = true);
    bool    readXML (const QUrl &url,    const IErrorLogClient &logger, bool enableMessageBox = true);

    QString writeXML (const IErrorLogClient &logger) const;
    bool    writeXML (const QUrl &url, const IErrorLogClient &logger, bool enableMessageBox = true) const;


    bool operator == (const StationList &x) const;
    bool operator != (const StationList &x) const { return !operator ==(x); }


    // iteration stuff

    typedef QList<RadioStation*>::iterator       iterator;
    typedef QList<RadioStation*>::const_iterator const_iterator;

    iterator       begin()       { return m_stations.begin(); }
    const_iterator begin() const { return m_stations.begin(); }

    iterator       end()         { return m_stations.end(); }
    const_iterator end()   const { return m_stations.end(); }

protected:
    QList<RadioStation*>  m_stations;
    StationListMetaData   m_metaData;
};


extern const StationList emptyStationList;

#endif
