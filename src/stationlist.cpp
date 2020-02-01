/***************************************************************************
                          stationlist.cpp  -  description
                             -------------------
    begin                : Sat March 29 2003
    copyright            : (C) 2003 by Klas Kalass, Ernst Martin Witte
    email                : klas@kde.org, emw-kradio@nocabal.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "radiostation.h"
#include "errorlog_interfaces.h"
#include "stationlist.h"
#include "stationlistxmlhandler.h"

#include <QString>
#include <QFile>
#include <QIODevice>
#include <QMessageBox>
#include <QTextCodec>
#include <QXmlStreamWriter>
#include <QTemporaryFile>
#include <klocalizedstring.h>

#include "kio_get_wrapper.h"
#include "kio_put_wrapper.h"

//////////////////////////////////////////////////////////////////////////////

const StationList emptyStationList;

//////////////////////////////////////////////////////////////////////////////

StationList::StationList()
{
}

StationList::StationList(const StationList &sl)
    : m_metaData (sl.m_metaData)
{
    setStations(sl);
}


StationList::~StationList()
{
    clearStations();
}


StationList &StationList::clearStations()
{
    qDeleteAll(m_stations);
    m_stations.clear();
    return *this;
}


StationList &StationList::setStations(const StationList &x)
{
    if (&x != this) {
        clearStations();
        addStations(x);
    }
    return *this;
}

StationList &StationList::addStations(const StationList &x)
{
    const_iterator it = x.begin();
    for (; it != x.end(); ++it) {
        m_stations.append((*it)->copy());
    }
    return *this;
}

StationList &StationList::addStation(const RadioStation &x)
{
    m_stations.append(x.copy());
    return *this;
}

void StationList::moveStation(int old_idx, int new_idx)
{
    if (old_idx >= 0 && old_idx < m_stations.size()) {
        if (new_idx >= m_stations.size()) {
            new_idx = m_stations.size() - 1;
        }
        m_stations.move(old_idx, new_idx);
    }
}

StationList &StationList::removeStationAt(int idx)
{
    if (idx >= 0 && idx < m_stations.size()) {
        RadioStation *s = m_stations.at(idx);
        m_stations.removeAt(idx);
        delete s;
    }
    return *this;
}

void StationList::merge(const StationList & other)
{
    // merge meta information: honor merge in comment

    StationListMetaData const & metaData = other.metaData();

    if (! m_metaData.comment.isEmpty())
        m_metaData.comment += "\n";

    m_metaData.lastChange = QDateTime::currentDateTime();

    if (!metaData.maintainer.isEmpty())
        m_metaData.maintainer += (count() ? QString(" / ") : QString()) + metaData.maintainer;

    if (!metaData.country.isEmpty())
        m_metaData.country += (count() ? QString(" / ") : QString()) + metaData.country;

    if (!metaData.city.isEmpty())
        m_metaData.city = (count()     ? QString(" / ") : QString()) + metaData.city;

    if (!metaData.media.isEmpty())
        m_metaData.media += (count()   ? QString(" / ") : QString()) + metaData.media;

    if (!metaData.comment.isEmpty())
        m_metaData.comment += (count() ? QString(" / ") : QString()) + metaData.comment;
    if (count() && other.count())
        m_metaData.comment += " " + i18n("Contains merged Data");


    // merge stations
    addStations(other);
}



StationList &StationList::operator = (const StationList &other)
{
    m_metaData = other.metaData();
    setStations(other);
    return *this;
}


const RadioStation &StationList::at(int idx) const
{
    return idx >= 0 && idx < count() ? *m_stations[idx] : (const RadioStation &) undefinedRadioStation;
}


RadioStation &StationList::at(int idx)
{
    return idx >= 0 && idx < count() ? *m_stations[idx] : (RadioStation &) undefinedRadioStation;
}




bool StationList::readXML (const QXmlInputSource &xmlInp, const IErrorLogClient &logger, bool enableMessageBox)
{
    // FIXME: TODO: error handling

    QXmlSimpleReader      reader;
    StationListXmlHandler handler(logger);
    reader.setContentHandler (&handler);
    if (reader.parse(xmlInp)) {
        if (handler.wasCompatMode() && enableMessageBox) {
            QMessageBox::information(NULL, "KRadio",
                                     i18n("Probably an old station preset file was read.\n"
                                     "You have to rebuild your station selections for "
                                     "the quickbar and the docking menu.")
                                     );
        }

        *this = handler.getStations();
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


bool StationList::readXML (const QUrl &url, const IErrorLogClient &logger, bool enableMessageBox)
{
    kio_get_wrapper_t  rxJob(url, 2ull * 1024ull * 1024ull);
    rxJob.exec();
    
    if (!rxJob.ok()) {
        if (enableMessageBox) {
            logger.logError("StationList::readXML: " +
                            i18n("error downloading preset file %1: %2", url.toString(), rxJob.errorString()));
            QMessageBox::warning(NULL, "KRadio",
                                 i18n("Download of the station preset file at %1 failed: %2", url.toString(), rxJob.errorString()));
        } else {
            logger.logWarning("StationList::readXML: " +
                              i18n("error downloading preset file %1: %2", url.toString(), rxJob.errorString()));
        }
        return false;
    }

    bool retval = false;
    
    const QString xmlData = QString::fromLocal8Bit(rxJob.data());

    // preset file written with kradio <= 0.2.x
    if (xmlData.indexOf("<format>") < 0) {
        logger.logInfo(i18n("Old Preset File Format detected"));
        QXmlInputSource xmlInput;
        xmlInput.setData(xmlData);
        retval = readXML(xmlInput, logger, enableMessageBox);
    }
    // preset file written with kradio >= 0.3.0
    else {
        QTextStream     txtStream(rxJob.data());
        QXmlInputSource xmlInput (txtStream.device());
        retval = readXML(xmlInput, logger, enableMessageBox);
    }

    return retval;
}


QString StationList::writeXML (const IErrorLogClient &/*logger*/) const
{
    QString data;

    {
    QXmlStreamWriter writer(&data);
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(-1);
    writer.setCodec("UTF-8");

    writer.writeStartDocument();
    writer.writeStartElement(KRadioConfigElement);
    writer.writeStartElement(StationListElement);

    writer.writeTextElement(StationListFormat, STATION_LIST_FORMAT);
    writer.writeStartElement(StationListInfo);
    writer.writeTextElement(StationListInfoCreator, "kradio-" KRADIO_VERSION);
    writer.writeTextElement(StationListInfoMaintainer, m_metaData.maintainer);
    writer.writeTextElement(StationListInfoChanged, m_metaData.lastChange.toString(Qt::ISODate));
    writer.writeTextElement(StationListInfoCountry, m_metaData.country);
    writer.writeTextElement(StationListInfoCity, m_metaData.city);
    writer.writeTextElement(StationListInfoMedia, m_metaData.media);
    writer.writeTextElement(StationListInfoComments, m_metaData.comment);
    writer.writeEndElement();

    QHash<QString, QStringList> propertyNames;

    for (const_iterator it = begin(); it != end(); ++it) {
        RadioStation *s = *it;
        const QString className = s->getClassName();

        writer.writeStartElement(className);

        QHash<QString, QStringList>::iterator propIt = propertyNames.find(className);
        if (propIt == propertyNames.end()) {
            propIt = propertyNames.insert(className, s->getPropertyNames());
        }
        foreach (const QString &prop, propIt.value()) {
            writer.writeTextElement(prop, s->getProperty(prop));
        }
        writer.writeEndElement();

    }

    writer.writeEndElement();
    writer.writeEndElement();
    }

    return data + QLatin1Char('\n');
}


