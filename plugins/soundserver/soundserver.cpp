/***************************************************************************
                          soundserver.cpp  -  description
                             -------------------
    begin                : Sun Apr 17 2005
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

#include "soundserver.h"

#include <kapplication.h>
#include <klocale.h>


///////////////////////////////////////////////////////////////////////
//// plugin library functions

PLUGIN_LIBRARY_FUNCTIONS(SoundServer, PROJECT_NAME, i18n("KRadio internal sound server"));

/////////////////////////////////////////////////////////////////////////////

SoundServer::SoundServer(const QString &instanceID, const QString &name)
    : PluginBase(instanceID, name, i18n("Sound Server Plugin"))
{
    logDebug(i18n("initializing KRadio sound server"));
}

SoundServer::~SoundServer()
{
}

bool SoundServer::connectI (Interface *i)
{
    bool a = PluginBase::connectI(i);
    bool b = ISoundStreamServer::connectI(i);
    return a || b;
}


bool SoundServer::disconnectI (Interface *i)
{
    bool a = PluginBase::disconnectI(i);
    bool b = ISoundStreamServer::disconnectI(i);
    return a || b;
}



void   SoundServer::saveState (KConfigGroup &config) const
{
    PluginBase::saveState(config);
}

void   SoundServer::restoreState (const KConfigGroup &config)
{
    PluginBase::restoreState(config);
}

ConfigPageInfo SoundServer::createConfigurationPage()
{
    return ConfigPageInfo ();
}

// AboutPageInfo SoundServer::createAboutPage()
// {
//     return AboutPageInfo();
// }
