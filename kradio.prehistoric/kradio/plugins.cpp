/***************************************************************************
                          plugins.cpp  -  description
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

#include "plugins.h"
#include <kdialogbase.h>

PluginBase::PluginBase()
{
	manager = NULL;
	configurationPage = NULL;
}


PluginBase::~PluginBase()
{
	if (manager)
		manager->removePlugin(this);
}


bool PluginBase::setManager (PluginManager *m)
{
	if (!manager && m) {
		manager = m;
		return true;
	} else {
		return false;
	}
}


void PluginBase::unsetManager ()
{
	if (manager) {
		PluginManager *old = manager;
		manager = 0;
		old->removePlugin(this);
	}
}


bool PluginBase::isManagerSet () const
{
	return manager != NULL;
}


QFrame *PluginBase::getConfigurationPage()
{
	return configurationPage;
}

QFrame *PluginBase::createConfigurationPage(KDialogBase *)
{
	// we do not have a config page yet
	return configurationPage = NULL;
}

QFrame *PluginBase::getAboutPage()
{
	// we do not have an about page yet
	return NULL;
}

void   PluginBase::saveState (KConfig *) const
{
	// do nothing
}


void   PluginBase::restoreState (KConfig *)
{
	// do nothing
}


/////////////////////////////////////////////////////////////////////////////

PluginManager::PluginManager()
{
}


PluginManager::~PluginManager()
{
	PluginBase *p = NULL;
	while ((p = plugins.getFirst())) {
		removePlugin(p);
	}
}


void PluginManager::insertPlugin(PluginBase *p)
{
	if (p) {
		plugins.append(p);
		p->setManager(this);
	}
}


void PluginManager::removePlugin(PluginBase *p)
{
	if (p && plugins.contains(p)) {
		plugins.remove(p);
		p->unsetManager();
	}
}


KDialogBase *PluginManager::createSetupDialog(QWidget *parent, const QString &title, bool modal)
{
	KDialogBase *sud = new KDialogBase( KDialogBase::IconList,
	                                    title,
								    	KDialogBase::Apply|KDialogBase::Ok|KDialogBase::Cancel,
    									KDialogBase::Ok,
                                        parent, title, modal, true);

	// FIXME:
	// add an own page for plugin management

	for (PluginIterator i(plugins); sud && i.current(); ++i) {
		i.current()->createConfigurationPage(sud);
	}

	return sud;
}


void PluginManager::saveState (KConfig *c) const
{
	for (PluginIterator i(plugins); i.current(); ++i) {
		i.current()->saveState(c);
	}
}


void PluginManager::restoreState (KConfig *c)
{
	for (PluginIterator i(plugins); i.current(); ++i) {
		i.current()->restoreState(c);
	}
}

