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

#include "interfaces.h"
#include <qstring.h>

class PluginBase;
class KDialogBase;
class QWidget;
class KConfig;

class PluginManager
{
public :
	         PluginManager();
	virtual ~PluginManager();


	// managing plugins

	// after insert, pluginManager is responsible for deletion
	virtual void         insertPlugin(PluginBase *);

	// remove and delete plugin
	virtual void         deletePlugin(PluginBase *);
	
	// remove plugin, afterwards pluginManager is no longer responsible for deletion
	virtual void         removePlugin(PluginBase *);

	// operations on all plugins

	virtual KDialogBase* createSetupDialog(QWidget *parent = 0,
		                                   const QString &title = QString(),
		                                   bool modal = true);

	virtual void         saveState    (KConfig *) const;
	virtual void         restoreState (KConfig *);

protected :

    typedef QPtrList<PluginBase>           PluginList;
    typedef QPtrListIterator<PluginBase>   PluginIterator;

    PluginList   m_plugins;
};

#endif
