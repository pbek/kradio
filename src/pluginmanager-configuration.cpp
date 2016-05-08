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

#include "pluginmanager-configuration.h"
#include "instancemanager.h"
#include "pluginmanager.h"
#include "pluginsmodel.h"
#include "pluginsdelegate.h"

#include <kpushbutton.h>
#include <kurlrequester.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kdemacros.h>
#include <kapplication.h>
#include <kicon.h>

#include <QCheckBox>
#include <QStandardItemModel>


#include "id-generator.h"

PluginManagerConfiguration::PluginManagerConfiguration(QWidget *parent, InstanceManager *im, PluginManager *pm)
  : QWidget(parent),
    m_instanceManager(im),
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
    editPluginLibrary->setStartDir(defaultPluginDir);
    editPluginLibrary->setMode(editPluginLibrary->mode() | KFile::LocalOnly);

    m_pluginsModel = new PluginsModel(listPlugins);
    listPlugins->setModel(m_pluginsModel);
    listPlugins->setItemDelegate(new PluginsDelegate(listPlugins));

    QObject::connect(btnAddLibrary,           SIGNAL(clicked()),     this, SLOT(slotAddLibrary()));
    QObject::connect(btnRemoveLibrary,        SIGNAL(clicked()),     this, SLOT(slotRemoveLibrary()));
    QObject::connect(btnNewPluginInstance,    SIGNAL(clicked()),     this, SLOT(slotNewPluginInstance()));
    QObject::connect(btnRemovePluginInstance, SIGNAL(clicked()),     this, SLOT(slotRemovePluginInstance()));
    QObject::connect(cbShowProgressBar,       SIGNAL(toggled(bool)), this, SLOT(slotSetDirty()));

    QObject::connect(rbAutoLoadYes,           SIGNAL(clicked(bool)), this, SLOT(slotSetDirty()));
    QObject::connect(rbAutoLoadNo,            SIGNAL(clicked(bool)), this, SLOT(slotSetDirty()));
    QObject::connect(rbAutoLoadAsk,           SIGNAL(clicked(bool)), this, SLOT(slotSetDirty()));

    QObject::connect(listPlugins->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                     this, SLOT(slotCurrentChanged(QModelIndex,QModelIndex)));


    // will directly call noticePluginLibrariesChanged()
    slotCancel();

    slotCurrentChanged(listPlugins->currentIndex(), QModelIndex());
}


PluginManagerConfiguration::~PluginManagerConfiguration ()
{
}


void PluginManagerConfiguration::noticePluginLibrariesChanged()
{
    listPluginLibraries->clear();
    const QMap<QString, PluginLibraryInfo> &libs = m_instanceManager->getPluginLibraries();
    QMap<QString,PluginLibraryInfo>::const_iterator end = libs.end();
    for (QMap<QString,PluginLibraryInfo>::const_iterator it = libs.begin(); it != end; ++it) {
        listPluginLibraries->addItem(it.key());
    }

    m_pluginsModel->setPluginClasses(m_instanceManager->getPluginClasses());

    foreach (PluginBase *plugin, m_PluginManager->plugins()) {
        noticePluginAdded(plugin);
    }
}


void PluginManagerConfiguration::noticePluginRenamed(PluginBase *p, const QString &)
{
    m_pluginsModel->notifyPluginRenamed(p);
}


void PluginManagerConfiguration::noticePluginAdded(PluginBase *p)
{
    m_pluginsModel->addPlugin(p);
}


void PluginManagerConfiguration::noticePluginRemoved(PluginBase *p)
{
    m_pluginsModel->removePlugin(p);
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
    if (m_instanceManager && url.pathOrUrl().length()) {
        if (!url.isLocalFile()) {
            return;  // shouldn't happen, the url requester takes care
                     // of that
        }
        m_instanceManager->LoadLibrary(url.toLocalFile());
    }
}


void PluginManagerConfiguration::slotRemoveLibrary()
{
    slotSetDirty();
    if (m_instanceManager) {
        QListWidgetItem *item = listPluginLibraries->currentItem();
        if (item) {
            m_instanceManager->UnloadLibrary(item->text());
        }
    }
}


void PluginManagerConfiguration::slotNewPluginInstance()
{
    const QModelIndex index = listPlugins->currentIndex();
    if (!index.isValid()) {
        return;
    }

    const QString class_name = index.data(ClassNameRole).toString();
    if (class_name.isEmpty()) {
        return;
    }

    int default_object_id = 1;
    while (m_PluginManager->getPluginByName(class_name + QString::number(default_object_id))) {
        ++default_object_id;
    }

    bool ok = false;
    const QString object_name = KInputDialog::getText(i18n("Enter Plugin Instance Name"),
                                                      i18n("Instance name:"),
                                                      class_name + QString::number(default_object_id),
                                                      &ok);
    if (ok && !object_name.isEmpty()) {
        PluginBase *p = m_instanceManager->CreatePlugin(m_PluginManager, generateRandomID(70), class_name, object_name);

        KConfig *cfg = kapp->sessionConfig();
        m_PluginManager->restorePluginInstanceState (p, cfg);
        p->startPlugin();
    }
}


void PluginManagerConfiguration::slotRemovePluginInstance()
{
    const QModelIndex index = listPlugins->currentIndex();
    if (!index.isValid()) {
        return;
    }

    const QString instance_name = index.data(InstanceNameRole).toString();
    if (instance_name.isEmpty()) {
        return;
    }

    m_PluginManager->deletePluginByName(instance_name);
}


void PluginManagerConfiguration::slotCurrentChanged(const QModelIndex &current, const QModelIndex &)
{
    btnNewPluginInstance->setEnabled(current.isValid());
    btnRemovePluginInstance->setEnabled(current.isValid() && current.data(InstanceNameRole).isValid());
}


void PluginManagerConfiguration::slotSetDirty()
{
    m_dirty = true;
}


#include "pluginmanager-configuration.moc"
