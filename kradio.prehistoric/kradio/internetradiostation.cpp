/***************************************************************************
                          internetradiostation.cpp  -  description
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

#include "internetradiostation.h"
#include <typeinfo>

/////////////////////////////////////////////////////////////////////////////

const char *StationUrlElement = "url";

static InternetRadioStation  emptyInternetRadioStation(registerStationClass);

/////////////////////////////////////////////////////////////////////////////

InternetRadioStation::InternetRadioStation()
    : RadioStation(),
      m_url()
{
}

InternetRadioStation::InternetRadioStation(const KURL &url)
    : RadioStation(),
      m_url(url)
{
}

InternetRadioStation::InternetRadioStation(const QString &name,
                                           const QString &shortName,
                                           const KURL &url)
    : RadioStation(name, shortName),
      m_url(url)
{
}

InternetRadioStation::InternetRadioStation(const InternetRadioStation &s)
    : RadioStation(s),
      m_url(s.m_url)
{
}


InternetRadioStation::InternetRadioStation(RegisterStationClass, const QString &classname)
    : RadioStation(registerStationClass, !classname.isNull() ? classname : getClassName()),
      m_url()
{
}


/** returns an exact copy of this station*/
RadioStation *InternetRadioStation::copy() const
{
    return new InternetRadioStation(*this);
}

InternetRadioStation::~InternetRadioStation()
{
}


/*  = 0 : this.url = s.url
    > 0 : this.url > s.url
    < 0 : this.url < s.url
    other class than FrequencyRadioStation: compare typeid(.).name()
*/
int InternetRadioStation::compare(const RadioStation &_s) const
{
	InternetRadioStation const *s = dynamic_cast<InternetRadioStation const*>(&_s);

	if (!s)
		return (typeid(this).name() > typeid(&_s).name()) ? 1 : -1;

	QString thisurl = m_url.url(-1);    // -1: remove trailing '/'
	QString thaturl = s->m_url.url(-1);

	// empty urls are never identical
	if (thisurl.length () == 0)
		return -1;
	if (thaturl.length() == 0)
		return 1;

	return thisurl.compare(thaturl);
}



bool InternetRadioStation::isValid() const
{
    // TODO: maybe we need to do more to validate this...
    return !m_url.isEmpty();
}

QString InternetRadioStation::longName() const
{
	QString longN = m_name;
	if ( ! longN.isEmpty() )
		longN = longN + ", ";

	return longN + m_url.url();
}


bool InternetRadioStation::setProperty(const QString &pn, const QString &val)
{
	bool retval = false;
	if (pn == StationUrlElement) {
		m_url = val;
		retval = true;
	} else {
		retval = RadioStation::setProperty(pn, val);
	}
	return retval;
}

QString InternetRadioStation::getProperty(const QString &pn) const
{
	if (pn == StationUrlElement) {
		return m_url.url();
	} else {
		return RadioStation::getProperty(pn);
	}
}

QStringList InternetRadioStation::getPropertyNames() const
{
	QStringList l = RadioStation::getPropertyNames();
	l.push_back(StationUrlElement);
	return l;
}
