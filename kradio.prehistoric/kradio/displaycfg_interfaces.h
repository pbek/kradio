/***************************************************************************
                          displaycfg_interfaces.h  -  description
                             -------------------
    begin                : Fr Aug 15 2003
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

#ifndef KRADIO_DISPLAYCFG_INTERFACES_H
#define KRADIO_DISPLAYCFG_INTERFACES_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "interfaces.h"

class QColor;


///////////////////////////////////////////////////////////////////////


INTERFACE(IDisplayCfg, IDisplayCfgClient)
{
public :
	IF_CON_DESTRUCTOR(IDisplayCfg, -1)

RECEIVERS:
	IF_RECEIVER(  setColors(const QColor &activeColor, const QColor &bkgnd) )

SENDERS:
	IF_SENDER  (  notifyColorsChanged(const QColor &activeColor, const QColor &bkgnd) )

ANSWERS:
	IF_ANSWER  (  QColor   getActiveColor() const )
	IF_ANSWER  (  QColor   getBkgndColor() const )

};


INTERFACE(IDisplayCfgClient, IDisplayCfg)
{
friend class IDisplayCfg;

public :
	IF_CON_DESTRUCTOR(IDisplayCfgClient, 1)

SENDERS:
	IF_SENDER  (  sendColors(const QColor &activeColor, const QColor &bkgnd) )

RECEIVERS:
	IF_RECEIVER(  noticeColorsChanged(const QColor &activeColor, const QColor &bkgnd) )

QUERIES:
	IF_QUERY   (  QColor   queryActiveColor() )
	IF_QUERY   (  QColor   queryBkgndColor()  )

RECEIVERS:
	virtual void noticeConnected    (cmplInterface *, bool pointer_valid);
	virtual void noticeDisconnected (cmplInterface *, bool pointer_valid);
};


#endif
