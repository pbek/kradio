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


/** returns an exact copy of this station*/
RadioStation *InternetRadioStation::copy() const
{
    return new InternetRadioStation(*this);
}

InternetRadioStation::~InternetRadioStation()
{
}

bool InternetRadioStation::equals(const RadioStation &_s) const
{
	InternetRadioStation const *s = dynamic_cast<InternetRadioStation const*>(&_s);

	if (!s)
		return false;

    return m_url.equals ( s->m_url, true /*ignore trailing / */ );
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
