/***************************************************************************
                          pluginmanager.cpp  -  description
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


#include "pluginbase.h"
#include "pluginmanager.h"
#include "pluginmanager-configuration.h"
#include "plugin_configuration_dialog.h"
#include "instancemanager.h"
#include "id-generator.h"

// test only
#include "stationselection_interfaces.h"


#include <kicon.h>
#include <kpagedialog.h>
#include <klocalizedstring.h>
#include <kconfig.h>
#include <kprogressdialog.h>
#include <kwindowsystem.h>
#include <kmessagebox.h>
#include <kglobal.h>

#include <QProgressBar>
#include <QLayout>
#include <QMenu>

#include "debug-profiler.h"

PluginManager::PluginManager(
    const QString &name,
    InstanceManager *im,
    const QString &configDialogTitle)
 : m_Name(name),
   m_instanceManager(im),
   m_showProgressBar(true),
   m_configDialog (NULL),
   m_configDialogID(generateRandomID(70)),
   m_pluginManagerConfiguration(NULL),
   m_configDialogTitle(configDialogTitle),
   m_widgetPluginHideShowMenu(NULL)
{
}


PluginManager::~PluginManager()
{
    delete m_pluginManagerConfiguration;
    m_pluginManagerConfiguration = NULL;

    // config Dialog must be deleted first, so we can clear m_configPages
    // without problems (this is the only place where our config dialog is deleted)
    // Without clearing this list, those pages would be deleted, but
    // we would try to delete them another time when the associated plugin is
    // deleted, because m_configPages is out of date.
    if (m_configDialog) {
        m_configDialog->cancel();
        delete m_configDialog;
    }
    m_configPages     .clear();
    m_configPageFrames.clear();
    m_configPageInfos .clear();
    m_configDialog = NULL;

    while (!m_plugins.isEmpty()) {
        deletePlugin(m_plugins.first());
    }

    if (m_widgetPluginHideShowMenu) {
        delete m_widgetPluginHideShowMenu;
        m_widgetPluginHideShowMenu = NULL;
    }
}

void PluginManager::noticeLibrariesChanged()
{
    if (m_pluginManagerConfiguration)
        m_pluginManagerConfiguration->noticePluginLibrariesChanged();
}


void PluginManager::unloadPlugins(const QString &class_name)
{
    PluginList plugins = m_plugins;
    foreach (PluginBase *p, plugins) {
        if (p->pluginClassName() == class_name) {
            deletePlugin(p);
        }
    }
}


void PluginManager::addWidgetPluginMenuItems(QMenu *menu) const
{
    foreach (PluginBase *plugin, m_plugins) {
        WidgetPluginBase *b = dynamic_cast<WidgetPluginBase*>(plugin);
        if (!b) continue;

        //QAction *action = b->getHideShowAction();
        menu->addAction(b->getHideShowAction());
        /*int id = menu->insertItem("dummy", b->getWidget(), SLOT(toggleShown()));
        map.insert(b, id);
        updateWidgetPluginMenuItem(b, menu, map, b->isReallyVisible());*/
    }
}


// not necessary any more
/*
void PluginManager::updateWidgetPluginMenuItem(WidgetPluginBase *b, QMenu *menu, QMap<WidgetPluginBase *, QAction*> &map, bool shown) const
{
    if (!b || !map.contains(b))
        return;

    const QString &name = b->description();
    QString text = (shown ? i18n("Hide %1") : i18n("Show %1", name);

    menu->changeItem(map[b],
                     QIconSet(SmallIconSet(!shown ? "kradio_show" : "kradio_hide")),
                     text);
}
*/

void PluginManager::noticeWidgetPluginShown(WidgetPluginBase *p, bool shown)
{
    foreach (PluginBase *plugin, m_plugins) {
        plugin->noticeWidgetPluginShown(p, shown);
    }
    if (shown) {
        m_widgetsShownCache.clear();
    }
}

void PluginManager::noticePluginRenamed(PluginBase *p, const QString &name)
{
    foreach (PluginBase *plugin, m_plugins) {
        plugin->noticePluginRenamed(p, name);

    }
    if (m_pluginManagerConfiguration)
        m_pluginManagerConfiguration->noticePluginRenamed(p, name);

    setConfigPageNameEtc(p);
}

