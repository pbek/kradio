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
#include <klocale.h>
#include "aboutwidget.h"

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
  : KApplication()
{
    m_Instances.setAutoDelete(true);
}


KRadioApp::~KRadioApp()
{
}


void KRadioApp::saveState (KConfig *c) const
{
    c->setGroup("Global");
    c->writeEntry("instances", m_Instances.count());

    int i = 0;
    QDictIterator<PluginManager> it(m_Instances);
    for (; it.current(); ++it, ++i) {
        c->setGroup("Global");
        c->writeEntry("instance_name_" + QString::number(i), it.currentKey());
        it.current()->saveState(c);
    }
}


void KRadioApp::restoreState (KConfig *c)
{
    c->setGroup("Global");

    int n = c->readNumEntry("instances", 1);

    for (int i = 0; i < n; ++i) {
        c->setGroup("Global");
        QString name = c->readEntry("instance_name_" + QString::number(i),
                                    n > 1 ? (i18n("Instance") + " " + QString::number(i+1)) : QString(""));
        createNewInstance(name)->restoreState(c);
    }
}


PluginManager *KRadioApp::createNewInstance(const QString &_name)
{
    QString instance_name = _name;
    QString id = QString::number(m_Instances.count()+1);
    if (instance_name.isNull()) {
        instance_name = " - " + i18n("Instance") + " " + id;
    } else {
        instance_name = instance_name.length() ? (" - " + instance_name) : QString("");
    }
    PluginManager *pm = new PluginManager ( i18n("KRadio Configuration")    + instance_name,
                                            i18n("About KRadio Components") + instance_name
                                          );
    m_Instances.insert(instance_name, pm);

    /* Until we don't have library plugins we must instantiate them hard-wired */
    KRadioAbout      *about       = new KRadioAbout     (      "kradio-about-"     + id);
    LircSupport      *lircsupport = new LircSupport     (      "lirc-"             + id);
    V4LRadio         *v4lradio    = new V4LRadio        (      "v4lradio-"         + id);
    Radio            *radio       = new Radio           (      "radio-"            + id);
    TimeControl      *timecontrol = new TimeControl     (      "timecontrol-"      + id);
    QuickBar         *quickbar    = new QuickBar        (NULL, "quickbar-"         + id);
    RadioDocking     *docking     = new RadioDocking    (      "docking-"          + id);
    docking->show();
    RadioView        *view        = new RadioView       (NULL, "radioview-"        + id);
    Recording        *record      = new Recording       (      "recording-"        + id);
    RecordingMonitor *monitor     = new RecordingMonitor(NULL, "recordingmonitor-" + id);
    ErrorLog         *logger      = new ErrorLog        (NULL, "logger-"           + id);

    pm->insertPlugin(about);
    pm->insertPlugin(logger);
    pm->insertPlugin(lircsupport);
    pm->insertPlugin(radio);
    pm->insertPlugin(timecontrol);
    pm->insertPlugin(quickbar);
    pm->insertPlugin(docking);
    pm->insertPlugin(view);
    pm->insertPlugin(record);
    pm->insertPlugin(monitor);
    pm->insertPlugin(v4lradio);

    return pm;
}

