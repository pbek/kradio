/***************************************************************************
                          StationListXmlHandler.cpp  -  description
                             -------------------
    begin                : Son Jan 12 2003
    copyright            : (C) 2003 by Martin Witte
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

#include "stationlistxmlhandler.h"

 
const char *KRadioConfigElement         = "kradiorc";

const char *StationListElement			= "stationlist";

const char *StationListInfo             = "info";
const char *StationListInfoMaintainer   = "maintainer";
const char *StationListInfoCountry      = "country";
const char *StationListInfoCity         = "city";
const char *StationListInfoMedia        = "media";
const char *StationListInfoComments     = "comments";
const char *StationListInfoChanged      = "changed";

//const char *StationElement			= "station";
const char *StationQuickSelectElement	= "quickselect";
const char *StationDockingMenuElement   = "dockingmenu";



StationListXmlHandler::StationListXmlHandler ()
{
	m_newStation = NULL;
}


StationListXmlHandler::~StationListXmlHandler ()
{
}


bool StationListXmlHandler::startDocument ()
{
	m_status.clear();

	m_stations.clear();
	clearNewStation();

	return true;
}



#define START_ELEMENT_ERROR    kdDebug() << "StationListXmlHandler::startElement: " \
                                         << (const char*)i18n("misplaced element") << " " \
                                         << (const char*)qname << endl; \
							   return false;
							   
bool StationListXmlHandler::startElement (const QString &/*ns*/, const QString &/*localname*/,
				                       const QString& qname, const QXmlAttributes &)
{
	if (qname == KRadioConfigElement) {
		if (m_status.size()) { START_ELEMENT_ERROR }

	// station list
	} else if (qname == StationListElement) {
		if (!m_status.size() || m_status.back() != KRadioConfigElement) { START_ELEMENT_ERROR }
		m_stations.clear();
		clearNewStation();

	} else if (qname == StationListInfo) {
		if (!m_status.size() || m_status.back() != StationListElement) { START_ELEMENT_ERROR }

	} else if (qname == StationListInfoMaintainer ||
               qname == StationListInfoCountry ||
               qname == StationListInfoCity ||
               qname == StationListInfoMedia ||
               qname == StationListInfoComments ||
               qname == StationListInfoChanged
			   )
	{
		if (!m_status.size() || m_status.back() != StationListInfo) { START_ELEMENT_ERROR }

	} else if (!m_newStation && m_status.size() && m_status.back() == StationListElement) {
	
		const RadioStation *x = RadioStation::getStationClass(qname);
		m_newStation = x ? x->copy() : NULL;

		if (!m_newStation) { START_ELEMENT_ERROR }
		
	} else if (m_newStation && m_status.size() && m_status.back() == m_newStation->getClassName()) {

		// check done later when characters arrive
	
	} else { // unknown
		kdDebug() << "StationListXmlHandler::startElement: "
		          << i18n("unknown element") << " "
		          << (const char*)qname << endl;
	}

	m_status.push_back(qname);
	return true;
}


bool StationListXmlHandler::endElement (const QString &/*ns*/, const QString &/*localname*/,
	                                 const QString &qname)
{
	if (m_status.size() && m_status.back() == qname) {
	
		if (m_newStation) {
			if (qname == m_newStation->getClassName()) {
				m_stations.append(m_newStation);
				clearNewStation();
			} else {
				kdDebug() << "StationListXmlHandler::endElement: "
				          << i18n("broken newStation pointer (internal error)") << endl;
				clearNewStation();
			}
		}

		m_status.pop_back();
		
	} else {
		if (m_status.size()) {
			kdDebug() << "StationListXmlHandler::endElement: " << i18n("expected element")
			          << " " << m_status.back() << ", " << "found " << qname << endl;
		} else {
			kdDebug() << "StationListXmlHandler::endElement: " << i18n("unexpected element")
			          << qname << endl;
		}
	}
	return true;
}


#define CHARACTERS_ERROR    kdDebug() << "StationListXmlHandler::characters: " \
                                      << i18n("invalid data for element") << " " \
                                      << (const char*)stat << endl; \
					        return false;
					        
bool StationListXmlHandler::characters (const QString &ch)
{
	QString stat = m_status.back();
	QString str = ch.stripWhiteSpace();

	// Station parsing

	// information on list
	if (stat == StationListInfo) {

	} else if (stat == StationListInfoMaintainer) {

		m_metaData.maintainer = str;

	} else if (stat == StationListInfoCountry) {

		m_metaData.country = str;

	} else if (stat == StationListInfoCity) {

		m_metaData.city = str;

	} else if (stat == StationListInfoMedia) {

		m_metaData.media = str;

	} else if (stat == StationListInfoComments) {

		m_metaData.comment = str;

	} else if (stat == StationListInfoChanged) {

		m_metaData.lastChange = QDateTime::fromString(str, Qt::ISODate);

    // stations
		
	} else if (m_newStation) {

		if (!m_newStation->setProperty(stat, str))
			return false;

	} else if (str.length()) {
		kdDebug() << "StationListXmlHandler::characters: "
		          << i18n("characters ignored for element") << " "
		          << (const char*)stat << endl;
	}
	return true;
}


void StationListXmlHandler::clearNewStation()
{
	if (m_newStation)
		delete m_newStation;
	m_newStation = NULL;
}
