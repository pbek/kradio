/***************************************************************************
                          plugins.h  -  description
                             -------------------
    begin                : Mon Mär 10 2003
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

/////////////////////////////////////////////////////////////////////////////

#ifndef KRADIO_PLUGINS_INTERFACES_H
#define KRADIO_PLUGINS_INTERFACES_H

#include "interfaces.h"

class PluginManager;
class KDialogBase;
class QWidget;
class QFrame;
class KConfig;

class PluginBase : public Interface
{
public :
	         PluginBase();
	virtual ~PluginBase();


	// Only the plugin itself knows what interfaces it implements. Thus it has
	// to call the apropriate InterfaceBase::establishConnection methods

	virtual void connect    (PluginBase *p) = 0;
	virtual void disconnect (PluginBase *p) = 0;

	// interaction with pluginmanager

	virtual bool setManager (PluginManager *);
	virtual void unsetManager ();
	virtual bool isManagerSet () const;

	////////////////////////////////////

	virtual QFrame *createConfigurationPage(KDialogBase *);
	virtual QFrame *getConfigurationPage();
	virtual QFrame *getAboutPage();


	virtual void   saveState (KConfig *) const;
	virtual void   restoreState (KConfig *);


protected :

    PluginManager *manager;
    QFrame        *configurationPage;
};


/////////////////////////////////////////////////////////////////////////////

class PluginManager
{
public :
	         PluginManager();
	virtual ~PluginManager();


	// managing plugins
	
	virtual void         insertPlugin(PluginBase *);
	virtual void         removePlugin(PluginBase *);

	// operations on all plugins

	virtual KDialogBase* createSetupDialog(QWidget *parent = 0,
		                                   const QString &title = QString(),
		                                   bool modal = true);
		                                   
	virtual void         saveState (KConfig *) const;
	virtual void         restoreState (KConfig *);

protected :

    typedef QPtrList<PluginBase>           PluginList;
    typedef QPtrListIterator<PluginBase>   PluginIterator;

    PluginList   plugins;
};


#endif
