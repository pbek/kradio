/***************************************************************************
                          pluginmanager.cpp  -  description
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

#include "include/plugins.h"
#include "include/pluginmanager.h"
#include "include/pluginmanager-configuration.h"
#include "include/plugin_configuration_dialog.h"
#include "include/kradioapp.h"

#include <kiconloader.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <kconfig.h>
#include <kprogress.h>

#include <qlayout.h>
#include <qframe.h>
#include <qmenudata.h>

#include "include/debug-profiler.h"

PluginManager::PluginManager(
    const QString &name,
    KRadioApp     *app,
    const QString &configDialogTitle,
    const QString &aboutDialogTitle)
 : m_Name(name),
   m_Application(app),
   m_showProgressBar(true),
   m_configDialog (NULL),
   m_pluginManagerConfiguration(NULL),
   m_aboutDialog(NULL),
   m_configDialogTitle(configDialogTitle),
   m_aboutDialogTitle (aboutDialogTitle)
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
    m_configPages.clear();
    m_configPageFrames.clear();
    m_configDialog = NULL;

    if (m_aboutDialog)
        delete m_aboutDialog;
    m_aboutPages.clear();
    m_aboutPageFrames.clear();
    m_aboutDialog = NULL;

    while (PluginBase *p = m_plugins.getFirst()) {
        deletePlugin(p);
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
    for (PluginIterator it(plugins); it.current(); ++it) {
        PluginBase *p = it.current();
        if (p->pluginClassName() == class_name) {
            deletePlugin(p);
        }
    }
}


void PluginManager::addWidgetPluginMenuItems(QMenuData *menu, QMap<WidgetPluginBase *,int> &map) const
{
    map.clear();

    for (PluginIterator it(m_plugins); it.current(); ++it) {
        WidgetPluginBase *b = dynamic_cast<WidgetPluginBase*>(it.current());
        if (!b) continue;

        int id = menu->insertItem("dummy", b->getWidget(), SLOT(toggleShown()));
        map.insert(b, id);
        updateWidgetPluginMenuItem(b, menu, map, b->isReallyVisible());
    }
}


void PluginManager::updateWidgetPluginMenuItem(WidgetPluginBase *b, QMenuData *menu, QMap<WidgetPluginBase *,int> &map, bool shown) const
{
    if (!b || !map.contains(b))
        return;

    const QString &name = b->description();
    QString text = (shown ? i18n("Hide %1") : i18n("Show %1")).arg(name);

    menu->changeItem(map[b],
                     QIconSet(SmallIconSet(!shown ? "kradio_show" : "kradio_hide")),
                     text);
}


void PluginManager::noticeWidgetPluginShown(WidgetPluginBase *p, bool shown)
{
    for (PluginIterator it(m_plugins); it.current(); ++it) {
        it.current()->noticeWidgetPluginShown(p, shown);
    }
}


PluginBase *PluginManager::getPluginByName(const QString &name) const
{
    for (PluginIterator it(m_plugins); it.current(); ++it) {
        if (it.current()->name() == name)
            return it.current();
    }
    return NULL;
}


void PluginManager::insertPlugin(PluginBase *p)
{
    BlockProfiler profiler("PluginManager::insertPlugin");

    if (p) {
        BlockProfiler profiler_cfg("PluginManager::insertPlugin - about/config");

        /*kdDebug() << QDateTime::currentDateTime().toString(Qt::ISODate)
                  << " Debug: Adding Plugin: " << p->name() << "\n";*/

        if (!m_configDialog)
            createConfigDialog(m_configDialogTitle);
        if (!m_aboutDialog)
            createAboutDialog(m_aboutDialogTitle);

        m_plugins.append(p);
        p->setManager(this);

        addConfigurationPage (p, p->createConfigurationPage());
        addAboutPage         (p, p->createAboutPage());

        profiler_cfg.stop();
        BlockProfiler profiler_connect("PluginManager::insertPlugin - connect");

        // connect plugins with each other
        for (PluginIterator it(m_plugins); it.current(); ++it) {
            if (it.current() != p) {
                /*kdDebug() << QDateTime::currentDateTime().toString(Qt::ISODate)
                          << " Debug: connecting with " << it.current()->name() << "\n";*/
                p->connectI(it.current());
            }
        }

        // perhaps some existing config pages will profit from new plugin
        // example: timecontrol profits from radio plugin
        for (QWidgetDictIterator it(m_configPages); it.current(); ++it) {
            Interface *i = dynamic_cast<Interface *>(it.current());
            if (i)
                i->connectI(p);
        }

        profiler_connect.stop();
        BlockProfiler profiler_widget("PluginManager::insertPlugin - notifywidgets");

        WidgetPluginBase *w1 = dynamic_cast<WidgetPluginBase*>(p);
        for (PluginIterator it(m_plugins); it.current(); ++it) {
            it.current()->noticePluginsChanged(m_plugins);
            if (w1)
                it.current()->noticeWidgetPluginShown(w1, w1->isReallyVisible());

            WidgetPluginBase *w2 = dynamic_cast<WidgetPluginBase*>(it.current());
            if (w2)
                p->noticeWidgetPluginShown(w2, w2->isReallyVisible());
        }

        if (m_pluginManagerConfiguration)
            m_pluginManagerConfiguration->noticePluginsChanged();

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

        for (PluginIterator it(m_plugins); it.current(); ++it) {
            if (it.current() != p) {
                // workaround for buggy compilers/libstdc++
                if (p->destructorCalled()) {
                    p->PluginBase::disconnectI(it.current());
                } else {
                    p->disconnectI(it.current());
                }
            }
        }

        // remove config page from config dialog, only chance is to delete it
        // plugin will be notified automatically (mechanism implemented by
        // PluginBase)
        while (QFrame *f = m_configPageFrames.find(p)) {
            m_configPageFrames.remove(p);
            m_configPages.remove(p);
            delete f;
        }
        while (QFrame *f = m_aboutPageFrames.find(p)) {
            m_aboutPageFrames.remove(p);
            m_aboutPages.remove(p);
            delete f;
        }

        // remove bindings between me and plugin
        m_plugins.remove(p);
        p->unsetManager();

        p->noticePluginsChanged(PluginList());
        for (PluginIterator it(m_plugins); it.current(); ++it) {
            it.current()->noticePluginsChanged(m_plugins);
        }

        if (m_pluginManagerConfiguration)
            m_pluginManagerConfiguration->noticePluginsChanged();
    }
}


