/***************************************************************************
                          pluginmanager-configuration.h  -  description
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

#ifndef KRADIO_PLUGINMANAGER_CONFIGURATION_H
#define KRADIO_PLUGINMANAGER_CONFIGURATION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

class QColorGroup;
#include "ui_pluginmanager-configuration-ui.h"

class QWidget;
class KRadioApp;
class PluginManager;
class PluginBase;

class PluginManagerConfiguration : public QWidget,
                                   public Ui_PluginManagerConfigurationUI
{
Q_OBJECT
public :
    PluginManagerConfiguration (QWidget *parent, KRadioApp *app, PluginManager *pm);
    ~PluginManagerConfiguration ();

    void noticePluginLibrariesChanged();
    void noticePluginsChanged();
    void noticePluginRenamed(PluginBase *p, const QString &name);

public slots:

    void slotOK();
    void slotCancel();
    void slotSetDirty();

protected slots:

    void slotAddLibrary();
    void slotRemoveLibrary();
    void slotNewPluginInstance();
    void slotRemovePluginInstance();
    void slotPluginRenamed(Q3ListViewItem *item, int col, const QString &name);

protected:

    KRadioApp             *m_Application;
    PluginManager         *m_PluginManager;
    bool                   m_dirty;

    QMap<Q3ListViewItem*, PluginBase*> m_pluginItems;         // listviewitem => instanceID
};

#endif
