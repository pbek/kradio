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
#include <qxml.h>
#include <kio/netaccess.h>
#include <linux/soundcard.h>

#include "kradio.h"
//#include "kradiomw.h"

#include "kradioapp.h"
#include "docking.h"
#include "radiocfgxmlhandler.h"



QColor defaultDisplayColor    ( 77, 117,  77 );
QColor defaultDisplayTextColor( 20, 244,  20 );

const char *AlarmListElement            = "alarmlist";
const char *AlarmElement                = "alarm";
const char *AlarmDateElement            = "date";
const char *AlarmTimeElement            = "time";
const char *AlarmDailyElement           = "daily";
const char *AlarmEnabledElement         = "enabled";
const char *AlarmStationIDElement       = "stationID";
const char *AlarmFrequencyElement       = "frequency";
const char *AlarmVolumeElement          = "volume";



KRadioApp::KRadioApp()
 : kradio(0),
   tray(0),
   quickbar(0),
   timeControl(0),
   radio(0),
   setupDialog(0, false)
{
    // set configuration
    config = KGlobal::config();
    setupStdConnections (this);
    setupStdConnections (&setupDialog);

    // the actual radio
    setupData.radio = radio = new V4LRadio(this, "");
    setupStdConnections (radio);

    // the quick selection buttons
    quickbar = new QuickBar (radio, 0, "kradio-quickbar");
    setupStdConnections (quickbar);

    // the main dialog
    kradio = new KRadio(quickbar, radio, 0, "kradio-gui");
    setupStdConnections (kradio);

    // timeControl
    timeControl = new TimeControl(this, "kradio-timecontrol");
    setupStdConnections (timeControl);

    // Tray
    tray = new RadioDocking(kradio, quickbar, radio, timeControl);
    setupStdConnections (tray);
  
    // lirc
#ifdef HAVE_LIRC_CLIENT
	lircHelper = new LircSupport (this, radio, timeControl);
	setupStdConnections (lircHelper);
#endif

	// read configuration
	readConfiguration();

	// restore gui state
	emit sigRestoreState();


	//

	tray->setPixmap(BarIcon("kradio"));
	connect(tray, SIGNAL(showAbout()),   &AboutApplication, SLOT(show()));
	tray->show();

	connect(kradio, SIGNAL(showAbout()), &AboutApplication, SLOT(show()));
	connect(kradio, SIGNAL(runConfigure()), this, SLOT(slotRunConfigure()));

	connect(timeControl, SIGNAL(sigAlarm(Alarm *)), this, SLOT(slotAlarm(Alarm *)));
	connect(timeControl, SIGNAL(sigCountdownZero()), radio, SLOT(PowerOff()));

	connect(radio, SIGNAL(sigPowerOn(bool)), timeControl, SLOT(stopCountdown()));

	connect(&setupDialog, SIGNAL(apply()),         this, SLOT(slotApplyConfig()));
	connect(&setupDialog, SIGNAL(okClicked()),     this, SLOT(slotApplyConfig()));
	connect(&setupDialog, SIGNAL(cancelClicked()), this, SLOT(initSetupDialog()));

}


KRadioApp::~KRadioApp()
{
    saveConfiguration();

    emit sigSaveState();

    if (kradio)
        delete kradio;
    if (radio)
        delete radio;
    if (quickbar)
        delete quickbar;
    radio = 0;

#ifdef HAVE_LIRC_CLIENT
    delete lircHelper;
#endif
}


void KRadioApp::addPlugin (QObject *o)
{
	if (!o)
		return;

	objects.append(o);
	if (quietconnect (this, SIGNAL(sigConnectPlugin(QObjectList &)),
					  o, SLOT(connectPlugin(QObjectList &)))) {
	    emit sigConnectPlugin(objects);
	    disconnect (this, SIGNAL(sigConnectPlugin(QObjectList &)),
					  o, SLOT(connectPlugin(QObjectList &)));
	}
}


