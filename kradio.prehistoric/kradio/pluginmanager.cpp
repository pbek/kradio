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

#include "plugins.h"
#include "pluginmanager.h"
#include <kdialogbase.h>
#include <kdebug.h>

PluginManager::PluginManager()
{
}


PluginManager::~PluginManager()
{
	PluginBase *p = NULL;
	while ((p = m_plugins.getFirst())) {
		removePlugin(p);
	}
}


void PluginManager::insertPlugin(PluginBase *p)
{
	if (p) {
		m_plugins.append(p);
		p->setManager(this);

		for (PluginIterator it(m_plugins); it.current(); ++it) {
			if (it.current() != p) {
				bool r = p->connect(it.current());
				kdDebug() << "PluginManager::insertPlugin: "
				          << p->name() << " <-> " << it.current()->name()
				          << ": "
				          << (r ? "ok" : "failed") << endl;
			}
		}
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
				bool r = p->disconnect(it.current());
				kdDebug() << "PluginManager::removePlugin: "
				          << p->name() << " <-> " << it.current()->name()
				          << ": "
				          << (r ? "ok" : "failed") << endl;
			}
		}
		
		m_plugins.remove(p);
		p->unsetManager();
	}
}


KDialogBase *PluginManager::createSetupDialog(QWidget *parent,
                                              const QString &title,
                                              bool modal)
{
	KDialogBase *sud = new KDialogBase( KDialogBase::IconList,
	                                    title,
								    	KDialogBase::Apply|KDialogBase::Ok|KDialogBase::Cancel,
    									KDialogBase::Ok,
                                        parent, title, modal, true);

	// FIXME:
	// add an own page for plugin management

	for (PluginIterator i(m_plugins); sud && i.current(); ++i) {
		i.current()->createConfigurationPage(sud);
	}

	return sud;
}


void PluginManager::saveState (KConfig *c) const
{
	for (PluginIterator i(m_plugins); i.current(); ++i) {
		i.current()->saveState(c);
	}
}


void PluginManager::restoreState (KConfig *c)
{
	for (PluginIterator i(m_plugins); i.current(); ++i) {
		i.current()->restoreState(c);
	}
}

