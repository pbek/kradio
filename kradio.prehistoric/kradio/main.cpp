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

#include "kradioapp.h"

static const char *description = "KRadio";


static KCmdLineOptions options[] =
{
  { 0, 0, 0 }
};

int main(int argc, char *argv[])
{
  KAboutData aboutData("kradio", I18N_NOOP("KRadio"),
                      VERSION, description, KAboutData::License_GPL,
                      "(c) 2002 Martin Witte, Frank Schwanz, Klas Kalass",
                      0,
                      "http://sourceforge.net/projects/kradio",
                      0);
  aboutData.addAuthor("Martin Witte",  I18N_NOOP("misc, lirc support, alarm function"), "witte@kawo1.rwth-aachen.de");
  aboutData.addAuthor("Klas Kalass",   I18N_NOOP("Miscellaneous"), "klas.kalass@gmx.de");
  aboutData.addAuthor("Frank Schwanz", I18N_NOOP("idea, first basic application"), "schwanz@fh-brandenburg.de");

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KRadioApp a;

  return a.exec();
}
