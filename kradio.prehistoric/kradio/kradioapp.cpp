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
  radio = 0;

  // the actual radio
  radio = new V4LRadio(this, "");
  radio->readCfg (QString(getenv("HOME")) + "/.kradiorc");  //stations & alarms

  // the quick selection buttons
  quickbar = new QuickBar (radio, 0, "kradio-quickbar");

  // the main dialog
  kradio = new KRadio(quickbar, radio, 0, "kradio-gui");
 
  // Tray
  tray = new RadioDocking(kradio, quickbar, radio);


  // load Configuration
  config = KGlobal::config();
  readOptions();

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

	if (radio) {

		QString rDev = config->readEntry ("RadioDev", "/dev/radio");
		QString mDev = config->readEntry ("MixerDev", "/dev/mixer");
		QString s = config->readEntry ("MixerChannel", "line");
		int MixerChannel = 0;
		for (MixerChannel = 0; MixerChannel < SOUND_MIXER_NRDEVICES; ++MixerChannel) {
			if (s == mixerChannelLabels[MixerChannel] ||
				s == mixerChannelNames[MixerChannel])
				break;
		}
		if (MixerChannel == SOUND_MIXER_NRDEVICES)
			MixerChannel = SOUND_MIXER_LINE;

		radio->setDevices(rDev, mDev, MixerChannel);

		radio->setRangeOverride(config->readBoolEntry ("fRangeOverride", false),
		  					    config->readDoubleNumEntry ("fMinOverride", 87.0),
							    config->readDoubleNumEntry ("fMaxOverride", 108.0));
	}
}


void KRadioApp::saveOptions()
{
    config->setGroup("devices");
    config->writeEntry("MixerDev", radio ? radio->mixerDevice() : QString("/dev/radio"));
    config->writeEntry("RadioDev", radio ? radio->radioDevice() : QString("/dev/mixer"));
    int ch = radio ? radio->mixerChannel() : 0;
    if(ch < 0 || ch >= SOUND_MIXER_NRDEVICES)
		ch = SOUND_MIXER_LINE;
    config->writeEntry("MixerChannel", mixerChannelNames[ch]);
    config->writeEntry("fRangeOverride", radio ? radio->isRangeOverrideSet() : false);
    config->writeEntry("fMinOverride",   radio ? radio->minFrequency() : 87.0);
    config->writeEntry("fMaxOverride",   radio ? radio->maxFrequency() : 108.0);
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
	SetupDialog	sud (0, radio ? radio->radioDevice() : QString("/dev/radio"),
						radio ? radio->mixerDevice() : QString("/dev/mixer"),
						radio ? radio->mixerChannel() : 0,
						quickbar ? quickbar->getShowShortName() : false);
	sud.setStations(radio->getStations(), radio);
	sud.setAlarms(radio->getAlarms());
	sud.setRangeOverride(radio->isRangeOverrideSet(),
						 radio->minFrequency(),
						 radio->maxFrequency());

	connect (&sud, SIGNAL(sigSaveConfig(SetupDialog &)),
		this, SLOT(slotSaveConfig(SetupDialog &)));
	connect (&sud, SIGNAL(sigApplyConfig(SetupDialog &)),
		this, SLOT(slotApplyConfig(SetupDialog &)));
		
	if (sud.exec() == QDialog::Accepted) {
		slotApplyConfig (sud);
	}
}


void KRadioApp::slotApplyConfig (SetupDialog &sud)
{
	radio->setStations(sud.getStations());
	radio->setAlarms(sud.getAlarms());
	if (quickbar)
		quickbar->setShowShortName(sud.displayOnlyShortNames());
	radio->setDevices(sud.getRadioDevice(),
	                  sud.getMixerDevice(),
	                  sud.getMixerChannel()
					 );
	radio->setRangeOverride(sud.isRangeOverrideSet(),
							sud.fMinOverride(),
							sud.fMaxOverride());
}


void KRadioApp::slotSaveConfig (SetupDialog &sud)
{
	slotApplyConfig(sud);

	radio->writeCfg(QString(getenv("HOME")) + "/.kradiorc");
	saveOptions();
}

