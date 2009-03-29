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


#include <kmessagebox.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <klibloader.h>
#include <kconfig.h>
#include <ksharedconfig.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

// #include <kprogress.h>

#include "kradioapp.h"
//#include "aboutwidget.h"
#include "kradioaboutwidget.h"
#include "errorlog_interfaces.h"

#include "debug-profiler.h"

/////////////////////////////////////////////////////////////////////////////
//// KRadioAbout

// AboutPageInfo  KRadioAbout::createAboutPage ()
// {
//     const char *description = I18N_NOOP(
//         "KRadio - The Radio Application for KDE"
//         "<P>"
//         "With KRadio you can listen to radio broadcasts with the help of your "
//         "V4L/V4L2 compatible radio card."
//         "<P>"
//         "The KRadio Project contains a station preset data database. To complete "
//         "this database you are encouraged to contribute your station preset file "
//         "to the project. Just send it to one of the authors. "
//         "<P>"
//         "If you like to contribute your ideas, your own plugins or translations, "
//         "don't hesitate to contact one of the authors."
//         "<P>"
//     );
//
//     KAboutData aboutData("kradio", // appname
//                          "kradio4", // catalog name
//                          ki18n("KRadio4"), // programName
//                          KRADIO_VERSION,
//                          ki18n(description),
//                          KAboutData::License_GPL,
//                          ki18n("(c) 2002-2006 Martin Witte, Klas Kalass"),
//                          KLocalizedString(),
//                          "http://sourceforge.net/projects/kradio"
//                         );
//     aboutData.addAuthor(ki18n("Martin Witte"),  ki18n("Preset Database, Remote Control Support, Alarms, Rewrite for KRadio 0.3.0, Misc"), "emw-kradio@nocabal.de");
//     aboutData.addAuthor(ki18n("Marcus Camen"),  ki18n("Buildsystem, Standards Conformance, Cleanups"),                                    "mcamen@mcamen.de");
//     aboutData.addAuthor(ki18n("Klas Kalass"),   ki18n("Miscellaneous"),                                                                   "klas.kalass@gmx.de");
//     aboutData.addAuthor(ki18n("Frank Schwanz"), ki18n("idea, first basic application"),                                                   "schwanz@fh-brandenburg.de");
//
//     aboutData.addCredit(ki18n("Many People around the World ... "),
//                         ki18n("... which contributed station preset files \n"
//                               "and tested early and unstable snapshots of KRadio \n"
//                               "with much patience"));
//
// #warning "FIXME: about widgets currently disabled"
//     return AboutPageInfo(
//               new KRadioAboutWidget(&aboutData),
//               "KRadio",
//               "KRadio",
//               "kradio4"
//            );
// }


/////////////////////////////////////////////////////////////////////////////
//// PluginLibraryInfo

PluginLibraryInfo::PluginLibraryInfo (const PluginLibraryInfo &libinfo)
 : library(libinfo.library.fileName()),
   init_func(libinfo.init_func),
   info_func(libinfo.info_func),
   libload_func(libinfo.libload_func),
   libunload_func(libinfo.libunload_func)
{
}


PluginLibraryInfo &PluginLibraryInfo::operator = (const PluginLibraryInfo &libinfo)
 {
    library.setFileName(libinfo.library.fileName());
    init_func      = libinfo.init_func;
    info_func      = libinfo.info_func;
    libload_func   = libinfo.libload_func;
    libunload_func = libinfo.libunload_func;
    return *this;
}


PluginLibraryInfo::PluginLibraryInfo (const QString &lib_name)
 : library(lib_name),
   init_func(NULL),
   info_func(NULL),
   libload_func(NULL),
   libunload_func(NULL)
{
    if (library.load()) {
        info_func      = (t_kradio_plugin_info_func)     library.resolveSymbol("KRadioPlugin_GetAvailablePlugins");
        init_func      = (t_kradio_plugin_init_func)     library.resolveSymbol("KRadioPlugin_CreatePlugin");
        libload_func   = (t_kradio_plugin_libload_func)  library.resolveSymbol("KRadioPlugin_LoadLibrary");
        libunload_func = (t_kradio_plugin_libunload_func)library.resolveSymbol("KRadioPlugin_UnloadLibrary");
        if (info_func && init_func && libload_func && libunload_func) {
            libload_func();
            info_func(plugins);
        } else {
            KMessageBox::error(NULL,
                               i18n("Library %1: Plugin Entry Point is missing\n", lib_name),
                               i18n("Plugin Library Load Error"));
            library.unload();
            info_func = NULL;
            init_func = NULL;
        }
    } else {
            KMessageBox::error(NULL,
                               i18n("Library %1: \n%2", lib_name,  library.errorString()),
                               i18n("Plugin Library Load Error"));
    }
}


