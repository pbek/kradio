/***************************************************************************
                          radio.cpp  -  description
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

#include "radio.h"
#include "radiostation.h"
#include "frequencyradiostation.h"
#include "internetradiostation.h"
#include "artsstreamradio.h"
#include "v4lradio.h"

Radio::Radio()
    : m_currentStation(0),
      m_frequencyRadio(0),
      m_artsStreamRadio(0)
{
}

Radio::~Radio()
{
}

void Radio::slotCurrentStationChanged(RadioStation *station)
{
    m_currentStation = station;
    emit signalStationChanged(station);
}

RadioStation *Radio::createFrequencyRadioStation(QString const &name, float frequency)
{
    if (!m_frequencyRadio) {
        // no instance of an ArtsStreamRadio yet, create one
        m_frequencyRadio = new V4LRadio(this);
    }

    FrequencyRadioStation *station = new FrequencyRadioStation(this, name, m_frequencyRadio,frequency);
    registerStation(station);
    return station;
}

RadioStation *Radio::createInternetRadioStation(QString const &name, KURL const &url)
{
    // TODO: when there are other internet stream implementations,
    // choose the right one here!
    if (!m_artsStreamRadio) {
        // no instance of an ArtsStreamRadio yet, create one
        m_artsStreamRadio = new ArtsStreamRadio(this);
    }

    InternetRadioStation * station = new InternetRadioStation(this, name, m_artsStreamRadio, url);
    registerStation(station);
    return station;
}

bool Radio::power()
{
    return (m_currentStation && m_currentStation->radio())
        ? m_currentStation->radio()->power()
        : false;
}

void Radio::slotPowerOn()
{
    if (m_currentStation && m_currentStation->radio())
        m_currentStation->radio()->setPower(true);
    emit signalPowerChanged(power());
}

void Radio::slotPowerOff()
{
    if (m_currentStation && m_currentStation->radio())
        m_currentStation->radio()->setPower(false);
    emit signalPowerChanged(power());
}

void Radio::slotPowerToggle()
{
    if (m_currentStation && m_currentStation->radio())
        m_currentStation->radio()->setPower(!m_currentStation->radio()->power());
    emit signalPowerChanged(power());
}

bool Radio::muted()
{
    return (m_currentStation && m_currentStation->radio())
        ? m_currentStation->radio()->muted()
        : false;
}

void Radio::slotMute()
{
    if (m_currentStation && m_currentStation->radio())
        m_currentStation->radio()->setMute(true);
}

void Radio::slotUnmute()
{
    if (m_currentStation && m_currentStation->radio())
        m_currentStation->radio()->setMute(false);
}

void Radio::registerStation(RadioStation * station)
{
    connect(station, SIGNAL(signalActivated(const RadioStation *)),
            this, SLOT(slotCurrentStationChanged(const RadioStation *)));
}