void PluginManager::addConfigurationPage (PluginBase *forWhom,
                                          const ConfigPageInfo &info)
{
    if (!forWhom || !m_plugins.containsRef(forWhom) || !info.page)
        return;
    QFrame *f = addConfigurationPage(info);

    // register this frame and config page
    m_configPageFrames.insert(forWhom, f);
    m_configPages.insert(forWhom, info.page);

    // perhaps new config page profits from existing plugins
    // example: timecontrol profits from radio plugin
    Interface *i = dynamic_cast<Interface *>(info.page);
    if (i) {
        for (PluginIterator it(m_plugins); it.current(); ++it)
            i->connectI(it.current());
    }
}


QFrame *PluginManager::addConfigurationPage (const ConfigPageInfo &info)
{
    if (!m_configDialog)
        createConfigDialog(i18n(m_configDialogTitle.ascii()));

    // create empty config frame
    QFrame *f = m_configDialog->addPage(
      info.itemName,
      info.pageHeader,
      KGlobal::instance()->iconLoader()->loadIcon( info.iconName, KIcon::NoGroup, KIcon::SizeMedium )
    );

    // fill config frame with layout ...
    QGridLayout *l = new QGridLayout(f);
    l->setSpacing( 0 );
    l->setMargin( 0 );

    // ... and externally created config page
    info.page->reparent (f, QPoint(0,0), true);
    l->addWidget( info.page, 0, 0 );

    // make sure, that config page receives ok, apply and cancel signals
    QObject::connect(this,           SIGNAL(sigConfigOK()),   info.page, SLOT(slotOK()));
    QObject::connect(m_configDialog, SIGNAL(cancelClicked()), info.page, SLOT(slotCancel()));

    return f;
}