/////////////////////////////////////////////////////////////////////////////
//// KRadioApp

KRadioApp::KRadioApp()
  : KApplication(),
    m_quitting(false)
{
    setQuitOnLastWindowClosed(false);
    //m_Instances.setAutoDelete(true);
    connect(this, SIGNAL(aboutToQuit()), this, SLOT(slotAboutToQuit()));
}


KRadioApp::~KRadioApp()
{
    IErrorLogClient::staticLogDebug("KRadioApp::~KRadioApp()");
}

void KRadioApp::saveState()
{
    IErrorLogClient::staticLogDebug(i18n("saveState"));
    saveState(&*KGlobal::config());
}

void KRadioApp::saveState (KConfig *c)
{
    KConfigGroup global_group = c->group("Global");
    KConfigGroup plugin_group = c->group("Plugin Libraries");

    global_group.writeEntry("instances", m_Instances.count());

    int i = 0;
    QMap<QString, PluginManager*>::const_iterator it1 = m_Instances.begin();
    for (; it1 != m_Instances.end(); ++it1, ++i) {
        global_group.writeEntry("instance_name_" + QString::number(i), it1.key());
        it1.value()->saveState(c);
    }

    plugin_group.writeEntry("count", m_PluginLibraries.count());
    int idx = 0;
    QMap<QString, PluginLibraryInfo>::const_iterator it2 = m_PluginLibraries.begin();
    for (; it2 != m_PluginLibraries.end(); ++it2, ++idx) {
        plugin_group.writeEntry("library_" + QString::number(idx), it2.key());
    }

    c->sync();
}


void KRadioApp::restoreState (KConfig *c)
{
    BlockProfiler profiler("KRadioApp::restoreState - loadLibraries");

    KConfigGroup global_group = c->group("Global");
    KConfigGroup plugin_group = c->group("Plugin Libraries");

    QStringList new_libs = KGlobal::dirs()->findAllResources("lib", "kradio4/plugins/*.so");

    int n_libs = plugin_group.readEntry("count", 0);

    for (int idx = 0; idx < n_libs; ++idx) {
        QString lib = plugin_group.readEntry("library_" + QString::number(idx), QString());
        if (lib.length()) {
            new_libs.removeAll(lib);
            LoadLibrary(lib);
        }
    }

    if (new_libs.size() > 0) {
        int action = KMessageBox::questionYesNoList(NULL,
                                                i18n("New plugin libraries found. Should the libraries be loaded now?"),
                                                new_libs,
                                                i18n("New Plugin Libraries found"),
                                                KStandardGuiItem::yes(),
                                                KStandardGuiItem::no(),
                                                "autoload_plugins");

        if (action == KMessageBox::Yes) {
            QString lib;
            foreach(lib, new_libs) {
                LoadLibrary(lib);
            }
        }
    }

//     if (n_libs < 6) {    // this seems to be a meaningful minimum value for a working kradio setup
//         QList<QString>::iterator end = libs.end();
//         int idx = 0;
//         for (QList<QString>::iterator it = libs.begin(); it != end; ++it, ++idx) {
//             LoadLibrary(*it);
//         }
//     }


    profiler.stop();


    BlockProfiler rest_profiler("KRadioApp::restoreState - restore");

    int n = global_group.readEntry("instances", 1);
    if (n < 1 || n > 10)
        n = 1;

    for (int i = 0; i < n; ++i) {
        QString name = global_group.readEntry("instance_name_" + QString::number(i),
                                              n > 1 ? (i18n("Instance") + " " + QString::number(i+1)) : QString(""));
        createNewInstance(name)->restoreState(c);
    }
}


PluginManager *KRadioApp::createNewInstance(const QString &_name)
{
    BlockProfiler profiler("KRadioApp::createNewInstance");

    QString instance_name = _name;
    QString title_ext = "";
    QString id = QString::number(m_Instances.count()+1);
    if (instance_name.length() == 0) {
        instance_name = "Instance " + id;
    }
    if (_name.length() && m_Instances.count() > 0) {
        title_ext = " " + instance_name;
    }
    PluginManager *pm = new PluginManager ( instance_name,
                                            this,
                                            i18n("KRadio Configuration")    + title_ext,
                                            i18n("About KRadio Components") + title_ext
                                          );

    m_Instances.insert(instance_name, pm);

    /* Until we don't have library plugins we must instantiate them hard-wired */
//     KRadioAbout      *about       = new KRadioAbout     (      "kradio-about-" + instance_name);
//     pm->insertPlugin(about);

    return pm;
}


