/***************************************************************************
                          radioview_element.h  -  description
                             -------------------
    begin                : Fre Jun 20 2003
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

#ifndef KRADIO_RADIOVIEW_ELEMENT_H
#define KRADIO_RADIOVIEW_ELEMENT_H

#include <QFrame>
#include "interfaces.h"
#include "pluginbase.h"

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

    virtual bool connectI   (Interface *)  override { return false; }   // default behaviour, please overwrite in derived class
    virtual bool disconnectI(Interface *)  override { return false; }   // default behaviour, please overwrite in derived class

    virtual float getUsability (Interface *) const { return 0.00; } // 0 <= Usability <= 1, used to decide wich Element to use
                                                            // should be overwritten ;)

    virtual RadioViewClass getClass() const { return myClass; }

    // Configuration ??
    virtual ConfigPageInfo  createConfigurationPage() { return ConfigPageInfo(); }

    virtual void   saveState   (      KConfigGroup &) const {}
    virtual void   restoreState(const KConfigGroup &)       {}


protected :

    RadioViewClass  myClass;
};


#endif