void PluginManager::setConfigPageNameEtc(PluginBase *p)
{
    if (p && m_configPageFrames.contains(p) && m_configPageInfos.contains(p)) {
        KPageWidgetItem *config_page = m_configPageFrames[p];
        ConfigPageInfo   info        = m_configPageInfos [p];

        if (config_page) {
            QString itemName = info.itemName;
            QString pageHeader = info.pageHeader;
            if (!pluginHasDefaultName(p)) {
                itemName = QString::fromLatin1("%1 (%2)").arg(info.itemName, p->name());
                pageHeader = QString::fromLatin1("%1 (%2)").arg(info.pageHeader, p->name());
            }
            config_page->setName  (itemName);
            config_page->setHeader(pageHeader);
            config_page->setIcon  (KIcon(info.iconName));
        }
    }
}

PluginBase *PluginManager::getPluginByName(const QString &name) const
{
    foreach (PluginBase *plugin, m_plugins) {
        if (plugin->name() == name)
            return plugin;
    }
    return NULL;
}


void PluginManager::insertPlugin(PluginBase *p)
{
    BlockProfiler profiler("PluginManager::insertPlugin");

    if (p) {
        const PluginList oldPlugins = m_plugins;

        BlockProfiler profiler_cfg("PluginManager::insertPlugin - about/config");

        /*kdDebug() << QDateTime::currentDateTime().toString(Qt::ISODate)
                  << " Debug: Adding Plugin: " << p->name() << "\n";*/

        getConfigDialog();

        m_plugins.append(p);
        p->setManager(this);

        addConfigurationPage (p, p->createConfigurationPage());

        profiler_cfg.stop();
        BlockProfiler profiler_connect("PluginManager::insertPlugin - connect");

        // connect plugins with each other
        foreach (PluginBase *plugin, m_plugins) {
            if (plugin != p) {
                /*kdDebug() << QDateTime::currentDateTime().toString(Qt::ISODate)
                          << " Debug: connecting with " << plugin->name() << "\n";*/
                p->connectI(plugin);
            }
        }

        // perhaps some existing config pages will profit from new plugin
        // example: timecontrol profits from radio plugin
        for (QPlugin2WidgetMapIterator it = m_configPages.begin(); it != m_configPages.end(); ++it) {
            Interface *i = dynamic_cast<Interface *>(*it);
            if (i)
                i->connectI(p);
        }

        profiler_connect.stop();
        BlockProfiler profiler_widget("PluginManager::insertPlugin - notifywidgets");

        notifyPluginsAdded(p, oldPlugins);
        WidgetPluginBase *w1 = dynamic_cast<WidgetPluginBase*>(p);
        foreach (PluginBase *plugin, m_plugins) {
            if (w1)
                plugin->noticeWidgetPluginShown(w1, w1->isReallyVisible());

            WidgetPluginBase *w2 = dynamic_cast<WidgetPluginBase*>(plugin);
            if (w2)
                p->noticeWidgetPluginShown(w2, w2->isReallyVisible());
        }

        if (w1) {
            QMenu *menu = getPluginHideShowMenu();
            menu->addAction(w1->getHideShowAction());
        }

        profiler_widget.stop();
    }
}


void PluginManager::deletePlugin(PluginBase *p)
{
    if (p && m_plugins.contains(p)) {
        removePlugin(p);
        delete p;
    }
}


