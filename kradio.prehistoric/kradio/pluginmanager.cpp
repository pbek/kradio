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
#include <kiconloader.h>

#include <qlayout.h>
#include <qframe.h>

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
	m_configPageFrames.clear();
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
		
		addConfigurationPage (p, p->createConfigurationPage());

		// connect plugins with each other
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

		// perhaps some existing config pages will profit from new plugin
		// example: timecontrol profits from radio plugin
		for (QWidgetDictIterator it(m_configPages); it.current(); ++it) {
			Interface *i = dynamic_cast<Interface *>(it.current());
			if (i)
				i->connect(p);
		}
	}
	for (PluginIterator it(m_plugins); it.current(); ++it) {
		it.current()->noticePluginsChanged(m_plugins);
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
		while (QFrame *f = m_configPageFrames.find(p)) {
			m_configPageFrames.remove(p);
			m_configPages.remove(p);
			delete f;
		}
		
		// remove bindings between me and plugin
		m_plugins.remove(p);
		p->unsetManager();

		p->noticePluginsChanged(PluginList());
		for (PluginIterator it(m_plugins); it.current(); ++it) {
			it.current()->noticePluginsChanged(m_plugins);
		}
	}
}


void PluginManager::addConfigurationPage (PluginBase *forWhom,
										  const ConfigPageInfo &info)
{
	if (   !forWhom || !m_plugins.containsRef(forWhom)
	    || !m_configDialog || !info.configPage)
		return;

	// create empty config frame
	QFrame *f = m_configDialog->addPage(
	  info.itemName,
	  info.pageHeader,
      KGlobal::instance()->iconLoader()->loadIcon( info.iconName, KIcon::NoGroup, KIcon::SizeMedium )
    );

    // register this frame and config page
	m_configPageFrames.insert(forWhom, f);
	m_configPages.insert(forWhom, info.configPage);

	// fill config frame with layout ...
    QGridLayout *l = new QGridLayout(f);
    l->setSpacing( 0 );
    l->setMargin( 0 );

    // ... and externally created config page
    info.configPage->reparent (f, QPoint(0,0), true);
    l->addWidget( info.configPage, 0, 0 );

	// perhaps new config page profits from existing plugins
	// example: timecontrol profits from radio plugin
    Interface *i = dynamic_cast<Interface *>(info.configPage);
    if (i) {
		for (PluginIterator it(m_plugins); it.current(); ++it)
			i->connect(it.current());
    }

    // make sure, that config page receives ok, apply and cancel signals
	QObject::connect(m_configDialog, SIGNAL(okClicked()),     info.configPage, SLOT(slotOK()));
	QObject::connect(m_configDialog, SIGNAL(applyClicked()),  info.configPage, SLOT(slotOK()));
	QObject::connect(m_configDialog, SIGNAL(cancelClicked()), info.configPage, SLOT(slotCancel()));
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
		addConfigurationPage(i.current(),
		                     i.current()->createConfigurationPage());
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

