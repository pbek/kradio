/***************************************************************************
                          instancemanager.h  -  description
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

#ifndef KRADIO_INSTANCEMANAGER_H
#define KRADIO_INSTANCEMANAGER_H

#include <QMap>
#include <QSharedPointer>
#include <QObject>

#include <kconfig.h>

#include "pluginbase.h"

class KLibrary;
class PluginManager;



struct PluginLibraryInfo {
    QSharedPointer<KRadioPluginFactoryBase> factory;
    QMap<QString,QString>          plugins;
    QString                        errorString;

    PluginLibraryInfo() : factory(NULL) {}
    PluginLibraryInfo(const QString &libname);
    PluginLibraryInfo (const PluginLibraryInfo &);
    PluginLibraryInfo &operator = (const PluginLibraryInfo &);
    bool valid() { return factory; }

private:
    // disable copy constructor
};


struct PluginClassInfo {
    QString                    class_name;
    QString                    description;
    QSharedPointer<KRadioPluginFactoryBase> factory;

    PluginClassInfo() : factory(NULL) {}
    PluginClassInfo(const QString &_name, const QString &descr, QSharedPointer<KRadioPluginFactoryBase> f)
        : class_name(_name), description(descr), factory(f) {}
    PluginBase *CreateInstance(const QString &instanceID, const QString &obj_name) { return factory ? factory->create(class_name, instanceID, obj_name) : NULL; }
};


class InstanceManager : public QObject
{
Q_OBJECT
public:
    InstanceManager();
    virtual ~InstanceManager();

    virtual void             saveState    (KConfig *c);
    virtual void             restoreState (KConfig *c);

    virtual PluginManager   *createNewInstance(const QString &name);

    virtual void        LoadLibrary (const QString &library);
    virtual void        UnloadLibrary (const QString &library);
    virtual PluginBase *CreatePlugin (PluginManager *manager, const QString &instanceID, const QString &pclass, const QString &object_name);

    virtual const QMap<QString, PluginLibraryInfo> &getPluginLibraries() const { return m_PluginLibraries; }
    virtual const QMap<QString, PluginClassInfo>   &getPluginClasses()   const { return m_PluginInfos; }

    virtual void  startPlugins();

    virtual bool  quitting() const { return m_quitting; }

protected slots:

    virtual void  saveState();
    virtual void  slotAboutToQuit();

protected:

    QMap<QString, PluginManager*>      m_Instances;
    QMap<QString, PluginLibraryInfo>   m_PluginLibraries;
    QMap<QString, PluginClassInfo>     m_PluginInfos;

    bool                               m_quitting;
};


#endif
