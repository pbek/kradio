/***************************************************************************
                          radiocfgxmlhandler.h  -  description
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
#ifndef KRADIO_RADIOCFGXMLHANDLER_H
#define KRADIO_RADIOCFGXMLHANDLER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qxml.h>
#include <qobject.h>

#include <kradio/radio-stations/radiostation.h>
#include "stationlist.h"

class StationListXmlHandler : public QXmlDefaultHandler
{
protected:
    QStringList           m_status;
    const IErrorLogClient&m_logger;

    RawStationList        m_stations;
    StationListMetaData   m_metaData;

    RadioStation         *m_newStation;

    bool                  m_compatMode;

public :
    StationListXmlHandler (const IErrorLogClient &logger);
    virtual ~StationListXmlHandler ();
    bool startDocument ();
    bool startElement (const QString &ns, const QString &localname,
                       const QString& qname, const QXmlAttributes &);
    bool endElement   (const QString &ns, const QString &localname,
                       const QString &qname);
    bool characters   (const QString &ch);

    const RawStationList &getStations() const { return m_stations; }
    const StationListMetaData &getMetaData() const { return m_metaData; }


    bool  wasCompatMode() const { return m_compatMode; }

protected:

    void clearNewStation();
};



extern const char *KRadioConfigElement;
extern const char *StationListElement;

extern const char *StationListInfo;
extern const char *StationListInfoMaintainer;
extern const char *StationListInfoCountry;
extern const char *StationListInfoCity;
extern const char *StationListInfoMedia;
extern const char *StationListInfoComments;
extern const char *StationListInfoChanged;

extern const char *StationQuickSelectElement;
extern const char *StationDockingMenuElement;
extern const char *StationListFormat;

#define STATION_LIST_FORMAT   "kradio-1.0"


#endif