void PluginManager::removePlugin(PluginBase *p)
{
    if (p && m_plugins.contains(p)) {

        foreach (PluginBase *plugin, m_plugins) {
            if (plugin != p) {
                // workaround for buggy compilers/libstdc++
                if (p->destructorCalled()) {
                    p->PluginBase::disconnectI(plugin);
                } else {
                    p->disconnectI(plugin);
                }
            }
        }

        // remove config page from config dialog, only chance is to delete it
        // plugin will be notified automatically (mechanism implemented by
        // PluginBase)
        while (m_configPageFrames.contains(p)) {

            m_configDialog->removePage(m_configPageFrames[p]);

//             KPageWidgetItem *f = m_configPageFrames[p];
            m_configPageFrames.remove(p);
            m_configPages     .remove(p);
            m_configPageInfos .remove(p);
//             delete f;
        }

        // remove bindings between me and plugin
        m_plugins.removeAll(p);
        p->unsetManager();

        notifyPluginsRemoved(p);

        if (p->destructorCalled()) {
            // no more vtable, so no way to dynamic_cast
            updatePluginHideShowMenu();
        } else if (m_widgetPluginHideShowMenu) {
            WidgetPluginBase *w1 = dynamic_cast<WidgetPluginBase*>(p);
            if (w1) {
                m_widgetPluginHideShowMenu->removeAction(w1->getHideShowAction());
            }
        }
    }
}


void PluginManager::notifyPluginsAdded(PluginBase *p, const PluginList &oldList)
{
    if (!oldList.isEmpty()) {
        const PluginList addedList = PluginList() << p;
        foreach (PluginBase *plugin, oldList) {
            plugin->noticePluginsAdded(addedList);
        }
        p->noticePluginsAdded(oldList);
    }
    if (m_pluginManagerConfiguration) {
        m_pluginManagerConfiguration->noticePluginAdded(p);
    }
}


void PluginManager::notifyPluginsRemoved(PluginBase *p)
{
    if (!m_plugins.isEmpty()) {
        const PluginList removedList = PluginList() << p;
        foreach (PluginBase *plugin, m_plugins) {
            plugin->noticePluginsRemoved(removedList);
        }
        p->noticePluginsRemoved(m_plugins);
    }
    if (m_pluginManagerConfiguration) {
        m_pluginManagerConfiguration->noticePluginRemoved(p);
    }
}


#ifdef KRADIO_ENABLE_FIXMES
    #warning "FIXME: what happens if a KPageWidgetItem is deleted? Will it automatically be removed from the KPageDialog?"
#endif
KPageWidgetItem *PluginManager::addConfigurationPage (PluginBase *forWhom,
                                                      const ConfigPageInfo &info)
{
    if (!forWhom || !m_plugins.contains(forWhom) || !info.page)
        return NULL;
    KPageWidgetItem *f = addConfigurationPage(info);

    // register this frame and config page
    m_configPageFrames.insert(forWhom, f);
    m_configPages     .insert(forWhom, info.page);
    m_configPageInfos .insert(forWhom, info);

    setConfigPageNameEtc(forWhom);

    // perhaps new config page profits from existing plugins
    // example: timecontrol profits from radio plugin
    Interface *i = dynamic_cast<Interface *>(info.page);
    if (i) {
        foreach (PluginBase *plugin, m_plugins)
            i->connectI(plugin);
    }
    return f;
}


KPageWidgetItem *PluginManager::addConfigurationPage (const ConfigPageInfo &info)
{
    getConfigDialog();

    QWidget *f = info.page;
    if (f->layout()) {
        f->layout()->setMargin(0);
    }

    // make sure, that config page receives ok, apply and cancel signals
    QObject::connect(this,           SIGNAL(sigConfigOK()),   info.page, SLOT(slotOK()));
    QObject::connect(m_configDialog, SIGNAL(cancelClicked()), info.page, SLOT(slotCancel()));
    QObject::connect(m_configDialog, SIGNAL(resetClicked()),  info.page, SLOT(slotCancel()));

    // insert into config dialog
    KPageWidgetItem *item = m_configDialog->addPage(f, info.itemName);
    item->setIcon(KIcon(info.iconName));

    return item;
}


void PluginManager::createConfigDialog(const QString &title)
{
    if (m_configDialog) delete m_configDialog;
    m_configDialog = NULL;

    PluginConfigurationDialog *cfg = new PluginConfigurationDialog(
        m_configDialogID,
        title,
        /*parent = */ NULL,
        title);

    m_configDialog = cfg;
    m_configDialog->PluginBase::setName(m_configDialog->pluginClassName());

    QObject::connect(m_configDialog, SIGNAL(okClicked()),     this, SLOT(slotConfigOK()));
    QObject::connect(m_configDialog, SIGNAL(applyClicked()),  this, SLOT(slotConfigOK()));

    insertPlugin(cfg);

    addConfigurationPage(createOwnConfigurationPage());

    foreach (PluginBase *plugin, m_plugins) {
        addConfigurationPage(plugin,
                             plugin->createConfigurationPage());
    }
}


