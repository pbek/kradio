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
#include "kradioapp.h"
#include "pluginmanager.h"

#include <kpushbutton.h>
#include <kurlrequester.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kdemacros.h>

#include <QCheckBox>
#include <QStandardItemModel>


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
    editPluginLibrary->setStartDir(defaultPluginDir);
    editPluginLibrary->setMode(editPluginLibrary->mode() | KFile::LocalOnly);

    m_pluginInstancesModel = new QStandardItemModel(listPluginInstances);
    QStringList headers;
    headers << i18n("Plugin Class");
    headers << i18n("Instance Name");
    headers << i18n("Description");
    m_pluginInstancesModel->setHorizontalHeaderLabels(headers);
    listPluginInstances->setModel(m_pluginInstancesModel);

    QObject::connect(btnAddLibrary,           SIGNAL(clicked()),     this, SLOT(slotAddLibrary()));
    QObject::connect(btnRemoveLibrary,        SIGNAL(clicked()),     this, SLOT(slotRemoveLibrary()));
    QObject::connect(btnNewPluginInstance,    SIGNAL(clicked()),     this, SLOT(slotNewPluginInstance()));
    QObject::connect(btnRemovePluginInstance, SIGNAL(clicked()),     this, SLOT(slotRemovePluginInstance()));
    QObject::connect(cbShowProgressBar,       SIGNAL(toggled(bool)), this, SLOT(slotSetDirty()));

    QObject::connect(rbAutoLoadYes,           SIGNAL(clicked(bool)), this, SLOT(slotSetDirty()));
    QObject::connect(rbAutoLoadNo,            SIGNAL(clicked(bool)), this, SLOT(slotSetDirty()));
    QObject::connect(rbAutoLoadAsk,           SIGNAL(clicked(bool)), this, SLOT(slotSetDirty()));

    QObject::connect(m_pluginInstancesModel,  SIGNAL(itemChanged      (QStandardItem *)),
                     this,                    SLOT  (slotPluginRenamed(QStandardItem *)));


    // will directly call noticePluginLibrariesChanged() and
    // noticePluginsChanged() indirectly
    slotCancel();
}


PluginManagerConfiguration::~PluginManagerConfiguration ()
{
}


void PluginManagerConfiguration::slotPluginRenamed(QStandardItem *item)
{
    if (item->column() == 1) {
        if (m_pluginItems.contains(item)) {
            PluginBase *p = m_pluginItems[item];
            p->setName(item->text());
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
        listPluginLibraries->addItem(it.key());
    }

    listPluginClasses->clear();
    const QMap<QString, PluginClassInfo> &classes = m_Application->getPluginClasses();
    QMap<QString, PluginClassInfo>::const_iterator end_cls = classes.end();
    for (QMap<QString, PluginClassInfo>::const_iterator it = classes.begin(); it != end_cls; ++it) {
        listPluginClasses->addTopLevelItem(new QTreeWidgetItem(QStringList() << it.key() << (*it).description));
    }
    listPluginClasses->resizeColumnToContents(0);
    listPluginClasses->resizeColumnToContents(1);

    noticePluginsChanged();
}


void PluginManagerConfiguration::noticePluginsChanged()
{
    // do not use clear() with QStandardItemModel, as it removes
    // also the header items!
    m_pluginInstancesModel->setRowCount(0);
    m_pluginItems      .clear();
    const PluginList &plugins = m_PluginManager->plugins();
    const QMap<QString, PluginClassInfo> &classes = m_Application->getPluginClasses();

    const Qt::ItemFlags baseFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    foreach (PluginBase *plugin, plugins) {
        QString class_name = plugin->pluginClassName();
        if (classes.contains(class_name)) {
            QString         obj_name = plugin->name();
            QList<QStandardItem *> items;
            items << new QStandardItem(class_name);
            items[0]->setFlags(baseFlags);
            items << new QStandardItem(obj_name);
            items[1]->setFlags(baseFlags | Qt::ItemIsEditable);
            items << new QStandardItem(classes[class_name].description);
            items[2]->setFlags(baseFlags);
            m_pluginInstancesModel->appendRow(items);

            m_pluginItems.insert(items[1], plugin);
        }
    }
    listPluginInstances->resizeColumnToContents(0);
    listPluginInstances->resizeColumnToContents(1);
    listPluginInstances->resizeColumnToContents(2);
}

void PluginManagerConfiguration::noticePluginRenamed(PluginBase *p, const QString &name)
{
    QStandardItem *item = m_pluginItems.key(p, NULL);
    if (item) {
        item->setText(name);
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
        // will directly call noticePluginsChanged()
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
    if (m_Application && url.pathOrUrl().length()) {
        if (!url.isLocalFile()) {
            return;  // shouldn't happen, the url requester takes care
                     // of that
        }
        m_Application->LoadLibrary(url.toLocalFile());
    }
}


void PluginManagerConfiguration::slotRemoveLibrary()
{
    slotSetDirty();
    if (m_Application) {
        QListWidgetItem *item = listPluginLibraries->currentItem();
        if (item) {
            m_Application->UnloadLibrary(item->text());
        }
    }
}


void PluginManagerConfiguration::slotNewPluginInstance()
{
    slotSetDirty();
    if (m_Application && m_PluginManager) {
        QTreeWidgetItem *item = listPluginClasses->currentItem();
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
        QModelIndex current = listPluginInstances->currentIndex();
        if (current.isValid()) {
            QStandardItem *item = m_pluginInstancesModel->item(current.row(), 1);
            m_PluginManager->deletePluginByName(item->text());
        }
    }
}


void PluginManagerConfiguration::slotSetDirty()
{
    m_dirty = true;
}


#include "pluginmanager-configuration.moc"
