/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Don Mär  8 21:57:17 CET 2001
    copyright            : (C) 2001-2005 by Ernst Martin Witte, Frank Schwanz
    email                : emw-kradio@nocabal.de, schwanz@fh-brandenburg.de
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

#include "kradioapp.h"

#include "debug-profiler.h"

static KCmdLineOptions options;

int main(int argc, char *argv[])
{
    IErrorLogClient::staticLogInfo(QString("KRadio4 Version %1 is starting").arg(KRADIO_VERSION));

    KAboutData aboutData("kradio4",
                         "kradio4",
                         ki18n("KRadio4"),
                         KRADIO_VERSION,
                         ki18n("A versatile KDE AM/FM/Internet radio application"),
                         KAboutData::License_GPL,
                         ki18n("(c) 2002-2009 Martin Witte, Klas Kalass, Frank Schwanz"),
                         ki18n(NULL),
                         "http://kradio.sourceforge.net",
                         NULL
                        );
    aboutData.addAuthor(ki18n("Ernst Martin Witte"),  ki18n("KDE4 port, RDS support, shortcuts, PVR support, rewrite for 1.0/0.3.0, recording, lirc support, alarms, misc"), "emw@nocabal.de", "http://www.nocabal.de/~emw");
    aboutData.addAuthor(ki18n("Marcus Camen"),  ki18n("Buildsystem, Standards Conformance, Cleanups"), "mcamen@mcamen.de");
    aboutData.addAuthor(ki18n("Klas Kalass"),   ki18n("Miscellaneous"), "klas.kalass@gmx.de");
    aboutData.addAuthor(ki18n("Frank Schwanz"), ki18n("initial idea, first basic KDE 2 application"), "schwanz@fh-brandenburg.de");

    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

    BlockProfiler  profiler_kradioapp("main::KRadioApp");


    KRadioApp a;

    profiler_kradioapp.stop();

    BlockProfiler  profiler_restore("main::restore");

    a.restoreState(KGlobal::config().data());
    a.startPlugins();

    profiler_restore.stop();

    int ret = a.exec();

    global_time_profiler.printData();
    global_mem_profiler.printData();

    IErrorLogClient::staticLogDebug("normal shutdown");

    return ret;
}

