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
#include <kstandarddirs.h>
#include <kconfig.h>
#include <klocale.h>
#include <qxml.h>
#include <kio/netaccess.h>

#include "kradio.h"
#include "kradioapp.h"
#include "docking.h"
#include "setupdialog.h"
#include "radiocfgxmlhandler.h"


KRadioApp::KRadioApp()
  :kradio(0),
   tray(0),
   quickbar(0),
   timeControl(0),
   radio(0)
{
  // the actual radio
  radio = new V4LRadio(this, "");

  // the quick selection buttons
  quickbar = new QuickBar (radio, 0, "kradio-quickbar");

  // the main dialog
  kradio = new KRadio(quickbar, radio, 0, "kradio-gui");
 
  // timeControl
  timeControl = new TimeControl(this, "kradio-timecontrol");

  // Tray
  tray = new RadioDocking(kradio, quickbar, radio, timeControl);

  // lirc
#ifdef HAVE_LIRC_CLIENT
	lircHelper = new LircSupport (this, radio, timeControl);
#endif
  
  // load XML station configuration (& backward compatibility: alarms)
  readXMLConfig();
  
  // load "normal" configuration
  config = KGlobal::config();
  readOptions();

  // restore Configuration
  restoreState();

  tray->setPixmap(BarIcon("kradio"));
  connect(tray, SIGNAL(showAbout()),   &AboutApplication, SLOT(show()));
  tray->show();

  connect(kradio, SIGNAL(showAbout()), &AboutApplication, SLOT(show()));
  connect(kradio, SIGNAL(runConfigure()), this, SLOT(slotConfigure()));

  connect(timeControl, SIGNAL(sigAlarm(Alarm *)), this, SLOT(slotAlarm(Alarm *)));
  connect(timeControl, SIGNAL(sigCountdownZero()), radio, SLOT(PowerOff()));

  connect(radio, SIGNAL(sigPowerOn(bool)), timeControl, SLOT(stopCountdown()));
}


KRadioApp::~KRadioApp()
{
  saveOptions();
  saveState();

  if (radio)
	  writeXMLCfg(locateLocal("data", "kradio/stations.krp"), radio->getStations());

  if (kradio){
    delete kradio;
  }
  if (radio)
    delete radio;
  if (quickbar)
    delete quickbar;
  radio = 0;

#ifdef HAVE_LIRC_CLIENT
  delete lircHelper;
#endif
}


void KRadioApp::readOptions()
{
	if (radio) {

		config->setGroup("devices");

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

		radio->setSignalMinQuality(config->readDoubleNumEntry ("signalMinQuality", 0.75));
	}


	if (timeControl) {
	
		config->setGroup("alarms");
		
		AlarmVector al;
		int nAlarms = config->readNumEntry ("nAlarms", 0);
		for (int idx = 1; idx <= nAlarms; ++idx) {
			QDateTime d = config->readDateTimeEntry(AlarmTimeElement    + QString().setNum(idx));
			bool enable = config->readBoolEntry(AlarmEnabledElement     + QString().setNum(idx), false);
			bool daily  = config->readBoolEntry(AlarmDailyElement       + QString().setNum(idx), false);
			float vol   = config->readDoubleNumEntry(AlarmVolumeElement + QString().setNum(idx), 1);
			int   stId  = config->readNumEntry(AlarmStationIDElement    + QString().setNum(idx), 0);

			enable &= d.isValid();
			
			Alarm *a = new Alarm ( this, d, daily, enable);
			a->setVolumePreset(vol);
			a->setStationID(stId);
			al.push_back(a);
		}

		if (al.size()) // only set alarms if alread written to standard kde config file (otherwise still in ~/.kradiorc)
			timeControl->setAlarms(al);
		for (iAlarmVector i = al.begin(); i != al.end(); ++i)
			delete *i;



		config->setGroup("sleep");
		
		timeControl->setCountdownSeconds(config->readNumEntry("sleepMinutes", 30) * 60);
	}

}


