/***************************************************************************
                          soundserver.cpp  -  description
                             -------------------
    begin                : Sun Apr 17 2005
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

#include "soundserver.h"

#include <kapplication.h>
#include <klocale.h>


///////////////////////////////////////////////////////////////////////
//// plugin library functions

PLUGIN_LIBRARY_FUNCTIONS(SoundServer, "kradio-soundserver", i18n("SoundServer"));

/////////////////////////////////////////////////////////////////////////////

SoundServer::SoundServer(const QString &name)
    : PluginBase(name, i18n("SoundServer Plugin"))
{
    logDebug(i18n("initializing kradio soundserver"));
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



void   SoundServer::saveState (KConfig *) const
{
}

void   SoundServer::restoreState (KConfig *)
{
}

ConfigPageInfo SoundServer::createConfigurationPage()
{
    return ConfigPageInfo ();
}

AboutPageInfo SoundServer::createAboutPage()
{
    return AboutPageInfo();
}
