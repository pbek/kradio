/**************************************************************************
                          kradioapp.cpp  -  description
                             -------------------
    begin                : Sa Feb  9 CET 2002
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kiconloader.h>
#include <kconfig.h>
#include <klocale.h>

#include "kradio.h"
#include "kradioapp.h"
#include "docking.h"
#include "setupdialog.h"


KRadioApp::KRadioApp()
  :kradio(0),
   tray(0),
   quickbar(0)
{
  config = KGlobal::config(); 
  readOptions();

  // the actual radio
  radio = new V4LRadio(this, "", RadioDev, MixerDev, MixerChannel);
  radio->readCfg (QString(getenv("HOME")) + "/.kradiorc");

  // the quick selection buttons
  quickbar = new QuickBar (radio, 0, "kradio-quickbar");

  // the main dialog
  kradio = new KRadio(quickbar, radio, 0, "kradio-gui");
 
  // Tray
  tray = new RadioDocking(kradio, quickbar, radio);

  // restore Configuration
  restoreState();

  if (tray){
    tray->setPixmap(BarIcon("kradio"));
    connect(tray, SIGNAL(showAbout()),   &AboutApplication, SLOT(show()));
    tray->show();
  }
  if (kradio){
    connect(kradio, SIGNAL(showAbout()), &AboutApplication, SLOT(show()));
    connect(kradio, SIGNAL(runConfigure()), this, SLOT(slotConfigure()));
  }
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
    delete quickbar;
  radio = 0;
}

void KRadioApp::readOptions()
{
  config->setGroup("devices");
  RadioDev = config->readEntry ("RadioDev", "/dev/radio");
  MixerDev = config->readEntry ("MixerDev", "/dev/mixer");	
  QString s = config->readEntry ("MixerChannel", "line");
  MixerChannel = 0;
  for (MixerChannel = 0; MixerChannel < SOUND_MIXER_NRDEVICES; ++MixerChannel)
	  if (s == mixerChannelLabels[MixerChannel] ||
		  s == mixerChannelNames[MixerChannel])
		  break;
  if (MixerChannel == SOUND_MIXER_NRDEVICES)
	  MixerChannel = SOUND_MIXER_LINE;
}

void KRadioApp::saveOptions()
{
    config->setGroup("devices");
    config->writeEntry("MixerDev", MixerDev);
    config->writeEntry("RadioDev", RadioDev);
    if(MixerChannel <= 0 || MixerChannel >= SOUND_MIXER_NRDEVICES)
		MixerChannel = SOUND_MIXER_LINE;
    config->writeEntry("MixerChannel", mixerChannelNames[MixerChannel]);
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
    kradio->restoreState(config);

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



void KRadioApp::slotConfigure()
{
	SetupDialog	sud (0, RadioDev, MixerDev, MixerChannel, quickbar ? quickbar->getShowShortName() : false);
	sud.setStations(radio->getStations(), radio);
	sud.setAlarms(radio->getAlarms());

	connect (&sud, SIGNAL(sigSaveConfig(const StationVector &, const AlarmVector &, const QString &, const QString &, int, bool)),
		this, SLOT(slotSaveConfig(const StationVector &, const AlarmVector &, const QString &, const QString &, int, bool)));
		
	if (sud.exec() == QDialog::Accepted) {
		radio->setStations(sud.getStations());
		radio->setAlarms(sud.getAlarms());
		if (quickbar)
			quickbar->setShowShortName(sud.displayOnlyShortNames());
		RadioDev = sud.getRadioDevice();
		MixerDev = sud.getMixerDevice();
		MixerChannel = sud.getMixerChannel();
		radio->setDevices(RadioDev, MixerDev, MixerChannel);
	}
}


void KRadioApp::slotSaveConfig (const StationVector &sl,
								const AlarmVector &al,
							    const QString &rdev,
							    const QString &mdev,
							    int ch,
							    bool sn)
{
	radio->setStations(sl);
	radio->setAlarms(al);
	if (quickbar)
		quickbar->setShowShortName(sn);
	RadioDev = rdev;
	MixerDev = mdev;
	MixerChannel = ch;
	radio->setDevices(RadioDev, MixerDev, MixerChannel);

	radio->writeCfg(QString(getenv("HOME")) + "/.kradiorc");
	saveOptions();
}

