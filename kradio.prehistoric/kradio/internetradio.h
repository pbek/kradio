/***************************************************************************
                          internetradio.h  -  description
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

#ifndef INTERNETRADIO_H
#define INTERNETRADIO_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "radiodevice.h"

// forward declarations
class InternetRadioStation;

/**
 * @author Klas Kalass
 */
class InternetRadio : public RadioDevice  {
public:
    InternetRadio(Radio *mainRadio):RadioDevice(mainRadio){};
    ~InternetRadio(){};

    virtual bool activateStation(InternetRadioStation *station) = 0;
};

#endif
