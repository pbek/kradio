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

#include "../radio-stations/radiostation.h"
#include "../interfaces/errorlog-interfaces.h"
#include "utils.h"
#include "stationlist.h"
#include "stationlistxmlhandler.h"
#include "../kradioversion.h"

#include <qstring.h>
#include <qfile.h>
#include <qiodevice.h>
#include <qmessagebox.h>
#include <kio/netaccess.h>
#include <ktempfile.h>
#include <klocale.h>

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


bool RawStationList::insert (uint index, const RadioStation * item )
{
    if (!item) return false;
    RadioStation *rs = &stationWithID(item->stationID());
    bool r = true;
    if (rs != item) {
        r = BaseClass::insert(index, item);
        removeRef(rs);
    }
    return r;
}


bool RawStationList::insert (const RadioStation * item )
{
    if (!item) return false;
    int idx = idxWithID(item->stationID());
    if (idx >= 0) {
        return replace(idx, item);
    } else {
        append(item);
        return true;
    }
}


void RawStationList::inSort ( const RadioStation * item )
{
    if (!item) return;
    RadioStation *rs = &stationWithID(item->stationID());
    if (rs != item) {
        removeRef(rs);
        BaseClass::inSort(item);
    }
}


void RawStationList::prepend ( const RadioStation * item )
{
    if (!item) return;
    RadioStation *rs = &stationWithID(item->stationID());
    if (rs != item) {
        removeRef(rs);
        BaseClass::prepend(item);
    }
}


void RawStationList::append ( const RadioStation * item )
{
    if (!item) return;
    RadioStation *rs = &stationWithID(item->stationID());
    if (rs != item) {
        removeRef(rs);
        BaseClass::append(item);
    }
}


bool RawStationList::replace ( uint index, const RadioStation * item )
{
    bool r = true;
    RadioStation *rs = &stationWithID(item->stationID());
    if (rs != item) {
        BaseClass::removeRef(rs);
        r = BaseClass::insert(index, item);
    }
    return r;
}


const RadioStation &RawStationList::stationWithID(const QString &sid) const
{
    Iterator it(*this);
    for (; const RadioStation *s = it.current(); ++it) {
        if (s->stationID() == sid)
            return *s;
    }
    return (RadioStation &) undefinedRadioStation;
}


RadioStation &RawStationList::stationWithID(const QString &sid)
{
    Iterator it(*this);
    for (; RadioStation *s = it.current(); ++it) {
        if (s->stationID() == sid)
            return *s;
    }
    return (RadioStation &) undefinedRadioStation;
}



int RawStationList::idxWithID(const QString &sid) const
{
    int i = 0;
    Iterator it(*this);
    for (; const RadioStation *s = it.current(); ++it, ++i) {
        if (s->stationID() == sid)
            return i;
    }
    return -1;
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

    m_metaData.lastChange = QDateTime::currentDateTime();

    if (!metaData.maintainer.isEmpty())
        m_metaData.maintainer += (count() ? QString(" / ") : QString::null) + metaData.maintainer;

    if (!metaData.country.isEmpty())
        m_metaData.country += (count() ? QString(" / ") : QString::null) + metaData.country;

    if (!metaData.city.isEmpty())
        m_metaData.city = (count()     ? QString(" / ") : QString::null) + metaData.city;

    if (!metaData.media.isEmpty())
        m_metaData.media += (count()   ? QString(" / ") : QString::null) + metaData.media;

    if (!metaData.comment.isEmpty())
        m_metaData.comment += (count() ? QString(" / ") : QString::null) + metaData.comment;
    if (count() && other.count())
        m_metaData.comment += " " + i18n("Contains merged Data");


    // merge stations

    QPtrListIterator<RadioStation> it(other.all());
    for (RadioStation *s = it.current(); s; s = ++it) {
        m_all.insert(s);
    }
}



