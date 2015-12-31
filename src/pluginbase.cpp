/***************************************************************************
                          plugins.cpp  -  description
                             -------------------
    begin                : Mon Mar 10 2003
    copyright            : (C) 2003 by Martin Witte
    email                : emw-kradio@nocabal.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "pluginbase.h"
#include "pluginmanager.h"


/***************************************************************************/


PluginBase::PluginBase(const QString &instance_id, const QString &name, const QString &description)
    : m_instanceID(instance_id),
      m_name(name),
      m_description(description),
      m_manager(NULL),
      m_destructorCalled(false)
{
}


// PluginBase::PluginBase(const QString &name, const QString &description)
//     : m_instanceID(generateRandomID(70)),
//       m_name(name),
//       m_description(description),
//       m_manager(NULL),
//       m_destructorCalled(false)
// {
// }


PluginBase::~PluginBase()
{
    m_destructorCalled = true;
    //IErrorLogClient::logDebug("destructing plugin " + m_name);
    unsetManager();
}

void PluginBase::setName(const QString &n)
{
    m_name = n;
    if (m_manager) {
        m_manager->noticePluginRenamed(this, m_name);
    }
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


ConfigPageInfo PluginBase::createConfigurationPage()
{
    return ConfigPageInfo();
}


void   PluginBase::saveState (KConfigGroup &c) const
{
    // just stored here for better readability of cfg file.
    c.writeEntry("plugin_instance_name", m_name);
    c.writeEntry("plugin_instance_id",   m_instanceID);
}


void   PluginBase::restoreState (const KConfigGroup &)
{
    // do nothing
    // we will not restore names or ids here since we'll get it in constructor by manager
}


void   PluginBase::startPlugin()
{
    // do nothing
}

void   PluginBase::aboutToQuit()
{
}


KRadioPluginFactoryBase::KRadioPluginFactoryBase()
{
}


KRadioPluginFactoryBase::~KRadioPluginFactoryBase()
{
}


QList<KAboutData> KRadioPluginFactoryBase::components() const
{
    return m_components;
}


void KRadioPluginFactoryBase::registerComponent(const KAboutData &aboutData)
{
    m_components.append(aboutData);
}