bool StationList::writeXML (const QUrl &url, const IErrorLogClient &logger, bool enableMessageBox) const
{
    QTemporaryFile tmpFile;
    tmpFile.setAutoRemove(true);
    if (tmpFile.open()) {
        QString tmpFileName = tmpFile.fileName();
//         logger.logDebug("tempfile: " + tmpFileName);

        QTextStream outs(&tmpFile);
        outs.setCodec(QTextCodec::QTextCodec::codecForName("UTF-8"));

        QString output = writeXML(logger);

        outs << output;
        if (tmpFile.error()) {
            logger.logError("StationList::writeXML: " +
                            i18n("error writing to temporary file %1", tmpFileName));
            if (enableMessageBox) {
                QMessageBox::warning(NULL, "KRadio",
                                     i18n("Writing station preset file %1 failed.", tmpFileName));
            }
            return false;
        }

        // close hopefully flushes buffers ;)
        tmpFile.close();

        if (count() < 1) {
            logger.logWarning("StationList::writeXML: " +
                              i18n("uploading preset file %1: ", url.toString()));
            logger.logWarning("StationList::writeXML: " +
                              i18n("something strange happened, station list has only %1 entries. Writing station preset file skipped", count()));
        } else {
            tmpFile.open();            
            
            const QByteArray tmpFileData = tmpFile.readAll();
            tmpFile.close();
            
            kio_put_wrapper_t kio_put_wrapper(url, tmpFileData, KIO::Overwrite | KIO::HideProgressInfo | KIO::NoPrivilegeExecution);
            
            kio_put_wrapper.exec();
            
            if (!kio_put_wrapper.ok()) {
                logger.logError("StationList::writeXML: " +
                                i18n("error uploading preset file %1: %2", url.toString(), kio_put_wrapper.errorString()));

                if (enableMessageBox) {
                    QMessageBox::warning(NULL, "KRadio",
                                         i18n("Upload of station preset file to %1 failed: %2", url.toString(), kio_put_wrapper.errorString()));
                }
                return false;
            }
        }
        return true;
    } else {
        logger.logError("StationList::writeXML: " + i18n("error creating temporary file"));
        QMessageBox::warning(NULL, "KRadio", i18n("error creating temporary file"));
        return false;
    }
    
    return false;
} // StationList::writeXML


bool StationList::operator == (const StationList &x) const
{
    if (count() != x.count())
        return false;

    if (m_metaData != x.m_metaData)
        return false;

    QList<RadioStation*>::const_iterator it1 = m_stations.begin();
    QList<RadioStation*>::const_iterator it2 = x.m_stations.begin();
    for ( ; it1 != m_stations.end() && it2 != x.m_stations.end(); ++it1, ++it2) {
        if (**it1 != **it2)
            return false;
    }
    return true;
}



const RadioStation &StationList::stationWithID(const QString &sid) const
{
    foreach (RadioStation *rs, m_stations) {
        if (rs->stationID() == sid)
            return *rs;
    }
    return (const RadioStation &) undefinedRadioStation;
}


RadioStation &StationList::stationWithID(const QString &sid)
{
    foreach (RadioStation *rs, m_stations) {
        if (rs->stationID() == sid)
            return *rs;
    }
    return (RadioStation &) undefinedRadioStation;
}


int StationList::idxWithID(const QString &sid) const
{
    int i = 0;
    for (const_iterator it = m_stations.begin(); it != m_stations.end(); ++it, ++i) {
        if ((*it)->stationID() == sid)
            return i;
    }
    return -1;
}


