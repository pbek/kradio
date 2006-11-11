/***************************************************************************
                          plugins.h  -  description
                             -------------------
    begin                : Mon Mär 10 2003
    copyright            : (C) 2003 by Martin Witte
    email                : witte@kawo1.rwth-aachen.de
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kglobal.h>

#include "errorlog-interfaces.h"
#include <qstring.h>
#include <qobject.h>
#include <qptrlist.h>

class PluginManager;
class PluginBase;
class QWidget;
class KConfig;

typedef QPtrList<PluginBase>           PluginList;
typedef QPtrListIterator<PluginBase>   PluginIterator;

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

struct ConfigPageInfo
{
    ConfigPageInfo () : page(NULL) {}
    ConfigPageInfo (QWidget *p,
                    const QString &in,
                    const QString &ph,
                    const QString &icon)
      : page (p),
        itemName(in),
        pageHeader(ph),
        iconName(icon)
    {}

    QWidget  *page;
    QString   itemName,
              pageHeader,
              iconName;
};

typedef ConfigPageInfo AboutPageInfo;


class PluginBase : public IErrorLogClient
{
friend class PluginManager;
public :
             PluginBase(const QString &name, const QString &description);
    virtual ~PluginBase();

    virtual  QString pluginClassName() const = 0;

    const QString &name() const { return m_name; }
          QString &name()       { return m_name; }

    const QString &description() const { return m_description; }

    // workaround for compiler bugs
    bool           destructorCalled() const { return m_destructorCalled; }

    // interaction with pluginmanager
protected:
    bool setManager (PluginManager *);
    void unsetManager ();
    bool isManagerSet () const;

public:

    // these two methods will request a configuration page or
    // plugin page from plugin manager
    // they will be deleted automatically when this plugin
    // is deleted, because we disconnect from pluginmanager
    // and the plugin manager will delete all associated gui elements
    virtual ConfigPageInfo createConfigurationPage () = 0;
    virtual AboutPageInfo  createAboutPage () = 0;

    // save/restore status, window position, etc...

    virtual void   saveState (KConfig *) const = 0;
    virtual void   restoreState (KConfig *) = 0;
    virtual void   startPlugin();

    virtual void   aboutToQuit();

    //

    virtual void noticeWidgetPluginShown(WidgetPluginBase *, bool /*shown*/) {}
    virtual void noticePluginsChanged(const PluginList &) {}

protected :
    QString            m_name;
    QString            m_description;
    PluginManager     *m_manager;
    bool               m_destructorCalled;
};


#define PLUGIN_LIBRARY_FUNCTIONS(class_name, i18nName, description) \
extern "C" void KRadioPlugin_LoadLibrary() \
{                                                                 \
    KGlobal::locale()->insertCatalogue(i18nName);                 \
}                                                                 \
                                                                  \
extern "C" void KRadioPlugin_UnloadLibrary() \
{                                                                 \
    KGlobal::locale()->removeCatalogue(i18nName);                 \
}                                                                 \
                                                                  \
extern "C" void KRadioPlugin_GetAvailablePlugins(QMap<QString, QString> &info) \
{                                                                 \
    info.insert(#class_name, (description));                      \
}                                                                 \
                                                                  \
extern "C" PluginBase *KRadioPlugin_CreatePlugin(const QString &type, const QString &object_name) \
{                                                                                    \
    if (type == #class_name) {                                                       \
        return new class_name(object_name);                                          \
    } else {                                                                         \
        return NULL;                                                                 \
    }                                                                                \
}


#define PLUGIN_LIBRARY_FUNCTIONS2(class_name1, i18nName, description1, class_name2, description2) \
extern "C" void KRadioPlugin_LoadLibrary() \
{                                                                 \
    KGlobal::locale()->insertCatalogue(i18nName);                 \
}                                                                 \
                                                                  \
extern "C" void KRadioPlugin_UnloadLibrary() \
{                                                                 \
    KGlobal::locale()->removeCatalogue(i18nName);                 \
}                                                                 \
                                                                  \
extern "C" void KRadioPlugin_GetAvailablePlugins(QMap<QString, QString> &info) \
{                                                                 \
    info.insert(#class_name1, (description1));                    \
    info.insert(#class_name2, (description2));                    \
}                                                                 \
                                                                  \
extern "C" PluginBase *KRadioPlugin_CreatePlugin(const QString &type, const QString &object_name) \
{                                                                                       \
    if (type == #class_name1) {                                                         \
        return new class_name1(object_name);                                            \
    } else if (type == #class_name2) {                                                  \
        return new class_name2(object_name);                                            \
    } else {                                                                            \
        return NULL;                                                                    \
    }                                                                                   \
}


#endif
