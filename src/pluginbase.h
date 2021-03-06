/***************************************************************************
                          plugins.h  -  description
                             -------------------
    begin                : Mon Mar 10 2003
    copyright            : (C) 2003 by Martin Witte
    email                : emw-kradio@nocabal.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/////////////////////////////////////////////////////////////////////////////

#ifndef KRADIO_PLUGINS_INTERFACES_H
#define KRADIO_PLUGINS_INTERFACES_H

#include <KAboutData>

#include "errorlog_interfaces.h"
#include <QString>
#include <QObject>
#include <QList>
#include <QWidget>
#include <kpluginfactory.h>

#include <kconfiggroup.h>


#include "kradio-def.h"




class PluginManager;
class PluginBase;
class QWidget;

typedef QList<PluginBase*>                   PluginList;
typedef QList<PluginBase*>::iterator         PluginIterator;
typedef QList<PluginBase*>::const_iterator   PluginConstIterator;

/* PluginBase must be inherited from Interface so that a plugin can be used
   in Interface::connect functions.

   PluginBase must not be inherited from QObject, because derived classes may
   be inherited e.g. from QWidget (multiple inheritance is not possible with
   OBjects). But we must be able to receive destroy messages e.g. from
   configuration pages. Thus we need the special callback member
   m_destroyNotifier.

   PluginBase is derived from Interface to provide connection facilities.
   In case of multiple inheritance from interface classes, connect and disconnect
   methods have to be reimplemented in order to call all inherited
   connect/disconnect methods.

*/


class WidgetPluginBase;
class PluginConfigPageBase;


struct ConfigPageInfo
{
    ConfigPageInfo () : page(NULL) {}
    ConfigPageInfo (PluginConfigPageBase * p,
                    const QString        & in,
                    const QString        & ph,
                    const QString        & icon)
      : page (p),
        itemName(in),
        pageHeader(ph),
        iconName(icon)
    {}

    PluginConfigPageBase  * page;
    QString                 itemName;
    QString                 pageHeader;
    QString                 iconName;
}; // ConfigPageInfo


class KRADIO5_EXPORT PluginBase : public IErrorLogClient
{
friend class PluginManager;
public :
             PluginBase(const QString &instanceID, const QString &name, const QString &description);
//              PluginBase(const QString &name, const QString &description);
    virtual ~PluginBase();

    virtual  QString pluginClassName() const = 0;

    const QString &name() const { return m_name; }
    void           setName(const QString &n);

    const QString &instanceID()  const { return m_instanceID; }

    const QString &description() const { return m_description; }

    // workaround for compiler bugs
    bool           destructorCalled() const { return m_destructorCalled; }

    // interaction with pluginmanager
protected:
    virtual bool setManager (PluginManager *);
    virtual void unsetManager ();
    bool isManagerSet () const;

public:

    // these two methods will request a configuration page or
    // plugin page from plugin manager
    // they will be deleted automatically when this plugin
    // is deleted, because we disconnect from pluginmanager
    // and the plugin manager will delete all associated gui elements
    virtual ConfigPageInfo createConfigurationPage();

    // save/restore status, window position, etc...

    virtual void   saveState    (      KConfigGroup &) const = 0;
    virtual void   restoreState (const KConfigGroup &)       = 0;
    virtual void   startPlugin();

    virtual void   aboutToQuit();

    //

    virtual void noticeWidgetPluginShown(WidgetPluginBase *, bool /*shown*/)     {}
    virtual void noticePluginsAdded(const PluginList &)                          {}
    virtual void noticePluginsRemoved(const PluginList &)                        {}
    virtual void noticePluginRenamed(PluginBase */*p*/, const QString &/*name*/) {}

protected :
    QString            m_instanceID;
    QString            m_name;
    QString            m_description;
    PluginManager     *m_manager;
    bool               m_destructorCalled;
};


class KRADIO5_EXPORT KRadioPluginFactoryBase : public QObject
{
Q_OBJECT
public:
    KRadioPluginFactoryBase();
    virtual ~KRadioPluginFactoryBase();

    QList<KAboutData> components() const;
    virtual PluginBase *create(const QString &type, const QString &instanceID, const QString &object_name) = 0;

protected:
    void registerComponent(const KAboutData &aboutData);

private:
    QList<KAboutData> m_components;
};


#define KRADIO_EXPORT_PLUGIN(class_name, aboutData)                 \
class class_name ## PluginFactory : public KRadioPluginFactoryBase  \
{                                                                   \
Q_OBJECT  							    \
public:                                                             \
    class_name ## PluginFactory(QObject *, const QVariantList &)    \
    {                                                               \
        Q_ASSERT(aboutData.componentName() == QLatin1String(#class_name)); \
        registerComponent(aboutData);                               \
    }                                                               \
                                                                    \
    virtual PluginBase *create(const QString &type, const QString &instanceID, const QString &object_name)  override \
    {                                                               \
        if (type == QLatin1String(#class_name)) {                   \
            return new class_name(instanceID, object_name);         \
        } else {                                                    \
            return NULL;                                            \
        }                                                           \
    }                                                               \
};                                                                  \
K_PLUGIN_FACTORY(class_name ## Plugin, registerPlugin<class_name ## PluginFactory>();) \
K_EXPORT_PLUGIN(class_name ## Plugin)


#define KRADIO_EXPORT_PLUGIN2(class_name1, aboutData1, class_name2, aboutData2) \
class class_name1 ## PluginFactory : public KRadioPluginFactoryBase  \
{                                                                    \
Q_OBJECT							     \
public:                                                              \
    class_name1 ## PluginFactory(QObject *, const QVariantList &)    \
    {                                                                \
        Q_ASSERT(aboutData1.componentName() == QLatin1String(#class_name1)); \
        Q_ASSERT(aboutData2.componentName() == QLatin1String(#class_name2)); \
        registerComponent(aboutData1);                               \
        registerComponent(aboutData2);                               \
    }                                                                \
                                                                     \
    virtual PluginBase *create(const QString &type, const QString &instanceID, const QString &object_name)  override \
    {                                                                \
        if (type == QLatin1String(#class_name1)) {                   \
            return new class_name1(instanceID, object_name);         \
        } else if (type == QLatin1String(#class_name2)) {            \
            return new class_name2(instanceID, object_name);         \
        } else {                                                     \
            return NULL;                                             \
        }                                                            \
    }                                                                \
};                                                                   \
K_PLUGIN_FACTORY(class_name1 ## Plugin, registerPlugin<class_name1 ## PluginFactory>();) \
K_EXPORT_PLUGIN(class_name1 ## Plugin)


#endif
