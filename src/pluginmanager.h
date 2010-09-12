/***************************************************************************
                          pluginmanager.h  -  description
                             -------------------
    begin                : Mon Apr 28 2003
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

#ifndef KRADIO_PLUGINMANAGER_INTERFACES_H
#define KRADIO_PLUGINMANAGER_INTERFACES_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QtCore/QString>
#include <QtCore/QMap>

#include "pluginbase.h"

class PluginBase;
class WidgetPluginBase;
class PluginConfigurationDialog;
class PluginManagerConfiguration;
class QWidget;
class KConfig;
class QFrame;
// class KAboutDialog;
class KPageDialog;
class KPageWidgetItem;

struct ConfigPageInfo;
class QMenu;
class KMenu;
class KRadioApp;


class KDE_EXPORT PluginManager : public QObject
{
Q_OBJECT
public :
             PluginManager(const QString &name,
                           KRadioApp *app,
                           const QString &configDialogTitle,
                           const QString &aboutTitle);
    virtual ~PluginManager();

    const QString        instanceName() const { return m_Name; }

    // Library Functions

    void                 noticeLibrariesChanged();
    void                 unloadPlugins(const QString &class_name);

    // managing plugins

    const PluginList    &plugins() const { return m_plugins; }
    void                 addWidgetPluginMenuItems(QMenu *menu) const;
    //void                 updateWidgetPluginMenuItem(WidgetPluginBase *p, QMenu *menu, QMap<WidgetPluginBase *,int> &map, bool shown) const;

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

    virtual void         saveState                  (KConfig *) const;
    virtual void         restoreState               (KConfig *);
    virtual void         restorePluginInstanceState (PluginBase *p, KConfig *c) const;

    virtual void         startPlugins();

    // configuration dialog handling

    virtual PluginConfigurationDialog *getConfigDialog();
//     virtual KPageDialog               *getAboutDialog();
    virtual bool                       showsProgressBar() const { return m_showProgressBar; }
    virtual void                       showProgressBar(bool b) { m_showProgressBar = b; }

    virtual void                       noticeWidgetPluginShown(WidgetPluginBase *p, bool shown);
    virtual void                       noticePluginRenamed(PluginBase *p, const QString &name);


    virtual KMenu                     *getPluginHideShowMenu();


protected :
    virtual void                       createConfigDialog(const QString &title = QString::null);
//     virtual void                       createAboutDialog (const QString &title = QString::null);

    virtual KPageWidgetItem           *addConfigurationPage (PluginBase *forWhom,
                                                             const ConfigPageInfo    &info);
//     virtual KPageWidgetItem           *addAboutPage         (PluginBase *forWhom,
//                                                              const AboutPageInfo     &info);
    virtual void                       setConfigPageNameEtc(PluginBase *p);


    virtual void         updatePluginHideShowMenu();
    virtual void         notifyPluginsChanged();

protected slots:

    virtual void         slotConfigOK();
    virtual void         slotDesktopChanged(int d);


public slots:
    virtual void         aboutToQuit();
    virtual void         slotShowAllWidgetPlugins();
    virtual void         slotHideAllWidgetPlugins();
    virtual void         slotRestoreAllWidgetPlugins();
    virtual void         slotHideRestoreAllWidgetPlugins();

signals:

            void         sigConfigOK();

private:
    virtual KPageWidgetItem      *addConfigurationPage (const ConfigPageInfo    &info);
    ConfigPageInfo                createOwnConfigurationPage();

    // PluginManager's data & types ;)
protected:
    typedef QMap<PluginBase*, KPageWidgetItem*>             QPlugin2ConfigPageMap;
    typedef QMap<PluginBase*, KPageWidgetItem*>::iterator   QPlugin2ConfigPageMapIterator;
    typedef QMap<PluginBase*, QWidget*>                     QPlugin2WidgetMap;
    typedef QMap<PluginBase*, QWidget*>::iterator           QPlugin2WidgetMapIterator;
    typedef QMap<PluginBase*, ConfigPageInfo>               QPlugin2ConfigPageInfoMap;
    typedef QMap<PluginBase*, ConfigPageInfo>::iterator     QPlugin2ConfigPageInfoMapIterator;

    QString      m_Name;
    KRadioApp   *m_Application;

    PluginList   m_plugins;
    bool         m_showProgressBar;

    QPlugin2ConfigPageMap       m_configPageFrames;
    QPlugin2WidgetMap           m_configPages;
    QPlugin2ConfigPageInfoMap   m_configPageInfos;

//     QPlugin2ConfigPageMap       m_aboutPageFrames;
//     QPlugin2WidgetMap           m_aboutPages;

    PluginConfigurationDialog  *m_configDialog;
    QString                     m_configDialogID;
    PluginManagerConfiguration *m_pluginManagerConfiguration;
//     KPageDialog                *m_aboutDialog;
    QString                     m_configDialogTitle;
//     QString                     m_aboutDialogTitle;

    KMenu                      *m_widgetPluginHideShowMenu;
    QMap<QString, bool>         m_widgetsShownCache;
};




#endif
