/***************************************************************************
                          radiostation.h  -  description
                             -------------------
    begin                : Sat Feb 2 2002
    copyright            : (C) 2003 by Martin Witte, Klas Kalass
    email                : witte@kawo1.rwth-aachen.de / klas@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


/*

  Each radio station is described by a KURL. Thus internet radio stations are
  no longer a problem, AM/FM stations can be described by a special protocol,
  for example frequency://95.10

*/ 

#ifndef KRADIO_RADIOSTATION_H
#define KRADIO_RADIOSTATION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "utils.h"
#include <qstring.h>

/**
  *@author Martin Witte, Klas Kalass
  */

/*

   RadioStation

   RadioStation is an abstract base class for any type of radio station,
   e.g. AM/FM stations or internet radio stations. Thus no specific knowledge
   about the frequency or URL is included in this class. A radio station
   should not contain information on a matching device as well. The device has
   to decide on its own to use or not to use a station.

   There are some important abstract functions, that have to be overridden by
   a derived radio station:

      copy        create an exact copy of a station (in case we only have a RadioStation*
      longName    return a verbous station description
      isValid     is this station setup correctly ?
      equals      is this station equivalent to another station, e.g. approximately same frequency
  
*/

class RadioStation
{
public:
	RadioStation ();
	RadioStation (const QString &name, const QString &shortName);
	RadioStation (const RadioStation &);
	virtual ~RadioStation();

    virtual QString	longName() const = 0;

	const QString  &name()           const { return m_name;          }
	const QString  &shortName()      const { return m_shortName;     }
	const QString  &iconName()       const { return m_iconName;      }
	float           initialVolume()  const { return m_initialVolume; }

	void  setName         (const QString &name)       { m_name          = name;          }
	void  setShortName    (const QString &shortName)  { m_shortName     = shortName;     }
    void  setIconName     (const QString &iconName)   { m_iconName      = iconName;      }
	void  setInitialVolume(float initialVolume)       { m_initialVolume = initialVolume; }

    // is this station equivalent to another station?
    // e.g. same url or aproximate same frequency
    virtual bool equals(const RadioStation &s) const = 0;

    // is this station setup correctly ? 
    virtual bool isValid() const = 0;

    /** returns an exact copy of this station */
    virtual RadioStation *copy() const = 0;

protected :
	QString  m_name;
	QString	 m_shortName;
	float	 m_initialVolume;		// <0: => Don't use
	QString	 m_iconName;
};

#endif
