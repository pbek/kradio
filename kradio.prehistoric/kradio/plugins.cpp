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
#include "pluginmanager.h"

#include <kdialogbase.h>
#include <kdebug.h>


PluginBase::PluginBase(const QString &name)
	: m_name(name),
	  m_manager(NULL)
{
}


PluginBase::~PluginBase()
{
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


void   PluginBase::saveState (KConfig *) const
{
	// do nothing
}


void   PluginBase::restoreState (KConfig *)
{
	// do nothing
}


///////////////////////////////////////////////////////////////////////////

QWidget *WidgetPluginBase::getWidget()
{
	return dynamic_cast<QWidget*>(this);
}


void WidgetPluginBase::notifyManager(bool shown)
{
	if (m_manager)
		m_manager->noticeWidgetPluginShown(this, shown);
}
