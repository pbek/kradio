/***************************************************************************
                          plugins.cpp  -  description
                             -------------------
    begin                : Mon M�r 10 2003
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

PluginBase::PluginBase(const QString &name)
	: QObject(NULL, name),
      m_manager(NULL)
{
}


PluginBase::~PluginBase()
{
	while (m_guiElements.first()) {
		delete m_guiElements.first();
	}
	unsetManager();
}


bool PluginBase::setManager (PluginManager *m)
{
	if (!m_manager && m) {
		m_manager = m;
		return true;
	} else {
		return false;
	}
}


void PluginBase::unsetManager ()
{
	if (m_manager) {
		PluginManager *old = m_manager;
		m_manager = NULL;
		old->removePlugin(this);
	}
}


bool PluginBase::isManagerSet () const
{
	return m_manager != NULL;
}


QFrame *PluginBase::createConfigurationPage(KDialogBase *dlg)
{
	QFrame *f = internal_createConfigurationPage(dlg);
	if (f)
		registerGuiElement(f);
	return f;
}

QFrame *PluginBase::createAboutPage(QWidget *parent)
{
	QFrame *f = internal_createAboutPage(parent);
	if (f)
		registerGuiElement(f);
	return f;
}

void   PluginBase::saveState (KConfig *) const
{
	// do nothing
}


void   PluginBase::restoreState (KConfig *)
{
	// do nothing
}

void PluginBase::registerGuiElement(QObject *o)
{
	if (o) {
		m_guiElements.append(o);
		QObject::connect(o, SIGNAL(destroyed(QObject *)), this, SLOT(unregisterGuiElement(QObject *)));
	}
}

void PluginBase::unregisterGuiElement(QObject *o)
{
	if (o) {
		m_guiElements.removeRef(o);
	}
}