void KRadioApp::saveOptions()
{
    if (radio) {

        config->setGroup("devices");

		config->writeEntry("MixerDev", radio->mixerDevice());
		config->writeEntry("RadioDev", radio->radioDevice());
		int ch = radio->mixerChannel();
		if(ch < 0 || ch >= SOUND_MIXER_NRDEVICES)
			ch = SOUND_MIXER_LINE;
		config->writeEntry("MixerChannel", mixerChannelNames[ch]);
		config->writeEntry("fRangeOverride", radio->isRangeOverrideSet());
		config->writeEntry("fMinOverride",   radio->minFrequency());
		config->writeEntry("fMaxOverride",   radio->maxFrequency());

		config->writeEntry("signalMinQuality", radio->getSignalMinQuality());

    }

    // save alarms

    if (timeControl) {

        config->setGroup("alarms");

		const AlarmVector &al = timeControl->getAlarms();
		config->writeEntry("nAlarms", al.size());
		int idx = 1;
		for (ciAlarmVector i = al.begin(); i != al.end(); ++i, ++idx) {
			const Alarm *a = *i;
			config->writeEntry(AlarmTimeElement      + QString().setNum(idx), a->alarmTime());
			config->writeEntry(AlarmEnabledElement   + QString().setNum(idx), a->isEnabled());
			config->writeEntry(AlarmDailyElement     + QString().setNum(idx), a->isDaily());
			config->writeEntry(AlarmVolumeElement    + QString().setNum(idx), a->getVolumePreset());
			config->writeEntry(AlarmStationIDElement + QString().setNum(idx), a->getStationID());
		}


		config->setGroup("sleep");

		config->writeEntry("sleepMinutes", (int)(timeControl->getCountdownSeconds() / 60));
	}
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
	sud.setAlarms(timeControl->getAlarms());
	sud.setRangeOverride(radio->isRangeOverrideSet(),
						 radio->minFrequency(),
						 radio->maxFrequency());
	sud.setSleep(timeControl->getCountdownSeconds() / 60);
	sud.setSignalMinQuality(radio->getSignalMinQuality());

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
	timeControl->setAlarms(sud.getAlarms());
	if (quickbar)
		quickbar->setShowShortName(sud.displayOnlyShortNames());
	radio->setDevices(sud.getRadioDevice(),
	                  sud.getMixerDevice(),
	                  sud.getMixerChannel()
					 );
	radio->setRangeOverride(sud.isRangeOverrideSet(),
							sud.fMinOverride(),
							sud.fMaxOverride());

	radio->setSignalMinQuality(sud.getSignalMinQuality());
	timeControl->setCountdownSeconds(sud.getSleep()*60);
}


void KRadioApp::slotSaveConfig (SetupDialog &sud)
{
	slotApplyConfig(sud);

	if (radio)
		writeXMLCfg(locateLocal("data", "kradio/stations.krp"), radio->getStations());
	saveOptions();
}



void KRadioApp::slotAlarm (Alarm *a)
{
	if (radio && a) {

		int id = a->getStationID();
		if (id >= 0)
			radio->activateStation (id);

		radio->PowerOn ();

		float v = a->getVolumePreset();

		radio->setVolume (v < 0 ? 1.0 : v);

	}
}


void KRadioApp::readXMLConfig()
{
	AlarmVector   al;
	StationVector sl;

	readXMLCfg (locateLocal("data", "kradio/stations.krp"), sl, al);
	readXMLCfg (QString(getenv("HOME")) + "/.kradiorc",     sl, al);  // for backward compatibility

	if (radio && radio->getStations().size() == 0)
		radio->setStations (sl);

	if (timeControl && timeControl->getAlarms().size() == 0)  // should be now in std kde config file,
		timeControl->setAlarms (al);                          // but take care for compatibility

	for (iAlarmVector i = al.begin(); i != al.end(); ++i)
		delete *i;
	for (iStationVector i = sl.begin(); i != sl.end(); ++i)
		delete *i;
}


//////////////////////////////////////////////////////////////////////////////////////
// global helper functions
//////////////////////////////////////////////////////////////////////////////////////