ConfigPageInfo PluginManager::createOwnConfigurationPage()
{
    m_pluginManagerConfiguration = new PluginManagerConfiguration(NULL, m_instanceManager, this);
    return ConfigPageInfo (m_pluginManagerConfiguration,
                           i18n("Plugins"),
                           i18n("Plugin Library Configuration"),
                           "preferences-plugin");
}





void PluginManager::saveState (KConfig *c) const
{
    KConfigGroup cfggrp = c->group("PluginManager-" + m_Name);
    cfggrp.writeEntry("show-progress-bar", m_showProgressBar);
    cfggrp.writeEntry("config_dialog_id",  m_configDialogID);

    cfggrp.writeEntry("show_hide_cache_entries", m_widgetsShownCache.count());
    int i = 1;
    for (QMap<QString, bool>::const_iterator it = m_widgetsShownCache.begin(); it != m_widgetsShownCache.end(); ++it, ++i) {
        cfggrp.writeEntry(QString("show_hide_cache_id_%1").arg(i), it.key());
        cfggrp.writeEntry(QString("show_hide_cache_value_%1").arg(i), *it);
    }

    int n = 0;
    foreach (PluginBase *plugin, m_plugins) {
        QString class_name  = plugin->pluginClassName();
        QString object_name = plugin->name();
        QString object_id   = plugin->instanceID();
        if (class_name.length() && object_name.length() &&
            m_instanceManager->getPluginClasses().contains(class_name))
        {
            ++n;
            const QString num = QString::number(n);
            cfggrp.writeEntry("plugin_class_"        + num, class_name);
            cfggrp.writeEntry("plugin_name_"         + num, object_name);
            cfggrp.writeEntry("plugin_instance_id_"  + num, object_id);
        }
    }
    cfggrp.writeEntry("plugins", n);

    foreach (PluginBase *p, m_plugins) {
        QString      config_name = m_Name + "-" + p->pluginClassName() + "-" + p->name() + "-" + p->instanceID();
        KConfigGroup config      = c->group(config_name);
        p->saveState(config);
    }
}


