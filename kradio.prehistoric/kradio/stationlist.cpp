/***************************************************************************
                          stationlist.cpp  -  description
                             -------------------
    begin                : Sat March 29 2003
    copyright            : (C) 2003 by Klas Kalass, Ernst Martin Witte
    email                : klas@kde.org, witte@kawo1.rwth-aachen.de
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
#include "stationlistxmlhandler.h"

//////////////////////////////////////////////////////////////////////////////

const StationList emptyStationList;

//////////////////////////////////////////////////////////////////////////////

RawStationList::RawStationList ()
{
	setAutoDelete(true);
}


RawStationList::RawStationList (const RawStationList &sl)
	: QPtrList<RadioStation>(sl)
{
	setAutoDelete(true);
}


RawStationList::~RawStationList ()
{
	clear();
}


QPtrCollection::Item RawStationList::newItem (QPtrCollection::Item s)
{
	if (s)
		return ((RadioStation*)s)->copy();
	else
		return NULL;
}


void  RawStationList::deleteItem (QPtrCollection::Item s)
{
	if (autoDelete())
		delete (RadioStation*)s;
}


int RawStationList::compareItems(QPtrCollection::Item a, QPtrCollection::Item b)
{
	if (!a && !b)
		return 0;
		
	if (!a)
		return -1;
		
	if (!b)
		return 1;
		
	return ((RadioStation*)a)->compare(*(RadioStation*)b);
}


//////////////////////////////////////////////////////////////////////////////

StationList::StationList()
{
	m_all.setAutoDelete(true);
}

StationList::StationList(const StationList &sl)
	: m_all      (sl.m_all),
	  m_metaData (sl.m_metaData)
{
	m_all.setAutoDelete(true);
}


StationList::~StationList()
{
}


void StationList::merge(const StationList & other)
{
    // merge meta information: honor merge in comment

    StationListMetaData const & metaData = other.metaData();
    
    if (! m_metaData.comment.isEmpty())
		m_metaData.comment += "\n";
		
    m_metaData.comment += i18n("Contains merged Data: ") + "\n";
    
    if (!metaData.lastChange.isValid())
        m_metaData.comment = "  " + i18n("Last Changed: ") + metaData.lastChange.toString() + "\n";
        
    if (!metaData.maintainer.isEmpty())
        m_metaData.comment = "  " + i18n("Maintainer: ")   + metaData.maintainer + "\n";
        
    if (!metaData.country.isEmpty())
        m_metaData.comment = "  " + i18n("Country: ")      + metaData.country    + "\n";
        
    if (!metaData.city.isEmpty())
        m_metaData.comment = "  " + i18n("City: ")         + metaData.city       + "\n";
        
    if (!metaData.media.isEmpty())
        m_metaData.comment = "  " + i18n("Media: ")        + metaData.media      + "\n";
        
    if (!metaData.comment.isEmpty())
        m_metaData.comment = "  " + i18n("Comment: ")      + metaData.comment    + "\n";


    // merge stations
    
    QPtrListIterator<RadioStation> it(other.all());
    for (RadioStation *s = it.current(); s; s = ++it) {
        m_all.append(s);
    }
}



StationList &StationList::operator = (const StationList &other)
{
	m_metaData = other.metaData();
	m_all = other.all();
	return *this;
}


const RadioStation &StationList::at(int idx) const
{
	RawStationList::Iterator it(m_all);
	it += idx;
	return it.current() ? *it.current() : (const RadioStation &) undefinedRadioStation;
}


RadioStation &StationList::at(int idx)
{
	RawStationList::Iterator it(m_all);
	it += idx;
	return it.current() ? *it.current() : (RadioStation &) undefinedRadioStation;
}



bool StationList::readXML (const QString &dat)
{
	// TODO: error handling
	QXmlInputSource source;
	source.setData(dat);
	QXmlSimpleReader      reader;
	StationListXmlHandler handler;
	reader.setContentHandler (&handler);
	if (reader.parse(source)) {
		m_all      = handler.getStations();
		m_metaData = handler.getMetaData();
		return true;
	} else {
		kdDebug() << "StationList::readXML: parsing failed\n";
		return false;
	}
}



QString StationList::writeXML () const
{
	QString data = "";

	// write station list

	QString t   = "\t";
	QString tt  = "\t\t";
	QString ttt = "\t\t\t";

	data +=       xmlOpenTag(KRadioConfigElement) +
	        t   + xmlOpenTag(StationListElement) +
	        tt  + xmlOpenTag(StationListInfo) +
	        ttt + xmlTag(StationListInfoMaintainer, m_metaData.maintainer) +
	        ttt + xmlTag(StationListInfoChanged,    m_metaData.lastChange.toString(Qt::ISODate)) +
	        ttt + xmlTag(StationListInfoCountry,    m_metaData.country) +
	        ttt + xmlTag(StationListInfoCity,       m_metaData.city) +
	        ttt + xmlTag(StationListInfoMedia,      m_metaData.media) +
	        ttt + xmlTag(StationListInfoComments,   m_metaData.comment) +
	        tt  + xmlCloseTag (StationListInfo);

	for (RawStationList::Iterator it(m_all); it.current(); ++it) {
	    RadioStation *s = it.current();

		data += tt + xmlOpenTag (s->getClassName());
		
	    QStringList properties = s->getPropertyNames();
	    for (QStringList::iterator sit = properties.begin(); sit != properties.end(); ++sit) {			
			data += ttt + xmlTag (*sit, s->getProperty(*sit));
		}
		data += tt + xmlCloseTag(s->getClassName());
		
	}
	
	data += t + xmlCloseTag(StationListElement) +
			    xmlCloseTag(KRadioConfigElement);

	return data;
}

