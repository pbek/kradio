/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Don Mär  8 21:57:17 CET 2001
    copyright            : (C) 2001, 2002 by Frank Schwanz, Ernst Martin Witte
    email                : schwanz@fh-brandenburg.de, witte@kawo1.rwth-aachen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kwin.h>

#include "kradio.h"

static const char *description =
	I18N_NOOP("KRadio");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE
	
	
static KCmdLineOptions options[] =
{
  { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[])
{
	KAboutData aboutData("kradio", I18N_NOOP("KRadio"),
    					 VERSION, description, KAboutData::License_GPL,
					     "(c) 2002 Martin Witte, Frank Schwanz", 0, "http://sourceforge.net/projects/kradio", "witte@kawo1.rwth-aachen.de");
	aboutData.addAuthor("Martin Witte", "revision, lirc support, docking, alarm function", "witte@kawo1.rwth-aachen.de", "http://sourceforge.net/projects/kradio/");
	aboutData.addAuthor("Frank Schwanz", "idea, first basic application", "schwanz@fh-brandenburg.de");
	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

	KApplication a;
	KRadio *kradio = new KRadio();
	a.setMainWidget(kradio);
	kradio->show();
	kradio->readXOptions();
	int ret = a.exec();
	if (kradio)
		delete kradio;
	return ret;
}