StationList &StationList::operator = (const StationList &other)
{
    m_metaData = other.metaData();
    m_all      = other.all();
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


const RadioStation &StationList::stationWithID(const QString &sid) const
{
    return m_all.stationWithID(sid);
}


RadioStation &StationList::stationWithID(const QString &sid)
{
    return m_all.stationWithID(sid);
}


bool StationList::readXML (const QString &dat, const IErrorLogClient &logger, bool enableMessageBox)
{
    // FIXME: TODO: error handling
    QXmlInputSource source;
    source.setData(dat);
    QXmlSimpleReader      reader;
    StationListXmlHandler handler(logger);
    reader.setContentHandler (&handler);
    if (reader.parse(source)) {
        if (handler.wasCompatMode() && enableMessageBox) {
            QMessageBox::information(NULL, "KRadio",
                                     i18n("Probably an old station preset file was read.\n"
                                     "You have to rebuild your station selections for "
                                     "the quickbar and the docking menu.")
                                     );
        }

        m_all      = handler.getStations();
        m_metaData = handler.getMetaData();
        return true;
    } else {
        logger.logError("StationList::readXML: " + i18n("parsing failed"));

        if (enableMessageBox) {
            QMessageBox::warning(NULL, "KRadio",
                                 i18n("Parsing the station preset file failed.\n"
                                      "See console output for more details."));
        }
        return false;
    }
}


bool StationList::readXML (const KURL &url, const IErrorLogClient &logger, bool enableMessageBox)
{
    QString tmpfile;
    if (!KIO::NetAccess::download(url, tmpfile, NULL)) {
        logger.logError("StationList::readXML: " +
                 i18n("error downloading preset file %1").arg(url.url()));
        if (enableMessageBox) {
            QMessageBox::warning(NULL, "KRadio",
                                 i18n("Download of the station preset file at %1 failed.")
                                 .arg(url.url()));
        }
        return false;
    }

    logger.logDebug("StationList::readXML: " +
             i18n("temporary file: ") + tmpfile);

    QFile presetFile (tmpfile);

    if (! presetFile.open(IO_ReadOnly)) {
        logger.logError("StationList::readXML: " +
                 i18n("error opening preset file %1").arg(tmpfile));
        if (enableMessageBox) {
            QMessageBox::warning(NULL, "KRadio",
                                 i18n("Opening of the station preset file at %1 failed.")
                                 .arg(tmpfile));
        }
        return false;
    }

    QString xmlData;

    // make sure that qtextstream is gone when we close presetFile
    QString tmp;
    {
        QTextStream ins(&presetFile);
        tmp = ins.read();
    }

    presetFile.reset();

    // preset file written with kradio <= 0.2.x
    if (tmp.find("<format>") < 0) {
        logger.logInfo(i18n("Old Preset File Format detected"));
        QTextStream ins(&presetFile);
        ins.setEncoding(QTextStream::Locale);
        xmlData = ins.read();
    }
    // preset file written with kradio >= 0.3.0
    else {
        QXmlInputSource tmp(&presetFile);
        xmlData = tmp.data();
    }

    presetFile.close();

    KIO::NetAccess::removeTempFile(tmpfile);

    return readXML(xmlData, logger, enableMessageBox);
}


QString StationList::writeXML (const IErrorLogClient &/*logger*/) const
{
    QString data = QString::null;

    // write station list

    QString t   = "\t";
    QString tt  = "\t\t";
    QString ttt = "\t\t\t";

    data +=       xmlOpenTag(KRadioConfigElement) +
            t   + xmlOpenTag(StationListElement) +
            tt  + xmlTag(StationListFormat, STATION_LIST_FORMAT) +
            tt  + xmlOpenTag(StationListInfo) +
            ttt + xmlTag(StationListInfoCreator,    "kradio-" KRADIO_VERSION) +
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
        QStringList::iterator end = properties.end();
        for (QStringList::iterator sit = properties.begin(); sit != end; ++sit) {
            data += ttt + xmlTag (*sit, s->getProperty(*sit));
        }
        data += tt + xmlCloseTag(s->getClassName());

    }

    data += t + xmlCloseTag(StationListElement) +
                xmlCloseTag(KRadioConfigElement);

    return data;
}


bool StationList::writeXML (const KURL &url, const IErrorLogClient &logger, bool enableMessageBox) const
{
    KTempFile tmpFile;
    tmpFile.setAutoDelete(true);
    QFile *outf = tmpFile.file();

    QTextStream outs(outf);
    outs.setEncoding(QTextStream::UnicodeUTF8);
    outs << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;

    QString output = writeXML(logger);

    outs << output;
    if (outf->status() != IO_Ok) {
        logger.logError("StationList::writeXML: " +
                 i18n("error writing to tempfile %1").arg(tmpFile.name()));
        if (enableMessageBox) {
            QMessageBox::warning(NULL, "KRadio",
                                 i18n("Writing station preset file %1 failed.")
                                 .arg(tmpFile.name()));
        }
        return false;
    }

    // close hopefully flushes buffers ;)
    outf->close();


    if (!KIO::NetAccess::upload(tmpFile.name(), url, NULL)) {
        logger.logError("StationList::writeXML: " +
                 i18n("error uploading preset file %1").arg(url.url()));

        if (enableMessageBox) {
            QMessageBox::warning(NULL, "KRadio",
                                 i18n("Upload of station preset file to %1 failed.")
                                 .arg(url.url()));
        }
        return false;
    }

    return true;
}