void PluginManager::restoreState (KConfig *c)
{
    BlockProfiler profile_all("PluginManager::restoreState");
    KConfigGroup cfggrp = c->group("PluginManager-" + m_Name);

    m_configDialogID  = cfggrp.readEntry("config_dialog_id", m_configDialogID);

    m_showProgressBar = cfggrp.readEntry("show-progress-bar", true);


    int n = cfggrp.readEntry("show_hide_cache_entries", 0);
    for (int i = 1; i <= n; ++i) {
        QString s = cfggrp.readEntry(QString("show_hide_cache_id_%1").arg(i), QString());
        bool    b = cfggrp.readEntry(QString("show_hide_cache_value_%1").arg(i), false);
        if (!s.isNull()) {
            m_widgetsShownCache.insert(s,b);
        }
    }


    const QMap<QString, PluginClassInfo> &plugin_classes = m_instanceManager->getPluginClasses();
    QList<QString> unused_classes = plugin_classes.keys();

    n = cfggrp.readEntry("plugins", 0);
    KProgressDialog  *progress = NULL;
    if (m_showProgressBar) {
        progress = new KProgressDialog(NULL, i18n("Starting Plugins"));
        progress->setMinimumWidth(400);
        progress->setAllowCancel(false);
        progress->show();
        progress->progressBar()->setRange(0, 2*n);
    }

    // restore previously present instances

    for (int i = 1; i <= n; ++i) {
        const QString num   = QString::number(i);
        QString class_name  = cfggrp.readEntry("plugin_class_"        + num);
        QString object_name = cfggrp.readEntry("plugin_name_"         + num);
        QString object_id   = cfggrp.readEntry("plugin_instance_id_"  + num);
        if (object_id.isEmpty())
            object_id = generateRandomID(70);

        unused_classes.removeAll(class_name);

        if (m_showProgressBar)
            progress->setLabelText(i18n("Creating Plugin %1", class_name));
        if (class_name.length() && object_name.length())
            m_instanceManager->CreatePlugin(this, object_id, class_name, object_name);
        if (m_showProgressBar)
            progress->progressBar()->setValue(i);
    }

    // create instances of so far unused plugin classes

    if (m_instanceManager && unused_classes.size() > 0) {
        QString     cls;
        QStringList lines;
        foreach (cls, unused_classes) {
            lines.append(cls + ": " + plugin_classes[cls].description);
        }
        lines.sort();

        int action = KMessageBox::questionYesNoList(NULL,
                                                    i18n("New plugins found. Do you want me to automatically create instances now?"),
                                                    lines,
                                                    i18n("New Plugins Found"),
                                                    KStandardGuiItem::yes(),
                                                    KStandardGuiItem::no(),
                                                    "autoload_plugins");

        if (action == KMessageBox::Yes) {

            int idx = n + 1;
            n += unused_classes.count();
            if (m_showProgressBar)
                progress->progressBar()->setRange(0, 2*n);
            QString cls;
            foreach (cls, unused_classes) {
                if (m_showProgressBar)
                    progress->setLabelText(i18n("Creating Plugin %1", cls));

                m_instanceManager->CreatePlugin(this, generateRandomID(70), cls, cls);

                if (m_showProgressBar)
                    progress->progressBar()->setValue(idx++);
            }
            getConfigDialog()->show();
        }
        m_pluginManagerConfiguration->slotSetDirty();
        m_pluginManagerConfiguration->slotCancel();
    }

    BlockProfiler  profile_plugins("PluginManager::restoreState -  plugins");

    // restore states of plugins

    int idx = n;
    for (PluginIterator it = m_plugins.begin(); it != m_plugins.end(); ++it, ++idx) {
        BlockProfiler profile_plugin("PluginManager::restoreState - " + (*it)->pluginClassName().toLatin1());
        if (m_showProgressBar)
            progress->setLabelText(i18n("Initializing Plugin %1", (*it)->pluginClassName()));

        restorePluginInstanceState (*it, c);

        if (m_showProgressBar)
            progress->progressBar()->setValue(idx+1);
    }
    if (m_showProgressBar)
        delete progress;
}




void PluginManager::restorePluginInstanceState (PluginBase *p, KConfig *c) const
{
    int d = KWindowSystem::currentDesktop();

    QString     orgName           = p->name();
    QString     oldPrefix         = m_Name + "-";
    bool        compatRestore     = false;
    QString     restoreConfigName;

    // we still have an old name with (generated) prefix
    if (oldPrefix.length() && orgName.startsWith(oldPrefix) && c->hasGroup(orgName)) {

        QString newName = orgName.right(orgName.length() - oldPrefix.length()).trimmed();

        p->setName(newName);

        restoreConfigName = orgName;
        compatRestore     = true;

    }

    // must be here because p->name might be changed above.
    QString storeConfigName = m_Name + "-" + p->pluginClassName() + "-" + p->name() + "-" + p->instanceID();

    if (!compatRestore) {
        // we have no generated prefix. might be a plugin generated with a new version of KRadio or named by the user

        // is the config still stored with the old naming scheme (user named the plugin instance)
        if (!c->hasGroup(storeConfigName)) {
            restoreConfigName = orgName;
            compatRestore     = true;
        }
    }

    if (!compatRestore) {
        restoreConfigName = storeConfigName;
    }


    p->restoreState(c->group(restoreConfigName));
    if (restoreConfigName != storeConfigName) {
        KConfigGroup   config = c->group(storeConfigName);
        p->saveState   (config);
        c->deleteGroup (restoreConfigName);
    }

    KWindowSystem::setCurrentDesktop(d);
}



PluginConfigurationDialog *PluginManager::getConfigDialog()
{
    if (!m_configDialog)
        createConfigDialog(m_configDialogTitle);
    return m_configDialog;
}


void PluginManager::slotConfigOK()
{
    emit sigConfigOK();
    if (m_instanceManager)
        m_instanceManager->saveState(KGlobal::config().data());
}


