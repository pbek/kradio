/***************************************************************************
                          kradioapp.h  -  description
                             -------------------
    begin                : Sa Feb  9 2002
    copyright            : (C) 2002 by Klas Kalass
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



#ifndef KRADIOAPP_H
#define KRADIOAPP_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapplication.h>
#include <qstring.h>
#include <kaboutapplication.h>

#include "v4lradio.h"

class RadioDocking;
class KRadio;

class KRadioApp : public KApplication 
{
 public:
  KRadioApp();
  virtual ~KRadioApp();
  
 private:
  void restoreState();
  void readOptions();

  void saveState();
  void saveOptions();

  KRadio *kradio;
  KAboutApplication AboutApplication;
  
  KConfig *config;
  RadioDocking *tray;
  V4LRadio *radio;
  
  QString MixerDev, RadioDev;

};

#endif