void KRadioApp::readConfiguration()
{
	// first read XML station presets (and alarms for compatibility

    setupData.alarms.clear();
    setupData.stations.clear();
    
	readXMLCfg (locateLocal("data", "kradio/stations.krp"), setupData.stations, setupData.info, setupData.alarms);
	readXMLCfg (QString(getenv("HOME")) + "/.kradiorc",     setupData.stations, setupData.info, setupData.alarms);  // for backward compatibility

    // now read options from kradiorc file
	
	config->setGroup("devices");

	setupData.radioDev = config->readEntry ("RadioDev", "/dev/radio");
	setupData.mixerDev = config->readEntry ("MixerDev", "/dev/mixer");
	QString s          = config->readEntry ("MixerChannel", "line");
	setupData.mixerChannel = 0;
	for (setupData.mixerChannel = 0; setupData.mixerChannel < SOUND_MIXER_NRDEVICES; ++setupData.mixerChannel) {
		if (s == mixerChannelLabels[MixerChannel] ||
			s == mixerChannelNames[MixerChannel])
			break;
	}
	if (setupData.mixerChannel == SOUND_MIXER_NRDEVICES)
		setupData.mixerChannel = SOUND_MIXER_LINE;

	setupData.enableRangeOverride   = config->readBoolEntry      ("fRangeOverride", false);
	setupData.minFrequency          = config->readDoubleNumEntry ("fMinOverride", 87.0);
	setupData.maxFrequency          = config->readDoubleNumEntry ("fMaxOverride", 108.0);

	setupData.signalMinQuality      = config->readDoubleNumEntry ("signalMinQuality", 0.75);
	setupData.scanStep              = config->readDoubleNumEntry ("scanStep", 0.05);


	
	config->setGroup("alarms");

	int nAlarms = config->readNumEntry ("nAlarms", 0);
	for (int idx = 1; idx <= nAlarms; ++idx) {
		QString num = QString().setNum(idx);
		QDateTime d = config->readDateTimeEntry(AlarmTimeElement       + num);
		bool enable = config->readBoolEntry(AlarmEnabledElement        + num, false);
		bool daily  = config->readBoolEntry(AlarmDailyElement          + num, false);
		float vol   = config->readDoubleNumEntry(AlarmVolumeElement    + num, 1);
		float freq  = config->readDoubleNumEntry(AlarmFrequencyElement + num, -1234);

		// read StationID only for compatibility
		int   stId  = config->readNumEntry(AlarmStationIDElement       + num, -1);

		// for compatibility
		if (freq != -1234) { // the freq entry already exists
			// nothing to do
		} else {             // the freq entry does not yet exist, use stationID if exists
			freq = -1;
			if (stId >= 0 && stId < setupData.stations.size())
				freq = setupData.stations[stId]->getFrequency();
		}

		enable &= d.isValid();

		Alarm a ( this, d, daily, enable);
		a->setVolumePreset(vol);
		a->setFrequency(freq);
		setupData.alarms.push_back(a);
	}

	// load colors

	setupData.displayColor     = config->readColorEntry ("displayColor",     &defaultDisplayColor);
	setupData.displayTextColor = config->readColorEntry ("displayTextColor", &defaultDisplayTextColor);
	setupData.displayTextFont  = config->readEntry      ("displayTextFont",  "Helvetica");

	// load other things

	config->setGroup("sleep");

	setupData.sleep  = config->readNumEntry("sleepMinutes", 30) * 60;

    config->setGroup("QuickBar");

	setupData->displayOnlyShortNames = config->readBoolEntry("showShortName", true);


	// notify all other objects

	emit sigConfigurationChanged(setupData);
}


void KRadioApp::saveConfiguration()
{
	writeXMLCfg(locateLocal("data", "kradio/stations.krp"),
							setupData.stations,
							setupData.info
			   );

	config->setGroup("devices");

	config->writeEntry("MixerDev", setupData.mixerDev);
	config->writeEntry("RadioDev", setupData.radioDev);
	int ch = setupData.mixerChannel;
	if(ch < 0 || ch >= SOUND_MIXER_NRDEVICES)
		ch = SOUND_MIXER_LINE;
	config->writeEntry("MixerChannel",   mixerChannelNames[ch]);
	config->writeEntry("fRangeOverride", setupData.enableRangeOverride);
	config->writeEntry("fMinOverride",   setupData.minFrequency);
	config->writeEntry("fMaxOverride",   setupData.maxFrequency);

	config->writeEntry("signalMinQuality", setupData.signalMinQuality);

	config->writeEntry("scanStep", setupData.scanStep);

    // save alarms

	config->setGroup("alarms");

	config->writeEntry("nAlarms", setupData.alarms.size());
	int idx = 1;
	for (ciAlarmVector i = setupData.alarms.begin(); i != setupData.alarms.end(); ++i, ++idx) {
		QString num = QString().setNum(idx);
		config->writeEntry (AlarmTimeElement      + num, i->alarmTime());
		config->writeEntry (AlarmEnabledElement   + num, i->isEnabled());
		config->writeEntry (AlarmDailyElement     + num, i->isDaily());
		config->writeEntry (AlarmVolumeElement    + num, i->getVolumePreset());
		config->writeEntry (AlarmFrequencyElement + num, i->getFrequency());
		config->deleteEntry(AlarmStationIDElement + num);
	}

	// save colors

	config->writeEntry ("displayColor",     setupData.displayColor);
	config->writeEntry ("displayTextColor", setupData.displayTextColor);
	config->writeEntry ("displayTextFont",  setupData.displayTextFont);

	// save other things

	config->setGroup("sleep");

	config->writeEntry("sleepMinutes",  setupData.sleep / 60);


    config->setGroup("QuickBar");

	config->writeEntry("showShortName", setupData.displayOnlyShortNames);
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

//    emit sigRestoreState(config);
/*    // kradio: restore window size and position
    if (kradio)
        kradio->restoreState(config);

    // quickbar: restore size/position
    if (quickbar)
        quickbar->restoreState(config);
*/
}


