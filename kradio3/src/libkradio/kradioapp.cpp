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


#include <kaboutdata.h>
#include <klocale.h>
#include <klibloader.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

#include "kradioapp.h"
#include "../libkradio-gui/aboutwidget.h"

// #include "v4lradio.h"
// #include "radio.h"
// #include "timecontrol.h"
// #include "lircsupport.h"
// #include "quickbar.h"
// #include "docking.h"
// #include "radioview.h"
// #include "recording.h"
// #include "recording-monitor.h"
// #include "errorlog.h"

/////////////////////////////////////////////////////////////////////////////
//// KRadioAbout

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


/////////////////////////////////////////////////////////////////////////////
//// PluginLibraryInfo

PluginLibraryInfo::PluginLibraryInfo (const QString &lib_name)
 : library (NULL),
   init_func(NULL),
   info_func(NULL)
{
    library = KLibLoader::self()->library(lib_name.ascii());
    if (library) {
        info_func = (t_kradio_plugin_info_func)library->symbol("getAvailablePlugins");
        init_func = (t_kradio_plugin_init_func)library->symbol("createPlugin");
        if (info_func && init_func) {
            info_func(plugins);
        } else {
            KMessageBox::error(NULL,
                               i18n("Library %1: Plugin Entry Point is missing\n")
                                .arg(lib_name),
                               "Plugin Library Load Error");
            library->unload();
            info_func = NULL;
            init_func = NULL;
            library   = NULL;
        }
    } else {
            KMessageBox::error(NULL,
                               i18n("Library %1: \n%2")
                                .arg(lib_name)
                                .arg(KLibLoader::self()->lastErrorMessage()),
                               "Plugin Library Load Error");
    }
}


/////////////////////////////////////////////////////////////////////////////
//// KRadioApp

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

    c->setGroup("Plugin Libraries");
    c->writeEntry("count", m_PluginLibraries.count());
    int idx = 0;
    for (QMapConstIterator<QString, PluginLibraryInfo> it = m_PluginLibraries.begin(); it != m_PluginLibraries.end(); ++it, ++idx) {
        c->writeEntry("library_" + QString::number(idx), it.key());
    }

    c->sync();
}


void KRadioApp::restoreState (KConfig *c)
{
    c->setGroup("Plugin Libraries");
    int n_libs = c->readNumEntry("count", 0);
    for (int idx = 0; idx < n_libs; ++idx) {
        QString lib = c->readEntry("library_" + QString::number(idx), QString::null);
        if (lib.length()) {
            LoadLibrary(lib);
        }
    }

    if (n_libs == 0) {
        QStringList libs
            = KGlobal::dirs()->findAllResources("lib", "kradio/plugins/*.la");
        for (QValueListIterator<QString> it=libs.begin(); it != libs.end(); ++it)
            LoadLibrary(*it);
    }

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
    if (instance_name.length() == 0) {
        instance_name = "Instance " + id;
    }
    PluginManager *pm = new PluginManager ( instance_name,
                                            this,
                                            i18n("KRadio Configuration")    + (_name.length() ? instance_name : ""),
                                            i18n("About KRadio Components") + (_name.length() ? instance_name : "")
                                          );

    m_Instances.insert(instance_name, pm);

    /* Until we don't have library plugins we must instantiate them hard-wired */
    KRadioAbout      *about       = new KRadioAbout     (      "kradio-about-" + instance_name);
    pm->insertPlugin(about);

    return pm;
}


KLibrary *KRadioApp::LoadLibrary (const QString &library)
{
    PluginLibraryInfo libinfo(library);
    if (libinfo.valid()) {
        m_PluginLibraries.insert(library, libinfo);
        for (QMapConstIterator<QString,QString> it = libinfo.plugins.begin(); it != libinfo.plugins.end(); ++it) {
            m_PluginInfos.insert(it.key(), PluginClassInfo (it.key(), *it, libinfo.init_func));
        }
    } else {
        kdDebug() << QDateTime::currentDateTime().toString(Qt::ISODate)
                  << " "
                  << i18n("Error: Loading Library %1 failed: %2")
                     .arg(library).arg(KLibLoader::self()->lastErrorMessage())
                  << endl;
    }

    for (QDictIterator<PluginManager> it_managers(m_Instances); it_managers.current(); ++it_managers) {
        it_managers.current()->noticeLibrariesChanged();
    }

    return libinfo.valid() ? libinfo.library : NULL;
}


void KRadioApp::UnloadLibrary (const QString &library)
{
    if (!m_PluginLibraries.contains(library))
        return;

    PluginLibraryInfo info = m_PluginLibraries[library];

    for (QMapConstIterator<QString, QString> it_classes = info.plugins.begin(); it_classes != info.plugins.end(); ++it_classes) {
        for (QDictIterator<PluginManager> it_managers(m_Instances); it_managers.current(); ++it_managers) {
            it_managers.current()->unloadPlugins(it_classes.key());
        }
        m_PluginInfos.remove(it_classes.key());
    }
    m_PluginLibraries.remove(library);
    info.library->unload();

    for (QDictIterator<PluginManager> it_managers(m_Instances); it_managers.current(); ++it_managers) {
        it_managers.current()->noticeLibrariesChanged();
    }
}


PluginBase *KRadioApp::CreatePlugin (PluginManager *manager, const QString &class_name, const QString &object_name)
{
    PluginBase *retval = NULL;
    if (m_PluginInfos.contains(class_name)) {
        retval = m_PluginInfos[class_name].CreateInstance(object_name);
        if (!retval) {
            kdDebug() << QDateTime::currentDateTime().toString(Qt::ISODate)
                    << " "
                    << i18n("Error: Creation of instance \"%1\" of class %2 falied.").arg(object_name).arg(class_name)
                    << endl;
        }
    } else {
        kdDebug() << QDateTime::currentDateTime().toString(Qt::ISODate)
                  << " "
                  << i18n("Error: Cannot create instance \"%1\" of unknown class %2.").arg(object_name).arg(class_name)
                  << endl;
    }
    if (retval) {
        manager->insertPlugin(retval);
        retval->restoreState(KGlobal::config());
    }
    return retval;
}



#include "kradioapp.moc"
