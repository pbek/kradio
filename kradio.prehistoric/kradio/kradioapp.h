/***************************************************************************
                          kradioapp.h  -  description
                             -------------------
    begin                : Sa Feb  9 2002
    copyright            : (C) 2002 by Klas Kalass / Martin Witte / Frank Schwanz
    email                : klas.kalass@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRADIO_KRADIOAPP_H
#define KRADIO_KRADIOAPP_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapplication.h>
#include <kaboutapplication.h>
#include "pluginmanager.h"

class KRadioApp : public KApplication,
				  public PluginManager
{
Q_OBJECT
public:
    KRadioApp();
    virtual ~KRadioApp();

private:
    KAboutApplication AboutApplication;

    KConfig       *config;

//    SetupDialog    setupDialog;
};


#endif