void readXMLCfg (const QString &url, StationVector &sl, AlarmVector &al)
{
	// TODO: error handling
	QString tmpfile;
	KIO::NetAccess::download(url, tmpfile);
	QFile cfg (tmpfile);
	QXmlInputSource source (cfg);
	QXmlSimpleReader reader;
	RadioCfgXmlHandler handler;
	reader.setContentHandler (&handler);
	reader.parse(source);
	KIO::NetAccess::removeTempFile(tmpfile);
	
	if (sl.size() == 0) {                                     // first non-empty configuration
		const StationVector &_sl = handler.getStations();
		for (ciStationVector i = _sl.begin(); i != _sl.end(); ++i)
			sl.push_back(new RadioStation(**i));
	}

	if (al.size() == 0) {
		const AlarmVector &_al = handler.getAlarms();
		for (ciAlarmVector i = _al.begin(); i != _al.end(); ++i)
			al.push_back(new Alarm(**i));
	}
}


void writeXMLCfg (const QString &FileName, const StationVector &sl /*, const AlarmVector &al*/)
{
//	const StationVector &sl = radio->getStations();
//	const AlarmVector   &al = timeControl->getAlarms();

	// TODO: create backup copy!
	// TODO: error handling
	::rename(FileName, FileName + "~");

	FILE *f = fopen (FileName, "w");

	// write station list

	fprintf (f, "<%s>\n", KRadioConfigElement);
	fprintf (f, "\t<%s>\n", StationListElement);
	for (ciStationVector i = sl.begin(); i != sl.end(); ++i) {
		const RadioStation *st = *i;
		fprintf (f, "\t\t<%s>\n", StationElement);
		fprintf (f, "\t\t\t<%s>%s</%s>\n",   StationNameElement,         (const char*)st->name(),		   StationNameElement);
		fprintf (f, "\t\t\t<%s>%s</%s>\n",   StationShortNameElement,    (const char*)st->getShortName(),  StationShortNameElement);
		fprintf (f, "\t\t\t<%s>%s</%s>\n",   StationIconStringElement,   (const char*)st->getIconString(), StationIconStringElement);
		fprintf (f, "\t\t\t<%s>%i</%s>\n",   StationQuickSelectElement,  st->useQuickSelect(),			   StationQuickSelectElement);
		fprintf (f, "\t\t\t<%s>%.6f</%s>\n", StationFrequencyElement,    st->getFrequency(),    		   StationFrequencyElement);
		fprintf (f, "\t\t\t<%s>%.6f</%s>\n", StationVolumePresetElement, st->getVolumePreset(), 		   StationVolumePresetElement);
		fprintf (f, "\t\t\t<%s>%i</%s>\n",   StationDockingMenuElement,  st->useInDockingMenu(), 		   StationDockingMenuElement);
		fprintf (f, "\t\t</%s>\n", StationElement);
	}
	fprintf (f, "\t</%s>\n", StationListElement);

	// write alarm list

/*	fprintf (f, "\t<%s>\n", AlarmListElement);
	for (ciAlarmVector i = al.begin(); i != al.end(); ++i) {
		const Alarm *a = *i;
		QDate d = a->alarmTime().date();
		QTime t = a->alarmTime().time();
		fprintf (f, "\t\t<%s>\n", AlarmElement);
		fprintf (f, "\t\t\t<%s>%04i-%02i-%02i</%s>\n",  AlarmDateElement,    d.year(), d.month(),  d.day(),	   AlarmDateElement);
		fprintf (f, "\t\t\t<%s>%02i:%02i:%02i</%s>\n",  AlarmTimeElement,    t.hour(), t.minute(), t.second(), AlarmTimeElement);
		fprintf (f, "\t\t\t<%s>%i</%s>\n",              AlarmEnabledElement, a->isEnabled(),		   	       AlarmEnabledElement);
		fprintf (f, "\t\t\t<%s>%i</%s>\n",              AlarmDailyElement,   a->isDaily(),                     AlarmDailyElement);
		fprintf (f, "\t\t\t<%s>%.6f</%s>\n",            AlarmVolumeElement,  a->getVolumePreset(),             AlarmVolumeElement);
		fprintf (f, "\t\t\t<%s>%i</%s>\n",              AlarmStationIDElement, a->getStationID(),              AlarmStationIDElement);
		fprintf (f, "\t\t</%s>\n", AlarmElement);
	}
	fprintf (f, "\t</%s>\n", AlarmListElement);
*/

	fprintf (f, "</%s>\n", KRadioConfigElement);
	fclose (f);
}
