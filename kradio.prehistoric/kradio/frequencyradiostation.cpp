/***************************************************************************
                          frequencyradiostation.cpp  -  description
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

#include "frequencyradiostation.h"

#include "frequencyradio.h"

// Kopenhagener Wellenplan: 300kHz
#define STATION_FREQ_INTERVAL_FM   0.3

// Kopenhagener Wellenplan:   9kHz
#define STATION_FREQ_INTERVAL_AM   0.009

// KDE includes
#include <kdebug.h>

FrequencyRadioStation::FrequencyRadioStation(QObject *parent, QString name, FrequencyRadio* radio, float frequency)
    : RadioStation(parent, name),
      m_frequencyRadio(radio),
      m_frequency(frequency)
{
}

FrequencyRadioStation::FrequencyRadioStation(QObject *parent, FrequencyRadioStation const &s)
    : RadioStation(parent, s),
      m_frequencyRadio(s.m_frequencyRadio),
      m_frequency(s.m_frequency)
{
}

FrequencyRadioStation::FrequencyRadioStation(FrequencyRadioStation const &s)
    : RadioStation(s),
      m_frequencyRadio(s.m_frequencyRadio),
      m_frequency(s.m_frequency)
{
}

/** returns an exact copy of this station, but the parent is the one given */
RadioStation *FrequencyRadioStation::copy(QObject *parent)
{
    return new FrequencyRadioStation(parent, (*this));
}

FrequencyRadioStation::~FrequencyRadioStation()
{
}

// implementation of the RadioStation pure virtuals
RadioDevice *FrequencyRadioStation::radio()
{
    return m_frequencyRadio;
}

bool FrequencyRadioStation::frequencyMatch(float frequency)
{
       float delta = f < 10 ? STATION_FREQ_INTERVAL_AM : STATION_FREQ_INTERVAL_FM;
       return m_frequency + delta/2 > frequency
           && m_frequency - delta/2 < frequency;
}

void FrequencyRadioStation::slotActivate()
{
    if (!isValid())
        kdDebug()<< "FrequencyRadioStation::slotActivate cannot activate invalid frequency "<<m_frequency<<"!" << endl;
    if (m_frequencyRadio) {
        if (m_frequencyRadio->activateStation(this))
            emit signalActivated(this);
    } else
        kdDebug()<< "FrequencyRadioStation::slotActivate called but there is no Radio device for this Station!" << endl;
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
