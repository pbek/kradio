/***************************************************************************
                          soundserver.h  -  description
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

#ifndef KRADIO_SOUNDSERVER_H
#define KRADIO_SOUNDSERVER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../../src/interfaces/soundstreamclient_interfaces.h"
#include "../../src/libkradio/plugins.h"


class SoundServer : public PluginBase,
                    public ISoundStreamServer
{
public:
    SoundServer(const QString &name);
    ~SoundServer();

    virtual bool connectI (Interface *);
    virtual bool disconnectI (Interface *);

    virtual QString pluginClassName() const { return "SoundServer"; }

    virtual const QString &name() const { return PluginBase::name(); }
    virtual       QString &name()       { return PluginBase::name(); }

    // PluginBase

public:
    virtual void   saveState (KConfig *) const;
    virtual void   restoreState (KConfig *);

    virtual ConfigPageInfo  createConfigurationPage();
    virtual AboutPageInfo   createAboutPage();
};

#endif
