/***************************************************************************
                          internetradiostation.h  -  description
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

#ifndef INTERNETRADIOSTATION_H
#define INTERNETRADIOSTATION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "radiostation.h"

// KDE includes
#include <kurl.h>

// forward declarations
class InternetRadio;
class RadioDevice;

/**
 * @author Klas Kalass
 */
class InternetRadioStation : public RadioStation  {
public:
    InternetRadioStation(QObject *parent, QString name, InternetRadio* radio, KURL const &url);
    InternetRadioStation(InternetRadioStation const&);
    InternetRadioStation(QObject *parent, InternetRadioStation const&);
    ~InternetRadioStation();

    KURL const & url() {return m_url;};
    void setUrl(KURL const &url) {m_url = url;};

    // implementation of the RadioStation interface
    RadioDevice *radio();

    bool urlMatch(KURL const &url);
    virtual bool isValid() const;

    /** returns an exact copy of this station, but the parent is the one given */
    virtual RadioStation *copy(QObject *parent);

public slots:
    void slotActivate();

protected:
    KURL m_url;
    InternetRadio *m_internetRadio;
};

#endif