void PluginManager::createConfigDialog(const QString &title)
{
    if (m_configDialog) delete m_configDialog;
    m_configDialog = NULL;

    PluginConfigurationDialog *cfg = new PluginConfigurationDialog(
        KDialogBase::IconList,
        title,
        KDialogBase::Apply|KDialogBase::Ok|KDialogBase::Cancel,
        KDialogBase::Ok,
        /*parent = */ NULL,
        title.ascii(),
        /*modal = */ false,
        true);

    m_configDialog = cfg;

    QObject::connect(m_configDialog, SIGNAL(okClicked()),     this, SLOT(slotConfigOK()));
    QObject::connect(m_configDialog, SIGNAL(applyClicked()),  this, SLOT(slotConfigOK()));

    insertPlugin(cfg);

    addConfigurationPage(createOwnConfigurationPage());

    for (PluginIterator i(m_plugins); m_configDialog && i.current(); ++i) {
        addConfigurationPage(i.current(),
                             i.current()->createConfigurationPage());
    }
}


ConfigPageInfo PluginManager::createOwnConfigurationPage()
{
    m_pluginManagerConfiguration = new PluginManagerConfiguration(NULL, m_Application, this);
    return ConfigPageInfo (m_pluginManagerConfiguration,
                           i18n("Plugins"),
                           i18n("Plugin Library Configuration"),
                           "kradio_plugins");
}





void PluginManager::addAboutPage (PluginBase *forWhom,
                                  const AboutPageInfo &info)
{
    if (!m_aboutDialog)
        createAboutDialog(i18n(m_aboutDialogTitle.ascii()));

    if (   !forWhom || !m_plugins.containsRef(forWhom)
        || !m_aboutDialog || !info.page)
        return;


    // create empty about frame
    QFrame *f = m_aboutDialog->addPage(
      info.itemName,
      info.pageHeader,
      KGlobal::instance()->iconLoader()->loadIcon( info.iconName, KIcon::NoGroup, KIcon::SizeMedium )
    );

    // register this frame and config page
    m_aboutPageFrames.insert(forWhom, f);
    m_aboutPages.insert(forWhom, info.page);

    // fill config frame with layout ...
    QGridLayout *l = new QGridLayout(f);
    l->setSpacing( 0 );
    l->setMargin( 0 );

    // ... and externally created config page
    info.page->reparent (f, QPoint(0,0), true);
    l->addWidget( info.page, 0, 0 );
}


void PluginManager::createAboutDialog(const QString &title)
{
    if (m_aboutDialog) delete m_aboutDialog;
    m_aboutDialog = NULL;

    m_aboutDialog = new KDialogBase(KDialogBase::IconList,
                                    title,
                                    KDialogBase::Close,
                                    KDialogBase::Close,
                                    /*parent = */ NULL,
                                    title.ascii(),
                                    /*modal = */ false,
                                    true);

    for (PluginIterator i(m_plugins); m_aboutDialog && i.current(); ++i) {
        addAboutPage(i.current(),
                     i.current()->createAboutPage());
    }
}


void PluginManager::saveState (KConfig *c) const
{
    c->setGroup("PluginManager-" + m_Name);
    c->writeEntry("show-progress-bar", m_showProgressBar);
    int n = 0;
    for (PluginIterator it(m_plugins); it.current(); ++it) {
        QString class_name  = it.current()->pluginClassName();
        QString object_name = it.current()->name();
        if (class_name.length() && object_name.length() &&
            m_Application->getPluginClasses().contains(class_name))
        {
            ++n;
            c->writeEntry("plugin_class_" + QString::number(n), class_name);
            c->writeEntry("plugin_name_"  + QString::number(n), object_name);
        }
    }
    c->writeEntry("plugins", n);

    for (PluginIterator i(m_plugins); i.current(); ++i) {
        i.current()->saveState(c);
    }
}


