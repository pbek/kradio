/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Don Mär  8 21:57:17 CET 2001
    copyright            : (C) 2001, 2002, 2003 by Ernst Martin Witte, Frank Schwanz
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

#include "kradioapp.h"

#include "v4lradio.h"
#include "radio.h"
#include "timecontrol.h"
#include "lircsupport.h"
#include "quickbar.h"
#include "docking.h"
#include "radioview.h"
#include "recording.h"
#include "recording-monitor.h"
#include "errorlog.h"

static KCmdLineOptions options[] =
{
  { 0, 0, 0 }
};


int main(int argc, char *argv[])
{
    KAboutData aboutData("kradio", I18N_NOOP("KRadio"),
                         VERSION, "KRadio", KAboutData::License_GPL,
                         "(c) 2002, 2003 Martin Witte, Klas Kalass, Frank Schwanz",
                         0,
                         "http://sourceforge.net/projects/kradio",
                         0);
    aboutData.addAuthor("Martin Witte",  I18N_NOOP("rewrite for 0.3.0, recording, lirc support, alarms, misc"), "witte@kawo1.rwth-aachen.de");
    aboutData.addAuthor("Klas Kalass",   I18N_NOOP("Miscellaneous"), "klas.kalass@gmx.de");
    aboutData.addAuthor("Frank Schwanz", I18N_NOOP("idea, first basic application"), "schwanz@fh-brandenburg.de");

    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

    KRadioApp a;

	/* Until we don't have library plugins we must instantiate them hard-wired */

    KRadioAbout      *about       = new KRadioAbout     (      "kradio-about-1");
    LircSupport      *lircsupport = new LircSupport     (      "lirc-1");
    V4LRadio         *v4lradio    = new V4LRadio        (      "v4lradio-1");
    Radio            *radio       = new Radio           (      "radio-1");
    TimeControl      *timecontrol = new TimeControl     (      "timecontrol-1");
    QuickBar         *quickbar    = new QuickBar        (NULL, "quickbar-1");
    RadioDocking     *docking     = new RadioDocking    (      "docking-1");
    docking->show();
    RadioView        *view        = new RadioView       (NULL, "radioview-1");
    Recording        *record      = new Recording       (      "recording-1");
    RecordingMonitor *monitor     = new RecordingMonitor(NULL, "recordingmonitor-1");
    ErrorLog         *logger      = new ErrorLog        (NULL, "logger-1");
    
    a.insertPlugin(about);
	a.insertPlugin(logger);
    a.insertPlugin(lircsupport);
    a.insertPlugin(v4lradio);
    a.insertPlugin(radio);
    a.insertPlugin(timecontrol);
    a.insertPlugin(quickbar);
    a.insertPlugin(docking);
    a.insertPlugin(view);
	a.insertPlugin(record);
	a.insertPlugin(monitor);

    a.restoreState(KGlobal::config());   
    int ret = a.exec();
    a.saveState(KGlobal::config());

    return ret;
}

