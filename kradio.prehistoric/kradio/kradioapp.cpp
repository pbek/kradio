/**************************************************************************
                          kradioapp.cpp  -  description
                             -------------------
    begin                : Sa Feb  9 CET 2002
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kradio.h"
#include "kradioapp.h"
#include "docking.h"
#include <kiconloader.h>
#include <kconfig.h>

KRadioApp::KRadioApp()
  :kradio(0),
   tray(0),
   quickbar(0)
{
  config = KGlobal::config(); 
  readOptions();

  // the actual radio
  radio = new V4LRadio(this, "", RadioDev, MixerDev);
  radio->readCfg (QString(getenv("HOME")) + "/.kradiorc");

  // the main dialog
  kradio = new KRadio(radio);

  // the quick selection buttons
  quickbar = new QuickBar (radio);
  
  // Tray
  tray = new RadioDocking(kradio, quickbar, radio);
  if (tray)
    tray->setPixmap(BarIcon("kradio"));

  if (kradio){
    connect(kradio, SIGNAL(showAbout()), &AboutApplication, SLOT(show()));
    kradio->show();
  }
  if (tray){
    connect(tray, SIGNAL(showAbout()),   &AboutApplication, SLOT(show()));
    tray->show();
  }

  quickbar->show();

  // restore Configuration
  restoreState();
}

KRadioApp::~KRadioApp()
{
  saveOptions();
  saveState();

  if (kradio){
    delete kradio;
  }
  if (radio)
    delete radio;
  if (quickbar)
    delete quickbar,
  radio = 0;
}

void KRadioApp::readOptions()
{
  config->setGroup("devices");
  RadioDev = config->readEntry ("RadioDev", "/dev/radio");
  MixerDev = config->readEntry ("MixerDev", "/dev/mixer");	
  
}

void KRadioApp::saveOptions()
{
    config->setGroup("devices");
    config->writeEntry("MixerDev", MixerDev);
    config->writeEntry("RadioDev", RadioDev);
}

void KRadioApp::restoreState()
{
  config->setGroup("Recent");
  float freq = config->readDoubleNumEntry("Frequency", 88);
  radio->setFrequency(freq);
  unsigned int vol = config->readNumEntry("Volume", 65535);
  radio->setVolume(vol);
  
  if (config->readBoolEntry ("PowerOn"))
    radio->PowerOn();

  // update kradio
  if (kradio)
    kradio->slotPowerOn (radio->isPowerOn());

  // kradio: restore window size and position
  if (kradio)
    kradio->readState(config);

  // quickbar: restore size/position
  if (quickbar)
      quickbar->restoreState(config);
}

void KRadioApp::saveState()
{
  config->setGroup("Recent");
  if (radio)
    config->writeEntry("Frequency", radio->getFrequency());
  config->writeEntry("Volume", radio->getVolume());
  
  config->writeEntry("PowerOn", radio->isPowerOn());

  // kradio: save window size and position
  if (kradio)
    kradio->saveState(config);

  // quickbar: restore size/position
  if (quickbar)
      quickbar->saveState(config);
}

