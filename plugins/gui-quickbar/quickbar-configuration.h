/***************************************************************************
                          quickbar-configuration.h  -  description
                             -------------------
    begin                : Son Aug 3 2003
    copyright            : (C) 2003 by Martin Witte
    email                : emw-kradio@nocabal.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRADIO_QUICKBAR_CONFIGURATION_H
#define KRADIO_QUICKBAR_CONFIGURATION_H

#include "stationselector.h"

class QuickbarConfiguration : public StationSelector
{
Q_OBJECT
public :
    QuickbarConfiguration (QWidget *parent);
    ~QuickbarConfiguration ();

};

#endif
