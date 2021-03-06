/***************************************************************************
                          displaycfg_interfaces.h  -  description
                             -------------------
    begin                : Fr Aug 15 2003
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

#ifndef KRADIO_DISPLAYCFG_INTERFACES_H
#define KRADIO_DISPLAYCFG_INTERFACES_H

#include "interfaces.h"

#include <QFont>
#include <QColor>

///////////////////////////////////////////////////////////////////////


INTERFACE(IDisplayCfg, IDisplayCfgClient)
{
public :
    IF_CON_DESTRUCTOR(IDisplayCfg, -1)

RECEIVERS:
    IF_RECEIVER(  setDisplayColors(const QColor &activeColor, const QColor &inactiveColor, const QColor &bkgnd) )
    IF_RECEIVER(  setDisplayFont  (const QFont &f)                                                             )

SENDERS:
    IF_SENDER  (  notifyDisplayColorsChanged(const QColor &activeColor, const QColor &inactiveColor, const QColor &bkgnd) )
    IF_SENDER  (  notifyDisplayFontChanged(const QFont &f)                                                               )

ANSWERS:
    IF_ANSWER  (  const QColor   &getDisplayActiveColor() const )
    IF_ANSWER  (  const QColor   &getDisplayInactiveColor() const )
    IF_ANSWER  (  const QColor   &getDisplayBkgndColor() const )
    IF_ANSWER  (  const QFont    &getDisplayFont() const )

};


INTERFACE(IDisplayCfgClient, IDisplayCfg)
{
friend class IDisplayCfg;

public :
    IF_CON_DESTRUCTOR(IDisplayCfgClient, 1)

SENDERS:
    IF_SENDER  (  sendDisplayColors(const QColor &activeColor, const QColor &inactiveColor, const QColor &bkgnd) )
    IF_SENDER  (  sendDisplayFont  (const QFont &f)                                                             )

RECEIVERS:
    IF_RECEIVER(  noticeDisplayColorsChanged(const QColor &activeColor, const QColor &inactiveColor, const QColor &bkgnd) )
    IF_RECEIVER(  noticeDisplayFontChanged(const QFont &f)                                                               )

QUERIES:
    IF_QUERY   (  const QColor   &queryDisplayActiveColor() )
    IF_QUERY   (  const QColor   &queryDisplayInactiveColor() )
    IF_QUERY   (  const QColor   &queryDisplayBkgndColor()  )
    IF_QUERY   (  const QFont    &queryDisplayFont()  )

RECEIVERS:
    virtual void noticeConnectedI    (cmplInterface *, bool pointer_valid) override;
    virtual void noticeDisconnectedI (cmplInterface *, bool pointer_valid) override;
};


#endif
