/***************************************************************************
                          radiostation.cpp  -  description
                             -------------------
    begin                : Sat Feb 2 2002
    copyright            : (C) 2002 by Martin Witte / Frank Schwanz
    email                : witte@kawo1.rwth-aachen.de / schwanz@fh-brandenburg.de
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
#include "radiostation-config.h"

#include <unistd.h>
#include <time.h>
#include <fcntl.h>

extern const char *StationNameElement;
extern const char *StationShortNameElement;
extern const char *StationIconStringElement;
extern const char *StationVolumePresetElement;
extern const char *StationIDElement;

const char *StationNameElement          = "name";
const char *StationShortNameElement     = "shortname";
const char *StationIconStringElement    = "icon";
const char *StationVolumePresetElement  = "volumepreset";
const char *StationIDElement            = "stationID";

const char *dev_urandom_radiostations = "/dev/urandom";

/////////////////////////////////////////////////////////////////////////////

QDict<RadioStation> *RadioStation::stationClassRegistry = 0;

/////////////////////////////////////////////////////////////////////////////

RegisterStationClass registerStationClass;
const UndefinedRadioStation undefinedRadioStation (registerStationClass);

/////////////////////////////////////////////////////////////////////////////


RadioStation::RadioStation(RegisterStationClass, const QString &classname)
    : m_stationID(QString::null),  // mark this station as a prototype station
                                   // so that we can create later a real stationID
      m_name(QString::null),
      m_shortName(QString::null),
      m_initialVolume(-1),
      m_iconName(QString::null)
{
    if (! stationClassRegistry)
        stationClassRegistry = new QDict<RadioStation>;
    stationClassRegistry->insert(classname, this);
}

RadioStation::RadioStation()
    : m_name(QString::null),
      m_shortName(QString::null),
      m_initialVolume(-1),
      m_iconName(QString::null)
{
    generateNewStationID();
}

RadioStation::RadioStation(const QString &name, const QString &shortName)
    : m_name(name),
      m_shortName(shortName),
      m_initialVolume(-1),
      m_iconName(QString::null)
{
    generateNewStationID();
}


RadioStation::RadioStation(const RadioStation &s)
    : m_stationID(s.m_stationID),
      m_name(s.m_name),
      m_shortName(s.m_shortName),
      m_initialVolume(s.m_initialVolume),
      m_iconName(s.m_iconName)
{
    // create a real stationID if "s" is a prototype
    if (m_stationID.isNull())
        generateNewStationID();
}


RadioStation::~RadioStation()
{
}


void  RadioStation::copyDescriptionFrom(const RadioStation &rs)
{
    m_name      = rs.m_name;
    m_shortName = rs.m_shortName;
    m_iconName  = rs.m_iconName;
    m_stationID = rs.m_stationID;
}


void RadioStation::generateNewStationID()
{
    const int buffersize = 32;
    unsigned char buffer[buffersize];

    QString stime, srandom = QString::null;
    stime.setNum(time(NULL));

    int fd = open (dev_urandom_radiostations, O_RDONLY);
    read(fd, buffer, buffersize);
    close(fd);
    for (int i = 0; i < buffersize; ++i)
        srandom += QString().sprintf("%02X", (unsigned int)buffer[i]);

    m_stationID = stime + srandom;
}


RadioStation const *RadioStation::getStationClass(const QString &classname)
{
    if (stationClassRegistry)
        return stationClassRegistry->find(classname);
    else
        return NULL;
}


bool RadioStation::setProperty(const QString &pn, const QString &val)
{
    bool retval = false;
    if (pn == StationIDElement) {
        m_stationID = val;
        retval = true;
    } else if (pn == StationNameElement) {
        m_name = val;
        retval = true;
    } else if (pn == StationShortNameElement) {
        m_shortName = val;
        retval = true;
    } else if (pn == StationIconStringElement) {
        m_iconName = val;
        retval = true;
    } else if (pn == StationVolumePresetElement) {
        float x = val.toFloat(&retval);
        if (retval)
            m_initialVolume = x;
    }
    return retval;
}


QString RadioStation::getProperty(const QString &pn) const
{
    if (pn == StationIDElement) {
        return m_stationID;
    } else if (pn == StationNameElement) {
        return m_name;
    } else if (pn == StationShortNameElement) {
        return m_shortName;
    } else if (pn == StationIconStringElement) {
        return m_iconName;
    } else if (pn == StationVolumePresetElement) {
        return QString().setNum(m_initialVolume);
    } else {
        return QString::null;
    }
}


QStringList RadioStation::getPropertyNames() const
{
    QStringList l;
    l.push_back(StationIDElement);
    l.push_back(StationNameElement);
    l.push_back(StationShortNameElement);
    l.push_back(StationIconStringElement);
    l.push_back(StationVolumePresetElement);
    return l;
}

bool RadioStation::operator == (const RadioStation &x) const
{
    return m_stationID     == x.m_stationID &&
           m_name          == x.m_name      &&
           m_shortName     == x.m_shortName &&
           m_initialVolume == x.m_initialVolume &&
           m_iconName      == x.m_iconName;
}

/////////////////////////////////////////////////////////////////////////

int UndefinedRadioStation::compare(const RadioStation &_s) const
{
    UndefinedRadioStation const *s = dynamic_cast<UndefinedRadioStation const*>(&_s);

    if (!s)
        return -1;

    return 0;

}


RadioStationConfig *UndefinedRadioStation::createEditor() const
{
    return new UndefinedRadioStationConfig(NULL);
}
