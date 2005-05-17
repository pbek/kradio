/***************************************************************************
                          pluginmanager.h  -  description
                             -------------------
    begin                : Mon Apr 28 2003
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

#ifndef KRADIO_PLUGINMANAGER_INTERFACES_H
#define KRADIO_PLUGINMANAGER_INTERFACES_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qstring.h>
#include <qptrdict.h>

#include "plugins.h"

class PluginBase;
class WidgetPluginBase;
class PluginConfigurationDialog;
class PluginManagerConfiguration;
class QWidget;
class KConfig;
class QFrame;
class KAboutDialog;
class KDialogBase;

struct ConfigPageInfo;
class QMenuData;
class KRadioApp;


class PluginManager : public QObject
{
Q_OBJECT
public :
             PluginManager(const QString &name,
                           KRadioApp *app,
                           const QString &configDialogTitle,
                           const QString &aboutTitle);
    virtual ~PluginManager();

    // Library Functions

    void                 noticeLibrariesChanged();
    void                 unloadPlugins(const QString &class_name);

    // managing plugins

    const PluginList    &plugins() const { return m_plugins; }
    void                 addWidgetPluginMenuItems(QMenuData *menu, QMap<WidgetPluginBase *,int> &map) const;
    void                 updateWidgetPluginMenuItem(WidgetPluginBase *p, QMenuData *menu, QMap<WidgetPluginBase *,int> &map, bool shown) const;

    PluginBase          *getPluginByName(const QString &name) const;

    // after insert, pluginManager is responsible for deletion
    void                 insertPlugin(PluginBase *);

    // remove and delete plugin
    void                 deletePlugin(PluginBase *);
    void                 deletePluginByName(const QString &name) { deletePlugin(getPluginByName(name)); }

    // remove plugin, afterwards pluginManager is no longer responsible for deletion
    void                 removePlugin(PluginBase *);
    void                 removePluginByName(const QString &name) { removePlugin(getPluginByName(name)); }

    // operations on all plugins

    virtual void         saveState    (KConfig *) const;
    virtual void         restoreState (KConfig *);

    // configuration dialog handling

    virtual PluginConfigurationDialog *getConfigDialog();
    virtual KDialogBase               *getAboutDialog();

    virtual void         noticeWidgetPluginShown(WidgetPluginBase *p, bool shown);

protected :
    virtual void         createConfigDialog(const QString &title = QString::null);
    virtual void         createAboutDialog (const QString &title = QString::null);

    virtual void         addConfigurationPage (PluginBase *forWhom,
                                               const ConfigPageInfo    &info);
    virtual void         addAboutPage         (PluginBase *forWhom,
                                               const AboutPageInfo     &info);

protected slots:

    virtual void         slotConfigOK();

signals:

    virtual void         sigConfigOK();

private:
    virtual QFrame      *addConfigurationPage (const ConfigPageInfo    &info);
    ConfigPageInfo       createOwnConfigurationPage();

    // PluginManager's data & types ;)
protected:
    typedef QPtrDict<QFrame>               QFrameDict;
    typedef QPtrDictIterator<QFrame>       QFrameDictIterator;
    typedef QPtrDict<QWidget>              QWidgetDict;
    typedef QPtrDictIterator<QWidget>      QWidgetDictIterator;

    QString      m_Name;
    KRadioApp   *m_Application;

    PluginList   m_plugins;

    QFrameDict   m_configPageFrames;
    QWidgetDict  m_configPages;

    QFrameDict   m_aboutPageFrames;
    QWidgetDict  m_aboutPages;

    PluginConfigurationDialog *m_configDialog;
    PluginManagerConfiguration*m_pluginManagerConfiguration;
    KDialogBase               *m_aboutDialog;
    QString                    m_configDialogTitle;
    QString                    m_aboutDialogTitle;
};




#endif
