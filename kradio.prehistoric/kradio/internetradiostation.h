/***************************************************************************
                          internetradiostation.h  -  description
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

#ifndef KRADIO_INTERNETRADIOSTATION_H
#define KRADIO_INTERNETRADIOSTATION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "radiostation.h"

// KDE includes
#include <kurl.h>

/**
 * @author Klas Kalass, Ernst Martin Witte
 */
 
class InternetRadioStation : public RadioStation  {
public:
	InternetRadioStation();
    InternetRadioStation(KURL const &url);
    InternetRadioStation(const QString &name, const QString &shortName, KURL const &url);
    InternetRadioStation(const InternetRadioStation &);
    ~InternetRadioStation();

    const KURL & url() const             { return m_url; }
    void         setUrl(KURL const &url) { m_url = url;  }

    virtual QString longName() const;
    virtual bool    isValid () const;

    /*  = 0 : this.url == s.url
        > 0 : this.url >  s.url
        < 0 : this.url <  s.url
        other class than InternetRadioStation: compare typeid(.).name()
    */
    virtual int     compare (const RadioStation &s) const;

    /** returns an exact copy of this station */
    virtual RadioStation *copy() const;

protected:
    KURL m_url;
};

#endif
