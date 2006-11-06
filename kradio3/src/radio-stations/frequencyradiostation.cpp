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
#include "radiostation-config.h"

/////////////////////////////////////////////////////////////////////////////

const char *StationFrequencyElement        = "frequency";

static FrequencyRadioStation emptyFrequencyRadioStation(registerStationClass);

/////////////////////////////////////////////////////////////////////////////

FrequencyRadioStation::FrequencyRadioStation (RegisterStationClass, const QString &classname)
    : RadioStation(registerStationClass, !classname.isNull() ? classname : getClassName()),
      m_frequency(0)
{
}

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

RadioStation *FrequencyRadioStation::copyNewID() const
{
    RadioStation *x = new FrequencyRadioStation(*this);
    x->generateNewStationID();
    return x;
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

    if (!s) {
        return (typeid(this).name() > typeid(&_s).name()) ? 1 : -1;
    }

    // stations with no valid frequency are never identical
    if (m_frequency == 0)
        return -1;
    if (s->m_frequency == 0)
        return 1;

    float delta = (m_frequency < 10) ? STATION_FREQ_INTERVAL_AM : STATION_FREQ_INTERVAL_FM;

    if (   m_frequency + delta/4 > s->m_frequency
        && m_frequency - delta/4 < s->m_frequency)
    {
        return 0;
    } else {
        return (m_frequency > s->m_frequency) ? 1 : -1;
    }
}


QString FrequencyRadioStation::longName() const
{
    QString longN = name();
    if (!longN.isEmpty()) {
        longN = i18n("%1, %2").arg(longN).arg(description());
    } else {
        longN = description();
    }

    return longN;
}


QString FrequencyRadioStation::description() const
{
    float   cf = frequency();
    QString f;
    if (cf >= 10)
        f = i18n("%1 MHz").arg(KGlobal::locale()->formatNumber(cf, 2));
    else
        f = i18n("%1 kHz").arg(KGlobal::locale()->formatNumber(cf * 1000, 0));

    return f;
}


bool FrequencyRadioStation::isValid() const
{
    return m_frequency > 0;
}



bool FrequencyRadioStation::setProperty(const QString &pn, const QString &val)
{
    bool retval = false;
    if (pn == StationFrequencyElement) {
        float f = val.toFloat(&retval);
        if (retval)
            m_frequency = f;
    } else {
        retval = RadioStation::setProperty(pn, val);
    }
    return retval;
}


QString FrequencyRadioStation::getProperty(const QString &pn) const
{
    if (pn == StationFrequencyElement) {
        return QString().setNum(m_frequency);
    } else {
        return RadioStation::getProperty(pn);
    }
}


QStringList FrequencyRadioStation::getPropertyNames() const
{
    QStringList l = RadioStation::getPropertyNames();
    l.push_back(StationFrequencyElement);
    return l;
}


RadioStationConfig *FrequencyRadioStation::createEditor() const
{
    return new FrequencyRadioStationConfig(NULL);
}

bool FrequencyRadioStation::operator == (const RadioStation &x) const
{
    if (!RadioStation::operator == (x))
        return false;

    FrequencyRadioStation const *fx = dynamic_cast<FrequencyRadioStation const*>(&x);
    if (!fx)
        return false;

    float delta = (m_frequency < 10) ? STATION_FREQ_INTERVAL_AM : STATION_FREQ_INTERVAL_FM;

    return m_frequency + delta/4 > fx->m_frequency
        && m_frequency - delta/4 < fx->m_frequency;

}
