/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Don Mär  8 21:57:17 CET 2001
    copyright            : (C) 2001-2005 by Ernst Martin Witte, Frank Schwanz
    email                : witte@kawo1.rwth-aachen.de, schwanz@fh-brandenburg.de
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

#include "kradioversion.h"
#include "libkradio/kradioapp.h"


static KCmdLineOptions options[] =
{
  { 0, 0, 0 }
};


int main(int argc, char *argv[])
{
    KAboutData aboutData("kradio", I18N_NOOP("KRadio"),
                         KRADIO_VERSION, "KRadio", KAboutData::License_GPL,
                         "(c) 2002-2005 Martin Witte, Klas Kalass, Frank Schwanz",
                         0,
                         "http://sourceforge.net/projects/kradio",
                         0);
    aboutData.addAuthor("Martin Witte",  I18N_NOOP("rewrite for 0.3.0, recording, lirc support, alarms, misc"), "witte@kawo1.rwth-aachen.de");
    aboutData.addAuthor("Marcus Camen",  I18N_NOOP("Buildsystem, Standards Conformance, Cleanups"), "mcamen@mcamen.de");
    aboutData.addAuthor("Klas Kalass",   I18N_NOOP("Miscellaneous"), "klas.kalass@gmx.de");
    aboutData.addAuthor("Frank Schwanz", I18N_NOOP("idea, first basic application"), "schwanz@fh-brandenburg.de");

    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

    KRadioApp a;

    a.restoreState(KGlobal::config());

    int ret = a.exec();
    a.saveState(KGlobal::config());
    return ret;
}

