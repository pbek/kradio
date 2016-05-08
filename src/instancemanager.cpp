/**************************************************************************
                          instancemanager.cpp  -  description
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

#include <kmessagebox.h>
#include <kaboutdata.h>
#include <klocalizedstring.h>
#include <kpluginloader.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include <QCoreApplication>

#include "instancemanager.h"
#include "errorlog_interfaces.h"
#include "pluginmanager.h"

#include "debug-profiler.h"

/////////////////////////////////////////////////////////////////////////////
//// PluginLibraryInfo

PluginLibraryInfo::PluginLibraryInfo (const PluginLibraryInfo &libinfo)
 : factory(libinfo.factory),
   plugins(libinfo.plugins),
   errorString(libinfo.errorString)
{
}


PluginLibraryInfo &PluginLibraryInfo::operator = (const PluginLibraryInfo &libinfo)
 {
    factory = libinfo.factory;
    plugins = libinfo.plugins;
    errorString = libinfo.errorString;
    return *this;
}


PluginLibraryInfo::PluginLibraryInfo (const QString &lib_name)
 : factory(NULL)
{
    KPluginLoader loader(lib_name);
    KPluginFactory* ff = loader.factory();
    if (ff) {
        factory = QSharedPointer<KRadioPluginFactoryBase>(ff->create<KRadioPluginFactoryBase>());
        if (factory) {
            foreach (const KAboutData &about, factory->components()) {
                KGlobal::locale()->insertCatalog(about.catalogName());
                plugins.insert(about.appName(), about.shortDescription());
            }
        } else {
            KMessageBox::error(NULL,
                               i18n("Library %1: plugin entry point is missing", lib_name),
                               i18n("Plugin Library Load Error"));
            loader.unload();
        }
    } else {
        errorString = loader.errorString();
            KMessageBox::error(NULL,
                               i18n("Library %1:\n%2", lib_name, errorString),
                               i18n("Plugin Library Load Error"));
    }
}


/////////////////////////////////////////////////////////////////////////////
//// InstanceManager

InstanceManager::InstanceManager()
  : QObject(),
    m_quitting(false)
{
    //m_Instances.setAutoDelete(true);
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(slotAboutToQuit()));
}


InstanceManager::~InstanceManager()
{
    IErrorLogClient::staticLogDebug("InstanceManager::~InstanceManager()");
    qDeleteAll(m_Instances);
}

void InstanceManager::saveState()
{
    IErrorLogClient::staticLogDebug("InstanceManager::saveState()");
    saveState(&*KGlobal::config());
}

void InstanceManager::saveState (KConfig *c)
{
    KConfigGroup global_group = c->group("Global");
    KConfigGroup plugin_group = c->group("Plugin Libraries");

    global_group.writeEntry("instances", m_Instances.count());

    int i = 0;
    QMap<QString, PluginManager*>::const_iterator it1 = m_Instances.constBegin();
    for (; it1 != m_Instances.constEnd(); ++it1, ++i) {
        global_group.writeEntry("instance_name_" + QString::number(i), it1.key());
        it1.value()->saveState(c);
    }

    plugin_group.writeEntry("count", m_PluginLibraries.count());
    int idx = 0;
    QMap<QString, PluginLibraryInfo>::const_iterator it2 = m_PluginLibraries.constBegin();
    for (; it2 != m_PluginLibraries.constEnd(); ++it2, ++idx) {
        plugin_group.writeEntry("library_" + QString::number(idx), it2.key());
    }

    c->sync();
}


void InstanceManager::restoreState (KConfig *c)
{
    BlockProfiler profiler("InstanceManager::restoreState - loadLibraries");

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
                                                i18n("New Plugin Libraries Found"),
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

    // minimum number of plugin libs is around 5: radio mux, snd srv, snd card, radio dev, stations
    if (m_PluginLibraries.size() < 5) {
        KMessageBox::error(NULL,
                           i18np("Found only %1 library. Expected a minimum of 5. Please check "
                                 "the KRadio4 installation directory and the environment variables "
                                 "KDEDIR, KDEDIRS and KDEHOME.\n",
                                 "Found only %1 libraries. Expected a minimum of 5. Please check "
                                 "the KRadio4 installation directory and the environment variables "
                                 "KDEDIR, KDEDIRS and KDEHOME.\n",
                                 m_PluginLibraries.size()
                                 ),
                           i18n("KRadio4 Installation Error"));
        m_quitting = true;
        return;
    }

//     if (n_libs < 6) {    // this seems to be a meaningful minimum value for a working kradio setup
//         QList<QString>::iterator end = libs.end();
//         int idx = 0;
//         for (QList<QString>::iterator it = libs.begin(); it != end; ++it, ++idx) {
//             LoadLibrary(*it);
//         }
//     }


    profiler.stop();


    BlockProfiler rest_profiler("InstanceManager::restoreState - restore");

    int n = global_group.readEntry("instances", 1);
    if (n < 1 || n > 10)
        n = 1;

    for (int i = 0; i < n; ++i) {
        QString name = global_group.readEntry("instance_name_" + QString::number(i),
                                              n > 1 ? i18n("Instance %1", i+1) : QString(""));
        createNewInstance(name)->restoreState(c);
    }
}


PluginManager *InstanceManager::createNewInstance(const QString &_name)
{
    BlockProfiler profiler("InstanceManager::createNewInstance");

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
                                            i18n("KRadio Configuration")    + title_ext
                                          );

    m_Instances.insert(instance_name, pm);

    return pm;
}


void InstanceManager::LoadLibrary (const QString &library)
{
    BlockProfiler profiler("InstanceManager::LoadLibrary");
    BlockProfiler libprofiler("InstanceManager::LoadLibrary - " + library.toLatin1());

    PluginLibraryInfo libinfo(library);
    if (libinfo.valid()) {
        m_PluginLibraries.insert(library, libinfo);
        QMap<QString,QString>::const_iterator end = libinfo.plugins.constEnd();
        for (QMap<QString,QString>::const_iterator it = libinfo.plugins.constBegin(); it != end; ++it) {
            m_PluginInfos.insert(it.key(), PluginClassInfo (it.key(), *it, libinfo.factory));
        }
    } else {
        kDebug()  << QDateTime::currentDateTime().toString(Qt::ISODate)
                  << " Error: Loading Library" << library << "failed:" << libinfo.errorString;
    }

    foreach (PluginManager *manager, m_Instances) {
        manager->noticeLibrariesChanged();
    }

    //return libinfo.valid() ? libinfo.library : NULL;
}


void InstanceManager::UnloadLibrary (const QString &library)
{
    if (!m_PluginLibraries.contains(library))
        return;

    PluginLibraryInfo info = m_PluginLibraries[library];

    QMap<QString, QString>::const_iterator end_classes = info.plugins.constEnd();
    for (QMap<QString, QString>::const_iterator it_classes = info.plugins.constBegin(); it_classes != end_classes; ++it_classes) {
        foreach (PluginManager *manager, m_Instances) {
            manager->unloadPlugins(it_classes.key());
        }
        m_PluginInfos.remove(it_classes.key());
    }
    m_PluginLibraries.remove(library);

    foreach (PluginManager *manager, m_Instances) {
        manager->noticeLibrariesChanged();
    }
}


PluginBase *InstanceManager::CreatePlugin (PluginManager *manager, const QString &instanceID, const QString &class_name, const QString &object_name)
{
    BlockProfiler all_profiler  ("InstanceManager::CreatePlugin");
    BlockProfiler class_profiler("InstanceManager::CreatePlugin - " + class_name.toLatin1());

    BlockProfiler create_profiler("InstanceManager::CreatePlugin - create");

    PluginBase *retval = NULL;
    if (m_PluginInfos.contains(class_name)) {
        retval = m_PluginInfos[class_name].CreateInstance(instanceID, object_name);
        if (!retval) {
            kDebug() << QDateTime::currentDateTime().toString(Qt::ISODate)
                     << " Error: Creation of instance" << object_name << "of class" << class_name << "failed.";
        }
    } else {
        kDebug()     << QDateTime::currentDateTime().toString(Qt::ISODate)
                     << " Error: Cannot create instance" << object_name << "of unknown class" << class_name << ".";
    }

    create_profiler.stop();

    if (retval) {

        BlockProfiler insert_profiler("InstanceManager::CreatePlugin - insert");
        manager->insertPlugin(retval);
        insert_profiler.stop();

        //BlockProfiler restore_profiler("InstanceManager::CreatePlugin - restore");
        //retval->restoreState(KGlobal::config());
    }

    return retval;
}

void  InstanceManager::startPlugins()
{
    foreach (PluginManager *manager, m_Instances) {
        manager->startPlugins();
    }
}

void  InstanceManager::slotAboutToQuit()
{
    IErrorLogClient::staticLogDebug("slotAboutToQuit");
    if (!m_quitting) {
        IErrorLogClient::staticLogDebug("slotAboutToQuit, m_quitting = false");
        m_quitting = true;
        saveState();
        foreach (PluginManager *manager, m_Instances) {
            manager->aboutToQuit();
        }
        m_quitting = false;
    }
}

#include "instancemanager.moc"
