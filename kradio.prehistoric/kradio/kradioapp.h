/***************************************************************************
                          kradioapp.h  -  description
                             -------------------
    begin                : Sa Feb  9 2002
    copyright            : (C) 2002 by Klas Kalass / Martin Witte / Frank Schwanz
    email                : klas.kalass@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRADIO_KRADIOAPP_H
#define KRADIO_KRADIOAPP_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qdict.h>

#include <kapplication.h>
#include <kaboutapplication.h>
#include "pluginmanager.h"
#include "plugins.h"

class KRadioAbout : public PluginBase
{
public:
    KRadioAbout(const QString &name) : PluginBase(name, "KRadio Application") {}

    virtual ConfigPageInfo createConfigurationPage () { return ConfigPageInfo(); }
    virtual AboutPageInfo  createAboutPage ();

    virtual void   saveState    (KConfig *) const {}
    virtual void   restoreState (KConfig *)       {}
};


class KRadioApp : public KApplication
{
Q_OBJECT
public:
    KRadioApp();
    virtual ~KRadioApp();

    virtual void             saveState    (KConfig *c) const;
    virtual void             restoreState (KConfig *c);

    virtual PluginManager   *createNewInstance(const QString &name);

protected:

    QDict<PluginManager>     m_Instances;
};


#endif