void  PluginManager::startPlugins()
{
    int d = KWindowSystem::currentDesktop();
    foreach (PluginBase *plugin, m_plugins) {
        plugin->startPlugin();
    }
    KWindowSystem::setCurrentDesktop(d);
}

void  PluginManager::aboutToQuit()
{
    foreach (PluginBase *plugin, m_plugins) {
        plugin->aboutToQuit();
    }
}






QMenu * PluginManager::getPluginHideShowMenu()
{
    if (!m_widgetPluginHideShowMenu) {
        updatePluginHideShowMenu();
    }
    return m_widgetPluginHideShowMenu;
}

void PluginManager::updatePluginHideShowMenu()
{
    if (!m_widgetPluginHideShowMenu) {
        m_widgetPluginHideShowMenu = new QMenu();
    }
    m_widgetPluginHideShowMenu->clear();
    QAction *a = NULL;
    a = m_widgetPluginHideShowMenu->addAction(i18nc("Show all widget plugins", "Show all"),       this, SLOT(slotShowAllWidgetPlugins()));
    a = m_widgetPluginHideShowMenu->addAction(i18nc("Hide all widget plugins", "Hide all"),       this, SLOT(slotHideAllWidgetPlugins()));
    a = m_widgetPluginHideShowMenu->addAction(i18nc("Restore all widget plugins", "Restore all"), this, SLOT(slotRestoreAllWidgetPlugins()));
    a->setToolTip(i18n("Restore state before last hide-all activation"));
    m_widgetPluginHideShowMenu->addSeparator();
    addWidgetPluginMenuItems(m_widgetPluginHideShowMenu);
}

void PluginManager::slotShowAllWidgetPlugins()
{
    foreach (PluginBase *plugin, m_plugins) {
        WidgetPluginBase *b = dynamic_cast<WidgetPluginBase*>(plugin);
        if (!b) continue;
        b->getWidget()->show();
    }
}

void PluginManager::slotHideAllWidgetPlugins()
{
    foreach (PluginBase *plugin, m_plugins) {
        WidgetPluginBase *b = dynamic_cast<WidgetPluginBase*>(plugin);
        if (!b) continue;
        bool visible = b->isAnywhereVisible();
        QString name = b->name();
        m_widgetsShownCache.insert(name, visible);
        b->getWidget()->hide();
    }
}

void PluginManager::slotRestoreAllWidgetPlugins()
{
    QMap<QString, bool> tmpCache = m_widgetsShownCache;
    int d = KWindowSystem::currentDesktop();
    foreach (PluginBase *plugin, m_plugins) {
        WidgetPluginBase *b = dynamic_cast<WidgetPluginBase*>(plugin);
        if (!b) continue;
        QString name = b ? b->name() : QString();
        if (b && tmpCache.contains(name) && tmpCache[name]) {
            b->showOnOrgDesktop();
        }
    }
    m_widgetsShownCache.clear();
    KWindowSystem::setCurrentDesktop(d);
}


void PluginManager::slotHideRestoreAllWidgetPlugins()
{
    if (!m_widgetsShownCache.count()) {
        slotHideAllWidgetPlugins();
    } else {
        slotRestoreAllWidgetPlugins();
    }
}


void PluginManager::slotDesktopChanged(int /*d*/)
{
    foreach (PluginBase *plugin, m_plugins) {
        WidgetPluginBase *b = dynamic_cast<WidgetPluginBase*>(plugin);
        if (!b) continue;

        b->updateHideShowAction(b->isReallyVisible());
    }
}


bool PluginManager::pluginHasDefaultName(PluginBase *p)
{
    QString tmp_class = p->pluginClassName();
    tmp_class = tmp_class.remove(QLatin1Char(' '));
    QString tmp_name = p->name();
    tmp_name = tmp_name.remove(QLatin1Char(' '));
    return QString::compare(tmp_class, tmp_name, Qt::CaseInsensitive) == 0 ||
           QString::compare(tmp_class + "1", tmp_name, Qt::CaseInsensitive) == 0;
}


#include "pluginmanager.moc"
