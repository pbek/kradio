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

extern const char *StationNameElement;
extern const char *StationShortNameElement;
extern const char *StationIconStringElement;
extern const char *StationVolumePresetElement;

const char *StationNameElement			= "name";
const char *StationShortNameElement		= "shortname";
const char *StationIconStringElement	= "icon";
const char *StationVolumePresetElement	= "volumepreset";

/////////////////////////////////////////////////////////////////////////////

QDict<RadioStation> *RadioStation::stationClassRegistry = 0;

/////////////////////////////////////////////////////////////////////////////

RegisterStationClass registerStationClass;
const UndefinedRadioStation undefinedRadioStation (registerStationClass);

/////////////////////////////////////////////////////////////////////////////


RadioStation::RadioStation(RegisterStationClass, const QString &classname)
	: m_name(""),
      m_shortName(""),
      m_initialVolume(-1),
      m_iconName("")
{
	if (! stationClassRegistry)
		stationClassRegistry = new QDict<RadioStation>;
	stationClassRegistry->insert(classname, this);
}

RadioStation::RadioStation()
    : m_name(""),
      m_shortName(""),
      m_initialVolume(-1),
      m_iconName("")
{
}

RadioStation::RadioStation(const QString &name, const QString &shortName)
    : m_name(name),
      m_shortName(shortName),
      m_initialVolume(-1),
      m_iconName("")
{
}


RadioStation::RadioStation(const RadioStation &s)
    : m_name(s.m_name),
      m_shortName(s.m_shortName),
      m_initialVolume(s.m_initialVolume),
      m_iconName(s.m_iconName)
{
}


RadioStation::~RadioStation()
{
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
	if (pn == StationNameElement) {
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
	if (pn == StationNameElement) {
		return m_name;
	} else if (pn == StationShortNameElement) {
		return m_shortName;
	} else if (pn == StationIconStringElement) {
		return m_iconName;
	} else if (pn == StationVolumePresetElement) {
		return QString().setNum(m_initialVolume);
	} else {
		return "";
	}
}


QStringList RadioStation::getPropertyNames() const
{
	QStringList l;
	l.push_back(StationNameElement);
	l.push_back(StationShortNameElement);
	l.push_back(StationIconStringElement);
	l.push_back(StationVolumePresetElement);
	return l;
}

/////////////////////////////////////////////////////////////////////////

int UndefinedRadioStation::compare(const RadioStation &_s) const
{
	UndefinedRadioStation const *s = dynamic_cast<UndefinedRadioStation const*>(&_s);

	if (!s)
		return -1;

	return 0;

}
