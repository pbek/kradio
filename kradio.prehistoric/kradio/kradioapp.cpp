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

#include "kradioapp.h"
#include <kaboutdata.h>
#include "aboutwidget.h"

// KRadioAbout

AboutPageInfo  KRadioAbout::createAboutPage ()
{
	const char *description = I18N_NOOP(
		"KRadio - The Radio Application for KDE"
		"<P>"
		"With KRadio you can listen to radio broadcasts with the help of your "
		"V4L/V4L2 compatible radio card."
		"<P>"
		"The KRadio Project contains a station preset data database. To complete "
		"this database you are encouraged to contribute your station preset file "
		"to the project. Just send it to one of the authors. "
		"<P>"
		"If you like to contribute your ideas, your own plugins or translations, "
		"don't hesitate to contact one of the authors."
		"<P>"
	);

    KAboutData aboutData("kradio", "KRadio",
                         VERSION,
                         description,
                         KAboutData::License_GPL,
                         "(c) 2002, 2003 Martin Witte, Klas Kalass",
                         0,
                         "http://sourceforge.net/projects/kradio",
                         0);
    aboutData.addAuthor("Martin Witte",  I18N_NOOP("Preset Database, Remote Control Support, Alarms, Rewrite for KRadio 0.3.0, Misc"), "witte@kawo1.rwth-aachen.de");
    aboutData.addAuthor("Klas Kalass",   I18N_NOOP("Miscellaneous"), "klas.kalass@gmx.de");
    aboutData.addAuthor("Frank Schwanz", I18N_NOOP("idea, first basic application"), "schwanz@fh-brandenburg.de");

    aboutData.addCredit(I18N_NOOP("Many People around the World ... "),
                        I18N_NOOP("... which contributed station preset files \n"
                                  "and tested early and unstable snapshots of KRadio \n"
                                  "with much patience"));
    
	return AboutPageInfo(
	          new KRadioAboutWidget(aboutData, KRadioAboutWidget::AbtAppStandard),
	          "KRadio",
	          "KRadio",
	          "kradio"
		   );
}


// KRadioApp

KRadioApp::KRadioApp()
  : KApplication(),
    PluginManager (I18N_NOOP("KRadio Configuration"), I18N_NOOP("About KRadio Components"))
{
}


KRadioApp::~KRadioApp()
{
}





