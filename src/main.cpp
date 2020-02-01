/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Don Mar  8 21:57:17 CET 2001
    copyright            : (C) 2001-2020 by Ernst Martin Witte, Frank Schwanz
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

#include <QtCore/QCommandLineParser>
#include <QtWidgets/QApplication>

#include <KAboutData>
#include <KSharedConfig>
#include <KLocalizedString>

#include "instancemanager.h"

#include "debug-profiler.h"

int main(int argc, char *argv[])
{

    QApplication         app(argc, argv);

    KAboutData           aboutData(QStringLiteral("kradio5"),
				   i18n("KRadio5"),
				   QStringLiteral(KRADIO_VERSION),
				   i18n("A versatile KDE AM/FM/Internet radio application"),
				   KAboutLicense::LicenseKey::GPL,
				   i18n("(c) 2002-2020 Martin Witte, Klas Kalass, Frank Schwanz"),
				   NULL,
				   "http://kradio.sourceforge.net",
				   NULL
				   );
    
    aboutData.addAuthor(i18n("Ernst Martin Witte"),  i18n("KDE4+KF5 ports, RDS support, shortcuts, PVR support, rewrite for 1.0/0.3.0, recording, LIRC support, alarms, misc"), "emw@nocabal.de", "http://www.nocabal.de/~emw");
    aboutData.addAuthor(i18n("Pino Toscano"),  i18n("MPRIS plugin, buildsystem, standards conformance, cleanups"), "toscano.pino@tiscali.it");
    aboutData.addAuthor(i18n("Marcus Camen"),  i18n("Buildsystem, standards conformance, cleanups"), "mcamen@mcamen.de");
    aboutData.addAuthor(i18n("Klas Kalass"),   i18n("Miscellaneous"), "klas.kalass@gmx.de");
    aboutData.addAuthor(i18n("Frank Schwanz"), i18n("Initial idea, first basic KDE 2 application"), "schwanz@fh-brandenburg.de");

    KAboutData::setApplicationData(aboutData);


    
    QCommandLineParser   cmdlineParser;
    cmdlineParser.setApplicationDescription("A versatile KDE AM/FM/Internet radio application");
    cmdlineParser.addHelpOption();
    cmdlineParser.addVersionOption();

    cmdlineParser.process(app);
    
    QApplication::setQuitOnLastWindowClosed(false);

    IErrorLogClient::staticLogInfo(QString("KRadio5 Version %1 is starting").arg(KRADIO_VERSION));

    BlockProfiler  profiler_manager("main::InstanceManager");

    InstanceManager manager;

    profiler_manager.stop();

    BlockProfiler  profiler_restore("main::restore");

    const auto cfgPtr = KSharedConfig::openConfig();
    manager.restoreState(cfgPtr.data());

    if (!manager.quitting()) {
        manager.startPlugins();
    }
    profiler_restore.stop();

    int ret = -1;
    if (!manager.quitting()) {
        ret = app.exec();
    }
#ifdef KRADIO_ENABLE_PROFILERS
//     global_time_profiler.printData();
//     global_mem_profiler.printData();
#endif

    IErrorLogClient::staticLogDebug("normal shutdown");

    return ret;
}

