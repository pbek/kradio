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
#include "plugin_configuration_dialog.h"
#include <kdebug.h>

PluginManager::PluginManager(const QString &configDialogTitle)
 : m_configDialog (NULL)
{
	createConfigDialog(configDialogTitle);
}


PluginManager::~PluginManager()
{
	// config Dialog must be deleted first, so we can clear m_configPages
	// without problems (this is the only place where our config dialog is deleted)
	// Without clearing this list, those pages would be deleted, but
	// we would try to delete them another time when the associated plugin is
	// deleted, because m_configPages is out of date.
	delete m_configDialog;
	m_configPages.clear();
	m_configDialog = NULL;

	while (PluginBase *p = m_plugins.getFirst()) {
		deletePlugin(p);
	}
}


void PluginManager::noticeWidgetPluginShown(WidgetPluginBase *p, bool shown)
{
	for (PluginIterator it(m_plugins); it.current(); ++it) {
		it.current()->noticeWidgetPluginShown(p, shown);
	}
}


void PluginManager::insertPlugin(PluginBase *p)
{
	if (p) {
		m_plugins.append(p);
		p->setManager(this);
		p->createConfigurationPage();

		for (PluginIterator it(m_plugins); it.current(); ++it) {
			if (it.current() != p) {
/*				bool r = */
                p->connect(it.current());
/*				kdDebug() << "PluginManager::insertPlugin: "
				          << p->name() << " <-> " << it.current()->name()
				          << ": "
				          << (r ? "ok" : "failed") << endl;
*/
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
/*				bool r = */
                p->disconnect(it.current());
/*				kdDebug() << "PluginManager::removePlugin: "
				          << p->name() << " <-> " << it.current()->name()
				          << ": "
				          << (r ? "ok" : "failed") << endl;
*/			}
		}

		// remove config page from config dialog, only chance is to delete it
		// plugin will be notified automatically (mechanism implemented by
		// PluginBase)
		while (QFrame *f = m_configPages.find(p)) {
			m_configPages.remove(p);
			delete f;			
		}
		
		// remove bindings between me and plugin
		m_plugins.remove(p);
		p->unsetManager();
	}
}


QFrame *PluginManager::addConfigurationPage (PluginBase *forWhom,
                                             const QString &itemname,
                                             const QString &header,
                                             const QPixmap &icon)
{
	if (!forWhom || !m_plugins.containsRef(forWhom) || !m_configDialog)
		return NULL;
		
	QFrame *f = m_configDialog->addPage( itemname, header, icon );
	m_configPages.insert(forWhom, f);
	return f;
}


void PluginManager::connectWithConfigDialog(QObject *o)
{
	if (m_configDialog) {
		QObject::connect(m_configDialog, SIGNAL(okClicked()),     o, SLOT(slotOk()));
		QObject::connect(m_configDialog, SIGNAL(applyClicked()),  o, SLOT(slotOk()));
		QObject::connect(m_configDialog, SIGNAL(cancelClicked()), o, SLOT(slotCancel()));
	}
}


void PluginManager::createConfigDialog(const QString &title)
{
	PluginConfigurationDialog *cfg = new PluginConfigurationDialog(
	    KDialogBase::IconList,
	    title,
		KDialogBase::Apply|KDialogBase::Ok|KDialogBase::Cancel,
    	KDialogBase::Ok,
        /*parent = */ NULL,
        title,
        /*modal = */ false,
        true);

    m_configDialog = cfg;

    insertPlugin(cfg);

	// FIXME:
	// add an own page for plugin management

	for (PluginIterator i(m_plugins); m_configDialog && i.current(); ++i) {
		i.current()->createConfigurationPage();
	}
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

