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

//#include <kiconloader.h>
//#include <kstandarddirs.h>
//#include <qxml.h>
//#include <kio/netaccess.h>
//#include <linux/soundcard.h>
//#include "docking.h"
//#include "radiocfgxmlhandler.h"
//#include "kradio.h"

#include "kradioapp.h"


//QColor defaultDisplayColor    ( 77, 117,  77 );
//QColor defaultDisplayTextColor( 20, 244,  20 );




KRadioApp::KRadioApp()
//  : setupDialog(0, false)
{
    config = KGlobal::config();

    PluginManager::restoreState (config);

/*    
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
*/

}


KRadioApp::~KRadioApp()
{
	PluginManager::saveState (config);
}

/*

what's left in comments is not yet ported to new concept

void KRadioApp::readConfiguration()
{
	// first read XML station presets (and alarms for compatibility

    setupData.alarms.clear();
    setupData.stations.clear();
    
	readXMLCfg (locateLocal("data", "kradio/stations.krp"), setupData.stations, setupData.info, setupData.alarms);
	readXMLCfg (QString(getenv("HOME")) + "/.kradiorc",     setupData.stations, setupData.info, setupData.alarms);  // for backward compatibility

	// load colors

	setupData.displayColor     = config->readColorEntry ("displayColor",     &defaultDisplayColor);
	setupData.displayTextColor = config->readColorEntry ("displayTextColor", &defaultDisplayTextColor);
	setupData.displayTextFont  = config->readEntry      ("displayTextFont",  "Helvetica");

	// load other things

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

	// save colors

	config->writeEntry ("displayColor",     setupData.displayColor);
	config->writeEntry ("displayTextColor", setupData.displayTextColor);
	config->writeEntry ("displayTextFont",  setupData.displayTextFont);

    config->setGroup("QuickBar");

	config->writeEntry("showShortName", setupData.displayOnlyShortNames);
}

*/

/*
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
*/


//////////////////////////////////////////////////////////////////////////////////////
// global helper functions
//////////////////////////////////////////////////////////////////////////////////////

/*
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

*/
