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

#include "internetradio.h"

// KDE includes
#include <kdebug.h>

InternetRadioStation::InternetRadioStation(QObject *parent, QString name,
                                           InternetRadio* radio, KURL const &url)
    : RadioStation(parent, name),
      m_url(url),
      m_internetRadio(radio)
{
}

InternetRadioStation::InternetRadioStation(QObject *parent, InternetRadioStation const &s)
    : RadioStation(parent,s),
      m_url(s.m_url),
      m_internetRadio(s.m_internetRadio)
{
}

InternetRadioStation::InternetRadioStation(InternetRadioStation const &s)
    : RadioStation(s),
      m_url(s.m_url),
      m_internetRadio(s.m_internetRadio)
{
}

/** returns an exact copy of this station, but the parent is the one given */
RadioStation *InternetRadioStation::copy(QObject *parent)
{
    return new InternetRadioStation(parent, (*this));
}

InternetRadioStation::~InternetRadioStation()
{
}

// implementation of the RadioStation pure virtuals
RadioDevice *InternetRadioStation::radio()
{
    return m_internetRadio;
}

bool InternetRadioStation::urlMatch(KURL const &url)
{
    return m_url.equals(url,true /*ignore trailing / */);
}

void InternetRadioStation::slotActivate()
{
    if (!isValid())
        kdDebug()<< "InternetRadioStation::slotActivate cannot activate invalid URL "<<m_url.url()<<"!" << endl;
    if (m_internetRadio) {
        if (m_internetRadio->activateStation(this))
            emit signalActivated(this);
    } else
        kdDebug()<< "InternetRadioStation::slotActivate called but there is no Radio device for this Station!" << endl;
}

bool InternetRadioStation::isValid() const
{
    // TODO: maybe we need to do more to validate this...
    return !m_url.isEmpty();
}