void KRadioApp::saveState()
{
    config->setGroup("Recent");
    if (radio)
        config->writeEntry("Frequency", radio->getFrequency());
    config->writeEntry("Volume", radio->getVolume());

    config->writeEntry("PowerOn", radio->isPowerOn());

//    emit sigSaveState(config);

/*    // kradio: save window size and position
    if (kradio)
        kradio->saveState(config);

    // quickbar: restore size/position
    if (quickbar)
        quickbar->saveState(config);
*/
}


void KRadioApp::configurationChanged (const SetupData &sud)
{
}


void KRadioApp::slotRunConfigure()
{
	setupDialog.show();
	setupDialog.raise();
}


void KRadioApp::slotApplyConfig ()
{
	setupDialog.getData(setupData);

	emit sigConfigurationChanged (setupData);
}


void KRadioApp::slotAlarm (Alarm *a)
{
	if (radio && a) {

		float f = a->getFrequency();
		if (f >= 0)
			radio->setFrequency (f);

		radio->PowerOn ();

		float v = a->getVolumePreset();

		if (v >= 0)
			radio->setVolume (v);

	}
}



//////////////////////////////////////////////////////////////////////////////////////
// global helper functions
//////////////////////////////////////////////////////////////////////////////////////


void readXMLCfg (const QString &url,
				 StationVector &sl,
                 StationListMetaData &info,
                 AlarmVector &al
                )
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
		sl = handler.getStations();
		info = handler.getStationListMetaData();
	}

	if (al.size() == 0) {
		al = handler.getAlarms();
	}
}




QString writeXMLCfg (const StationVector &sl,
				     const StationListMetaData &info
				    )
{
	QString data = "";

	// write station list

	QString t   = "\t";
	QString tt  = "\t\t";
	QString ttt = "\t\t\t";

	data +=       xmlOpenTag(KRadioConfigElement) +
	        t   + xmlOpenTag(StationListElement) +
	        tt  + xmlOpenTag(StationListInfo) +
	        ttt + xmlTag(StationListInfoMaintainer, info.Maintainer) +
	        ttt + xmlTag(StationListInfoChanged,    info.LastChange.toString(Qt::ISODate)) +
	        ttt + xmlTag(StationListInfoCountry,    info.Country) +
	        ttt + xmlTag(StationListInfoCity,       info.City) +
	        ttt + xmlTag(StationListInfoMedia,      info.Media) +
	        ttt + xmlTag(StationListInfoComments,   info.Comment) +
	        tt  + xmlCloseTag (StationListInfo);

	for (ciStationVector i = sl.begin(); i != sl.end(); ++i) {
		data += tt  + xmlOpenTag (StationElement) +
				ttt + xmlTag (StationNameElement,         i->name()) +
				ttt + xmlTag (StationShortNameElement,    i->getShortName()) +
				ttt + xmlTag (StationIconStringElement,   i->getIconString()) +
				ttt + xmlTag (StationQuickSelectElement,  i->useQuickSelect()) +
				ttt + xmlTag (StationFrequencyElement,    i->getFrequency()) +
				ttt + xmlTag (StationVolumePresetElement, i->getVolumePreset()) +
				ttt + xmlTag (StationDockingMenuElement,  i->useInDockingMenu()) +
				tt  + xmlCloseTag(StationElement);
	}
	data += t + xmlCloseTag(StationListElement) +
			    xmlCloseTag(KRadioConfigElement);

	return data;
}



void writeXMLCfg (const QString &FileName,
				  const StationVector &sl,
				  const StationListMetaData &info
				 )
{
	// TODO: error handling
	::rename(FileName, FileName + "~");

	FILE *f = fopen (FileName, "w");

	// write station list

	fprintf (f, "%s", (const char*)writeXMLCfg(sl, info));

	fclose (f);
}
