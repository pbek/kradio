/***************************************************************************
                          pluginmanager-configuration.h  -  description
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

#ifndef KRADIO_PLUGINMANAGER_CONFIGURATION_H
#define KRADIO_PLUGINMANAGER_CONFIGURATION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <pluginmanager-configuration-ui.h>

class QWidget;
class KRadioApp;
class PluginManager;

class PluginManagerConfiguration : public PluginManagerConfigurationUI
{
Q_OBJECT
public :
    PluginManagerConfiguration (QWidget *parent, KRadioApp *app, PluginManager *pm);
    ~PluginManagerConfiguration ();

    void noticePluginLibrariesChanged();
    void noticePluginsChanged();

protected slots:

    void slotOK();
    void slotCancel();

    void slotAddLibrary();
    void slotRemoveLibrary();
    void slotNewPluginInstance();
    void slotRemovePluginInstance();
    void slotSetDirty();

protected:

    KRadioApp     *m_Application;
    PluginManager *m_PluginManager;
    bool           m_dirty;

};

#endif
