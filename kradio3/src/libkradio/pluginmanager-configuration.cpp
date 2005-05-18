/***************************************************************************
                       pluginmanager-configuration.cpp  -  description
                             -------------------
    begin                : Thu Sep 30 2004
    copyright            : (C) 2004 by Martin Witte
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

#include "pluginmanager-configuration-ui.h"
#include "pluginmanager-configuration.h"
#include "kradioapp.h"
#include "pluginmanager.h"

#include <klistbox.h>
#include <klistview.h>
#include <kpushbutton.h>
#include <kurlrequester.h>
#include <kinputdialog.h>

PluginManagerConfiguration::PluginManagerConfiguration(QWidget *parent, KRadioApp *app, PluginManager *pm)
  : PluginManagerConfigurationUI(parent),
    m_Application(app),
    m_PluginManager(pm)
{
    noticePluginLibrariesChanged();
    noticePluginsChanged();

    QObject::connect(btnAddLibrary,           SIGNAL(clicked()), this, SLOT(slotAddLibrary()));
    QObject::connect(btnRemoveLibrary,        SIGNAL(clicked()), this, SLOT(slotRemoveLibrary()));
    QObject::connect(btnNewPluginInstance,    SIGNAL(clicked()), this, SLOT(slotNewPluginInstance()));
    QObject::connect(btnRemovePluginInstance, SIGNAL(clicked()), this, SLOT(slotRemovePluginInstance()));
}


PluginManagerConfiguration::~PluginManagerConfiguration ()
{
}


void PluginManagerConfiguration::noticePluginLibrariesChanged()
{
    listPluginLibraries->clear();
    const QMap<QString, PluginLibraryInfo> &libs = m_Application->getPluginLibraries();
    for (QMapConstIterator<QString,PluginLibraryInfo> it = libs.begin(); it != libs.end(); ++it) {
        listPluginLibraries->insertItem(it.key());
    }

    listPluginClasses->clear();
    const QMap<QString, PluginClassInfo> &classes = m_Application->getPluginClasses();
    for (QMapConstIterator<QString, PluginClassInfo> it = classes.begin(); it != classes.end(); ++it) {
        listPluginClasses->insertItem(new KListViewItem(listPluginClasses, it.key(), (*it).description));
    }

    noticePluginsChanged();
}


void PluginManagerConfiguration::noticePluginsChanged()
{
    listPluginInstances->clear();
    const PluginList &plugins = m_PluginManager->plugins();
    const QMap<QString, PluginClassInfo> &classes = m_Application->getPluginClasses();

    for (PluginIterator it(plugins); it.current(); ++it) {
        QString class_name = it.current()->pluginClassName();
        if (classes.contains(class_name)) {
            QString obj_name   = it.current()->name();
            listPluginInstances->insertItem(new KListViewItem(listPluginInstances, class_name, obj_name, classes[class_name].description));
        }
    }
}


void PluginManagerConfiguration::slotOK()
{
}


void PluginManagerConfiguration::slotCancel()
{
    noticePluginLibrariesChanged();
    noticePluginsChanged();
}


void PluginManagerConfiguration::slotAddLibrary()
{
    QString url = editPluginLibrary->url();
    if (m_Application && url.length())
        m_Application->LoadLibrary(url);
}


void PluginManagerConfiguration::slotRemoveLibrary()
{
    if (m_Application) {
        QString lib = listPluginLibraries->currentText();
        if (lib.length()) {
            m_Application->UnloadLibrary(lib);
        }
    }
}


void PluginManagerConfiguration::slotNewPluginInstance()
{
    if (m_Application && m_PluginManager) {
        QListViewItem *item = listPluginClasses->currentItem();
        QString class_name = item ? item->text(0) : QString::null;
        bool ok = false;
        int default_object_id = 1;
        while (m_PluginManager->getPluginByName(class_name + QString::number(default_object_id)))
            ++default_object_id;

        QString object_name = KInputDialog::getText(i18n("Enter Plugin Instance Name"),
                                                    i18n("Instance Name"),
                                                    class_name + QString::number(default_object_id),
                                                    &ok);
        if (ok && class_name.length() && object_name.length())
            m_Application->CreatePlugin(m_PluginManager, class_name, object_name);
    }
}


void PluginManagerConfiguration::slotRemovePluginInstance()
{
    if (m_Application && m_PluginManager) {
        QListViewItem *item = listPluginInstances->currentItem();
        QString object_name = item ? item->text(1) : QString::null;
        if (object_name.length())
            m_PluginManager->deletePluginByName(object_name);
    }
}


#include "pluginmanager-configuration.moc"
