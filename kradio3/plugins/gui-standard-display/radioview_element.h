/***************************************************************************
                          radioview_element.h  -  description
                             -------------------
    begin                : Fre Jun 20 2003
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

#ifndef KRADIO_RADIOVIEW_ELEMENT_H
#define KRADIO_RADIOVIEW_ELEMENT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qframe.h>
#include <kradio/interfaces/interfaces.h>
#include <kradio/libkradio/plugins.h>

enum RadioViewClass { clsRadioSound   = 0,
                      clsRadioSeek,
                      clsRadioDisplay,
                      clsClassMAX
                    };


// Defaults to an empty element
class RadioViewElement : public QFrame,
                         public virtual Interface
{
Q_OBJECT
public:
    RadioViewElement (QWidget *parent, const QString &name, RadioViewClass myClass);
    virtual ~RadioViewElement();

    bool connectI   (Interface *) { return false; }   // default behaviour, please overwrite in derived class
    bool disconnectI(Interface *) { return false; }   // default behaviour, please overwrite in derived class

    float getUsability (Interface *) const { return 0.01; } // 0 <= Usability <= 1, used to decide wich Element to use
                                                            // should be overwritten ;)

    RadioViewClass getClass() const { return myClass; }

    // Configuration ??
    virtual ConfigPageInfo  createConfigurationPage() { return ConfigPageInfo(); }

    virtual void   saveState (KConfig *) const {}
    virtual void   restoreState (KConfig *)    {}


protected :

    RadioViewClass  myClass;
};


#endif
