/***************************************************************************
                          frequencyradiostation.cpp  -  description
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

#include "frequencyradiostation.h"
#include <typeinfo>

// Kopenhagener Wellenplan: 300kHz
#define STATION_FREQ_INTERVAL_FM   0.3

// Kopenhagener Wellenplan:   9kHz
#define STATION_FREQ_INTERVAL_AM   0.009

FrequencyRadioStation::FrequencyRadioStation()
    : RadioStation(),
      m_frequency(0)
{
}

FrequencyRadioStation::FrequencyRadioStation(float frequency)
    : RadioStation(),
      m_frequency(frequency)
{
}

FrequencyRadioStation::FrequencyRadioStation(const QString &name,
                                             const QString &shortName,
                                             float frequency)
    : RadioStation(name, shortName),
      m_frequency(frequency)
{
}

FrequencyRadioStation::FrequencyRadioStation(FrequencyRadioStation const &s)
    : RadioStation(s),
      m_frequency(s.m_frequency)
{
}


/** returns an exact copy of this station */
RadioStation *FrequencyRadioStation::copy() const
{
    return new FrequencyRadioStation(*this);
}


FrequencyRadioStation::~FrequencyRadioStation()
{
}


/*  = 0 : "this" is same as "s", i.e. approximately same frequency
    > 0 : this.frequency > s.frequency
    < 0 : this.frequency < s.frequency
    other class than FrequencyRadioStation: compare typeid(.).name()
*/
int FrequencyRadioStation::compare(const RadioStation &_s) const
{
	FrequencyRadioStation const *s = dynamic_cast<FrequencyRadioStation const*>(&_s);

	if (!s)
		return (typeid(this).name() > typeid(&_s).name()) ? 1 : -1;			
	
    float delta = m_frequency < 10 ? STATION_FREQ_INTERVAL_AM : STATION_FREQ_INTERVAL_FM;
    
    if (   m_frequency + delta/2 > s->m_frequency
        && m_frequency - delta/2 < s->m_frequency)
    {    
		return 0;
	} else {
		return (m_frequency > s->m_frequency) ? 1 : -1;
	}	
}


QString FrequencyRadioStation::longName() const
{
    QString longN = name();
    if (!longN.isEmpty())
        longN += ", ";

    float   cf = frequency();
    QString f;
    if (cf >= 10)
        f = QString().setNum(cf, 'f', 2) + " MHz";
    else
        f = QString().setNum(cf * 1000, 'f', 0) + " kHz";

    longN = longN + f;
    return longN;
}


bool FrequencyRadioStation::isValid() const
{
	return m_frequency > 0;
}
