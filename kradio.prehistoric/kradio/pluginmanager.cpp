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
#include <kdialogbase.h>

#include <qlayout.h>
#include <qframe.h>

PluginManager::PluginManager(const QString &configDialogTitle, const QString &aboutDialogTitle)
 : m_configDialog (NULL),
   m_aboutDialog(NULL),
   m_configDialogTitle(configDialogTitle),
   m_aboutDialogTitle (aboutDialogTitle)
{
}


PluginManager::~PluginManager()
{
	// config Dialog must be deleted first, so we can clear m_configPages
	// without problems (this is the only place where our config dialog is deleted)
	// Without clearing this list, those pages would be deleted, but
	// we would try to delete them another time when the associated plugin is
	// deleted, because m_configPages is out of date.
	if (m_configDialog) {
		m_configDialog->cancel();
		delete m_configDialog;
	}
	m_configPages.clear();
	m_configPageFrames.clear();
	m_configDialog = NULL;

	if (m_aboutDialog)
		delete m_aboutDialog;
	m_aboutPages.clear();
	m_aboutPageFrames.clear();
	m_aboutDialog = NULL;

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
	    if (!m_configDialog)
			createConfigDialog(m_configDialogTitle);
	    if (!m_aboutDialog)
			createAboutDialog(m_aboutDialogTitle);
	
		m_plugins.append(p);
		p->setManager(this);
		
		addConfigurationPage (p, p->createConfigurationPage());
		addAboutPage         (p, p->createAboutPage());

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
	WidgetPluginBase *w1 = dynamic_cast<WidgetPluginBase*>(p);
	for (PluginIterator it(m_plugins); it.current(); ++it) {
		it.current()->noticePluginsChanged(m_plugins);
		if (w1)
			it.current()->noticeWidgetPluginShown(w1, w1->isReallyVisible());

		WidgetPluginBase *w2 = dynamic_cast<WidgetPluginBase*>(it.current());
		if (w2)
			p->noticeWidgetPluginShown(w2, w2->isReallyVisible());
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
		while (QFrame *f = m_aboutPageFrames.find(p)) {
			m_aboutPageFrames.remove(p);
			m_aboutPages.remove(p);
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
	if (!m_configDialog)
		createConfigDialog(m_configDialogTitle);

	if (   !forWhom || !m_plugins.containsRef(forWhom)
	    || !m_configDialog || !info.page)
		return;

	// create empty config frame
	QFrame *f = m_configDialog->addPage(
	  info.itemName,
	  info.pageHeader,
      KGlobal::instance()->iconLoader()->loadIcon( info.iconName, KIcon::NoGroup, KIcon::SizeMedium )
    );

    // register this frame and config page
	m_configPageFrames.insert(forWhom, f);
	m_configPages.insert(forWhom, info.page);

	// fill config frame with layout ...
    QGridLayout *l = new QGridLayout(f);
    l->setSpacing( 0 );
    l->setMargin( 0 );

    // ... and externally created config page
    info.page->reparent (f, QPoint(0,0), true);
    l->addWidget( info.page, 0, 0 );

	// perhaps new config page profits from existing plugins
	// example: timecontrol profits from radio plugin
    Interface *i = dynamic_cast<Interface *>(info.page);
    if (i) {
		for (PluginIterator it(m_plugins); it.current(); ++it)
			i->connect(it.current());
    }

    // make sure, that config page receives ok, apply and cancel signals
	QObject::connect(m_configDialog, SIGNAL(okClicked()),     info.page, SLOT(slotOK()));
	QObject::connect(m_configDialog, SIGNAL(applyClicked()),  info.page, SLOT(slotOK()));
	QObject::connect(m_configDialog, SIGNAL(cancelClicked()), info.page, SLOT(slotCancel()));
}


void PluginManager::createConfigDialog(const QString &title)
{
    if (m_configDialog) delete m_configDialog;
    m_configDialog = NULL;
    
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


void PluginManager::addAboutPage (PluginBase *forWhom,
								  const AboutPageInfo &info)
{
	if (!m_aboutDialog)
		createAboutDialog(m_aboutDialogTitle);
		
	if (   !forWhom || !m_plugins.containsRef(forWhom)
	    || !m_aboutDialog || !info.page)
		return;


	// create empty about frame
	QFrame *f = m_aboutDialog->addPage(
	  info.itemName,
	  info.pageHeader,
      KGlobal::instance()->iconLoader()->loadIcon( info.iconName, KIcon::NoGroup, KIcon::SizeMedium )
    );

    // register this frame and config page
	m_aboutPageFrames.insert(forWhom, f);
	m_aboutPages.insert(forWhom, info.page);

	// fill config frame with layout ...
    QGridLayout *l = new QGridLayout(f);
    l->setSpacing( 0 );
    l->setMargin( 0 );

    // ... and externally created config page
    info.page->reparent (f, QPoint(0,0), true);
    l->addWidget( info.page, 0, 0 );
}


void PluginManager::createAboutDialog(const QString &title)
{
    if (m_aboutDialog) delete m_aboutDialog;
    m_aboutDialog = NULL;

    m_aboutDialog = new KDialogBase(KDialogBase::IconList,
								    title,
									KDialogBase::Close,
									KDialogBase::Close,
									/*parent = */ NULL,
									title,
									/*modal = */ false,
									true);

	for (PluginIterator i(m_plugins); m_aboutDialog && i.current(); ++i) {
		addAboutPage(i.current(),
				     i.current()->createAboutPage());
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

PluginConfigurationDialog *PluginManager::getConfigDialog()
{
	if (!m_configDialog)
		createConfigDialog();
	return m_configDialog;
}

KDialogBase               *PluginManager::getAboutDialog()
{
	if (!m_aboutDialog)
		createAboutDialog();
	return m_aboutDialog;
}