void KRadioApp::LoadLibrary (const QString &library)
{
    BlockProfiler profiler("KRadioApp::LoadLibrary");
    BlockProfiler libprofiler("KRadioApp::LoadLibrary - " + library);

    PluginLibraryInfo libinfo(library);
    if (libinfo.valid()) {
        m_PluginLibraries.insert(library, libinfo);
        QMap<QString,QString>::const_iterator end = libinfo.plugins.end();
        for (QMap<QString,QString>::const_iterator it = libinfo.plugins.begin(); it != end; ++it) {
            m_PluginInfos.insert(it.key(), PluginClassInfo (it.key(), *it, libinfo.init_func));
        }
    } else {
        kDebug()  << QDateTime::currentDateTime().toString(Qt::ISODate)
                  << " "
                  << i18n("Error: Loading Library %1 failed: %2", library, libinfo.library.errorString())
                  << endl;
    }

    for (QMap<QString, PluginManager*>::iterator it_managers = m_Instances.begin(); it_managers != m_Instances.end(); ++it_managers) {
        (*it_managers)->noticeLibrariesChanged();
    }

    //return libinfo.valid() ? libinfo.library : NULL;
}


void KRadioApp::UnloadLibrary (const QString &library)
{
    if (!m_PluginLibraries.contains(library))
        return;

    PluginLibraryInfo info = m_PluginLibraries[library];

    QMap<QString, QString>::const_iterator end_classes = info.plugins.end();
    for (QMap<QString, QString>::const_iterator it_classes = info.plugins.begin(); it_classes != end_classes; ++it_classes) {
        for (QMap<QString, PluginManager*>::iterator it_managers = m_Instances.begin(); it_managers != m_Instances.end(); ++it_managers) {
            it_managers.value()->unloadPlugins(it_classes.key());
        }
        m_PluginInfos.remove(it_classes.key());
    }
    m_PluginLibraries.remove(library);

    info.libunload_func();
    info.library.unload();

    for (QMap<QString, PluginManager*>::iterator it_managers = m_Instances.begin(); it_managers != m_Instances.end(); ++it_managers) {
        it_managers.value()->noticeLibrariesChanged();
    }
}


PluginBase *KRadioApp::CreatePlugin (PluginManager *manager, const QString &instanceID, const QString &class_name, const QString &object_name)
{
    BlockProfiler all_profiler  ("KRadioApp::CreatePlugin");
    BlockProfiler class_profiler("KRadioApp::CreatePlugin - " + class_name);

    BlockProfiler create_profiler("KRadioApp::CreatePlugin - create");

    PluginBase *retval = NULL;
    if (m_PluginInfos.contains(class_name)) {
        retval = m_PluginInfos[class_name].CreateInstance(instanceID, object_name);
        if (!retval) {
            kDebug() << QDateTime::currentDateTime().toString(Qt::ISODate)
                     << " "
                     << i18n("Error: Creation of instance \"%1\" of class %2 falied.", object_name, class_name)
                     << endl;
        }
    } else {
        kDebug()     << QDateTime::currentDateTime().toString(Qt::ISODate)
                     << " "
                     << i18n("Error: Cannot create instance \"%1\" of unknown class %2.", object_name, class_name)
                     << endl;
    }

    create_profiler.stop();

    if (retval) {

        BlockProfiler insert_profiler("KRadioApp::CreatePlugin - insert");
        manager->insertPlugin(retval);
        insert_profiler.stop();

        //BlockProfiler restore_profiler("KRadioApp::CreatePlugin - restore");
        //retval->restoreState(KGlobal::config());
    }

    return retval;
}

void  KRadioApp::startPlugins()
{
    QMap<QString, PluginManager*>::iterator it = m_Instances.begin();
    for (; it != m_Instances.end(); ++it) {
        it.value()->startPlugins();
    }
}

void  KRadioApp::slotAboutToQuit()
{
    IErrorLogClient::staticLogDebug("slotAboutToQuit");
    if (!m_quitting) {
        IErrorLogClient::staticLogDebug("slotAboutToQuit, m_quitting = false");
        m_quitting = true;
        saveState();
        QMap<QString, PluginManager*>::iterator it = m_Instances.begin();
        for (; it != m_Instances.end(); ++it) {
            it.value()->aboutToQuit();
        }
        m_quitting = false;
    }
}

#include "kradioapp.moc"