/***************************************************************************
                          kradioapp.h  -  description
                             -------------------
    begin                : Sa Feb  9 2002
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

#ifndef KRADIO_KRADIOAPP_H
#define KRADIO_KRADIOAPP_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qdict.h>

#include <kapplication.h>
#include <kaboutapplication.h>
#include "pluginmanager.h"
#include "plugins.h"

class KLibrary;

class KRadioAbout : public PluginBase
{
public:
    KRadioAbout(const QString &name) : PluginBase(name, "KRadio Application") {}

    virtual QString pluginClassName() const { return "KRadioAbout"; }

    virtual ConfigPageInfo createConfigurationPage () { return ConfigPageInfo(); }
    virtual AboutPageInfo  createAboutPage ();

    virtual void   saveState    (KConfig *) const {}
    virtual void   restoreState (KConfig *)       {}
};




typedef  PluginBase     *(*t_kradio_plugin_init_func)(const QString & cls, const QString &obj);
typedef  void            (*t_kradio_plugin_info_func)(QMap<QString, QString> &);



struct PluginLibraryInfo {
    KLibrary                  *library;
    QMap<QString,QString>      plugins;
    t_kradio_plugin_init_func  init_func;
    t_kradio_plugin_info_func  info_func;

    PluginLibraryInfo() : library(NULL), init_func(NULL), info_func(NULL) {}
    PluginLibraryInfo(const QString &libname);
    bool valid() { return init_func && info_func && library; }
};


struct PluginClassInfo {
    QString                    class_name;
    QString                    description;
    t_kradio_plugin_init_func  create_function;

    PluginClassInfo() : create_function(NULL) {}
    PluginClassInfo(const QString &_name, const QString &descr, t_kradio_plugin_init_func init_func)
        : class_name(_name), description(descr), create_function(init_func) {}
    PluginBase *CreateInstance(const QString &obj_name) { return create_function ? create_function(class_name, obj_name) : NULL; }
};


class KRadioApp : public KApplication
{
Q_OBJECT
public:
    KRadioApp();
    virtual ~KRadioApp();

    virtual void             saveState    (KConfig *c);
    virtual void             restoreState (KConfig *c);

    virtual PluginManager   *createNewInstance(const QString &name);

    virtual KLibrary   *LoadLibrary (const QString &library);
    virtual void        UnloadLibrary (const QString &library);
    virtual PluginBase *CreatePlugin (PluginManager *manager, const QString &name, const QString &object_name);

    virtual const QMap<QString, PluginLibraryInfo> &getPluginLibraries() const { return m_PluginLibraries; }
    virtual const QMap<QString, PluginClassInfo>   &getPluginClasses()   const { return m_PluginInfos; }

    virtual void  startPlugins();

protected slots:

    virtual void  saveState();
    virtual void  slotAboutToQuit();

protected:

    QDict<PluginManager>        m_Instances;

    QMap<QString, PluginLibraryInfo>  m_PluginLibraries;
    QMap<QString, PluginClassInfo>    m_PluginInfos;

    bool                              m_quitting;
};


#endif