void PluginManager::restoreState (KConfig *c)
{
    BlockProfiler profile_all("PluginManager::restoreState");
    c->setGroup("PluginManager-" + m_Name);
    m_showProgressBar = c->readBoolEntry("show-progress-bar", true);
    int n = c->readNumEntry("plugins", 0);

    KProgressDialog  *progress = NULL;
    if (m_showProgressBar) {
        progress = new KProgressDialog(NULL, NULL, i18n("Starting Plugins"));
        progress->setMinimumWidth(400);
        progress->setAllowCancel(false);
        progress->show();
        progress->progressBar()->setTotalSteps(2*n);
    }

    for (int i = 1; i <= n; ++i) {
        c->setGroup("PluginManager-" + m_Name);
        QString class_name  = c->readEntry("plugin_class_" + QString::number(i));
        QString object_name = c->readEntry("plugin_name_"  + QString::number(i));

        if (m_showProgressBar)
            progress->QWidget::setCaption(i18n("Creating Plugin %1").arg(class_name));
        if (class_name.length() && object_name.length())
            m_Application->CreatePlugin(this, class_name, object_name);
        if (m_showProgressBar)
            progress->progressBar()->setProgress(i);
    }

    if (m_Application && n == 0) {
        const QMap<QString, PluginClassInfo> &classes = m_Application->getPluginClasses();
        QMapConstIterator<QString, PluginClassInfo> end = classes.end();
        n = classes.count();
        if (m_showProgressBar)
            progress->progressBar()->setTotalSteps(2*n);
        int idx = 1;
        for (QMapConstIterator<QString, PluginClassInfo> it=classes.begin(); it != end; ++it, ++idx) {
            const PluginClassInfo &cls = *it;
            if (m_showProgressBar)
                progress->QWidget::setCaption(i18n("Creating Plugin %1").arg(cls.class_name));
            m_Application->CreatePlugin(this, cls.class_name, m_Name + "-" + cls.class_name);
            if (m_showProgressBar)
                progress->progressBar()->setProgress(idx);
        }
        m_configDialog->show();
    }

    BlockProfiler  profile_plugins("PluginManager::restoreState -  plugins");

    int idx = n;
    for (PluginIterator i(m_plugins); i.current(); ++i, ++idx) {
        BlockProfiler profile_plugin("PluginManager::restoreState - " + i.current()->pluginClassName());
        if (m_showProgressBar)
            progress->QWidget::setCaption(i18n("Initializing Plugin %1").arg(i.current()->pluginClassName()));
        i.current()->restoreState(c);
        if (m_showProgressBar)
            progress->progressBar()->setProgress(idx+1);
    }
    if (m_showProgressBar)
        delete progress;
}

PluginConfigurationDialog *PluginManager::getConfigDialog()
{
    if (!m_configDialog)
        createConfigDialog(m_configDialogTitle);
    return m_configDialog;
}

KDialogBase               *PluginManager::getAboutDialog()
{
    if (!m_aboutDialog)
        createAboutDialog();
    return m_aboutDialog;
}



void PluginManager::slotConfigOK()
{
    emit sigConfigOK();
    if (m_Application)
        m_Application->saveState(KGlobal::config());
}


void  PluginManager::startPlugins()
{
    for (PluginIterator i(m_plugins); i.current(); ++i) {
        i.current()->startPlugin();
    }
}

void  PluginManager::aboutToQuit()
{
    for (PluginIterator i(m_plugins); i.current(); ++i) {
        i.current()->aboutToQuit();
    }
}


#include "pluginmanager.moc"
