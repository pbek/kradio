/***************************************************************************
                          docking-configuration.h  -  description
                             -------------------
    begin                : Son Aug 3 2003
    copyright            : (C) 2003 by Martin Witte
    email                : witte@kawo1.rwth-aachen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRADIO_DOCKING_CONFIGURATION_H
#define KRADIO_DOCKING_CONFIGURATION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../../src/libkradio-gui/stationselector.h"

class DockingConfiguration : public StationSelector
{
Q_OBJECT
public :
    DockingConfiguration (QWidget *parent);
    ~DockingConfiguration ();

};

#endif
