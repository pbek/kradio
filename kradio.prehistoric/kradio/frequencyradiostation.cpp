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

// Kopenhagener Wellenplan: 300kHz
#define STATION_FREQ_INTERVAL_FM   0.3

// Kopenhagener Wellenplan:   9kHz
#define STATION_FREQ_INTERVAL_AM   0.009

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


bool FrequencyRadioStation::equals(const RadioStation &_s) const
{
	FrequencyRadioStation const *s = dynamic_cast<FrequencyRadioStation const*>(&_s);

	if (!s)
	    return false;
	
    float delta = m_frequency < 10 ? STATION_FREQ_INTERVAL_AM : STATION_FREQ_INTERVAL_FM;
    
    return    m_frequency + delta/2 > s->m_frequency
           && m_frequency - delta/2 < s->m_frequency;
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
