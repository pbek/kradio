/***************************************************************************
                       pluginmanager-configuration.cpp  -  description
                             -------------------
    begin                : Thu Sep 30 2004
    copyright            : (C) 2004 by Martin Witte
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

#include "ui_pluginmanager-configuration-ui.h"
#include "pluginmanager-configuration.h"
#include "kradioapp.h"
#include "pluginmanager.h"

#include <k3listbox.h>
#include <k3listview.h>
#include <kpushbutton.h>
#include <kurlrequester.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>
#include <kdemacros.h>
#include <kdeversion.h>

#include <QtGui/QCheckBox>


#include "id-generator.h"

PluginManagerConfiguration::PluginManagerConfiguration(QWidget *parent, KRadioApp *app, PluginManager *pm)
  : QWidget(parent),
    m_Application(app),
    m_PluginManager(pm),
    m_dirty(true)
{
    setupUi(this);
    // temporary fix to set icons. it does not work propertly in .ui files any more in KDE4
    btnRemovePluginInstance->setIcon(KIcon("edit-delete"));
    btnNewPluginInstance   ->setIcon(KIcon("document-new"));
    btnRemoveLibrary       ->setIcon(KIcon("edit-delete"));
    btnAddLibrary          ->setIcon(KIcon("document-new"));

    QString defaultPluginDir = KStandardDirs::installPath ("lib") + "kradio4/plugins";
    #if KDE_IS_VERSION(4, 2, 80)
        editPluginLibrary->setStartDir(defaultPluginDir);
    #else
        editPluginLibrary->setPath    (defaultPluginDir);
    #endif

    noticePluginLibrariesChanged();
    noticePluginsChanged();

    QObject::connect(btnAddLibrary,           SIGNAL(clicked()),     this, SLOT(slotAddLibrary()));
    QObject::connect(btnRemoveLibrary,        SIGNAL(clicked()),     this, SLOT(slotRemoveLibrary()));
    QObject::connect(btnNewPluginInstance,    SIGNAL(clicked()),     this, SLOT(slotNewPluginInstance()));
    QObject::connect(btnRemovePluginInstance, SIGNAL(clicked()),     this, SLOT(slotRemovePluginInstance()));
    QObject::connect(cbShowProgressBar,       SIGNAL(toggled(bool)), this, SLOT(slotSetDirty()));

    QObject::connect(rbAutoLoadYes,           SIGNAL(clicked(bool)), this, SLOT(slotSetDirty()));
    QObject::connect(rbAutoLoadNo,            SIGNAL(clicked(bool)), this, SLOT(slotSetDirty()));
    QObject::connect(rbAutoLoadAsk,           SIGNAL(clicked(bool)), this, SLOT(slotSetDirty()));

    QObject::connect(listPluginInstances,     SIGNAL(itemRenamed      (Q3ListViewItem *, int, const QString &)),
                     this,                    SLOT  (slotPluginRenamed(Q3ListViewItem *, int, const QString &)));


    slotCancel();
}


PluginManagerConfiguration::~PluginManagerConfiguration ()
{
}


void PluginManagerConfiguration::slotPluginRenamed(Q3ListViewItem *item, int col, const QString &name)
{
    if (col == 1) {
        if (m_pluginItems.contains(item)) {
            PluginBase *p = m_pluginItems[item];
            p->setName(name);
            slotSetDirty();
        }
    }
}

void PluginManagerConfiguration::noticePluginLibrariesChanged()
{
    listPluginLibraries->clear();
    const QMap<QString, PluginLibraryInfo> &libs = m_Application->getPluginLibraries();
    QMap<QString,PluginLibraryInfo>::const_iterator end = libs.end();
    for (QMap<QString,PluginLibraryInfo>::const_iterator it = libs.begin(); it != end; ++it) {
        listPluginLibraries->insertItem(it.key());
    }

    listPluginClasses->clear();
    const QMap<QString, PluginClassInfo> &classes = m_Application->getPluginClasses();
    QMap<QString, PluginClassInfo>::const_iterator end_cls = classes.end();
    for (QMap<QString, PluginClassInfo>::const_iterator it = classes.begin(); it != end_cls; ++it) {
        listPluginClasses->insertItem(new K3ListViewItem(listPluginClasses, it.key(), (*it).description));
    }

    noticePluginsChanged();
}


void PluginManagerConfiguration::noticePluginsChanged()
{
    listPluginInstances->clear();
    m_pluginItems      .clear();
    const PluginList &plugins = m_PluginManager->plugins();
    const QMap<QString, PluginClassInfo> &classes = m_Application->getPluginClasses();

    for (QList<PluginBase*>::const_iterator it = plugins.begin(); it != plugins.end(); ++it) {
        QString class_name = (*it)->pluginClassName();
        if (classes.contains(class_name)) {
            QString         obj_name = (*it)->name();
            K3ListViewItem *item     = new K3ListViewItem(listPluginInstances, class_name, obj_name, classes[class_name].description);
            item->setRenameEnabled(1, true);
            listPluginInstances->insertItem(item);

            m_pluginItems.insert(item, *it);
        }
    }
}

void PluginManagerConfiguration::noticePluginRenamed(PluginBase *p, const QString &name)
{
    Q3ListViewItem *item = m_pluginItems.key(p, NULL);
    if (item) {
        item->setText(1, name);
    }
}


void PluginManagerConfiguration::slotOK()
{
    if (m_dirty) {
        m_PluginManager->showProgressBar(cbShowProgressBar->isChecked());
        m_dirty = false;

        if (rbAutoLoadYes->isChecked()) {
            KMessageBox::saveDontShowAgainYesNo("autoload_plugins", KMessageBox::Yes);
        } else if (rbAutoLoadNo->isChecked()) {
            KMessageBox::saveDontShowAgainYesNo("autoload_plugins", KMessageBox::No);
        } else if (rbAutoLoadAsk->isChecked()) {
            KMessageBox::enableMessage("autoload_plugins");
        }
    }
}


void PluginManagerConfiguration::slotCancel()
{
    if (m_dirty) {
        cbShowProgressBar->setChecked(m_PluginManager->showsProgressBar());
        noticePluginLibrariesChanged();
        noticePluginsChanged();

        KMessageBox::ButtonCode btn;
        if (!KMessageBox::shouldBeShownYesNo("autoload_plugins", btn)) {
            if (btn == KMessageBox::Yes) {
                rbAutoLoadYes->setChecked(true);
            } else {
                rbAutoLoadNo->setChecked(true);
            }
        } else {
            rbAutoLoadAsk->setChecked(true);
        }


        m_dirty = false;
    }
}


void PluginManagerConfiguration::slotAddLibrary()
{
    slotSetDirty();
    KUrl url = editPluginLibrary->url();
    if (m_Application && url.pathOrUrl().length()) {
        if (url.isLocalFile()) {
            m_Application->LoadLibrary(url.toLocalFile());
        }
        else {
            KMessageBox::error(NULL,
                               i18n("Library %1:\nCannot load non-local library", url.pathOrUrl()),
                               i18n("Plugin Library Load Error"));
        }
    }
}


void PluginManagerConfiguration::slotRemoveLibrary()
{
    slotSetDirty();
    if (m_Application) {
        QString lib = listPluginLibraries->currentText();
        if (lib.length()) {
            m_Application->UnloadLibrary(lib);
        }
    }
}


void PluginManagerConfiguration::slotNewPluginInstance()
{
    slotSetDirty();
    if (m_Application && m_PluginManager) {
        Q3ListViewItem *item = listPluginClasses->currentItem();
        QString class_name = item ? item->text(0) : QString::null;
        bool ok = false;
        int default_object_id = 1;
        while (m_PluginManager->getPluginByName(class_name + QString::number(default_object_id)))
            ++default_object_id;

        QString object_name = KInputDialog::getText(i18n("Enter Plugin Instance Name"),
                                                    i18n("Instance name:"),
                                                    class_name + QString::number(default_object_id),
                                                    &ok);
        if (ok && class_name.length() && object_name.length()) {
            PluginBase *p = m_Application->CreatePlugin(m_PluginManager, generateRandomID(70), class_name, object_name);

            KConfig *cfg = m_Application->sessionConfig();
            m_PluginManager->restorePluginInstanceState (p, cfg);
            p->startPlugin();
        }
    }
}


void PluginManagerConfiguration::slotRemovePluginInstance()
{
    slotSetDirty();
    if (m_Application && m_PluginManager) {
        Q3ListViewItem *item = listPluginInstances->currentItem();
        QString object_name = item ? item->text(1) : QString::null;
        if (object_name.length())
            m_PluginManager->deletePluginByName(object_name);
    }
}


void PluginManagerConfiguration::slotSetDirty()
{
    m_dirty = true;
}


#include "pluginmanager-configuration.moc"
