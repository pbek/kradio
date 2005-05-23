/***************************************************************************
                          displaycfg_interfaces.cpp  -  description
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

#include "displaycfg_interfaces.h"

// IDisplayCfg

IF_IMPL_SENDER  (  IDisplayCfg::notifyDisplayColorsChanged(const QColor &a, const QColor &b, const QColor &c),
                   noticeDisplayColorsChanged(a,b, c)                                          )

IF_IMPL_SENDER  (  IDisplayCfg::notifyDisplayFontChanged(const QFont &f),
                   noticeDisplayFontChanged(f)                                          )

// IDisplayCfgClient

IF_IMPL_SENDER  (  IDisplayCfgClient::sendDisplayColors(const QColor &a, const QColor &b, const QColor &c),
                   setDisplayColors(a,b, c)                                                    )

IF_IMPL_SENDER  (  IDisplayCfgClient::sendDisplayFont(const QFont &f),
                   setDisplayFont(f)                                                           )

const QColor default_display_active_color = QColor(20, 244, 20);
IF_IMPL_QUERY   (  const QColor &IDisplayCfgClient::queryDisplayActiveColor(),
                   getDisplayActiveColor(),
                   default_display_active_color                                               )

const QColor default_display_inactive_color = QColor(10, 117, 10).light(75);
IF_IMPL_QUERY   (  const QColor &IDisplayCfgClient::queryDisplayInactiveColor(),
                   getDisplayInactiveColor(),
                   default_display_inactive_color                                     )

const QColor default_display_bkgnd_color = QColor(10, 117, 10);
IF_IMPL_QUERY   (  const QColor &IDisplayCfgClient::queryDisplayBkgndColor(),
                   getDisplayBkgndColor(),
                   default_display_bkgnd_color                                               )

const QFont default_display_font = QFont("Helvetica");
IF_IMPL_QUERY   (  const QFont &IDisplayCfgClient::queryDisplayFont(),
                   getDisplayFont(),
                   default_display_font                                                )


void IDisplayCfgClient::noticeConnectedI    (cmplInterface *, bool /*pointer_valid*/)
{
    noticeDisplayColorsChanged(queryDisplayActiveColor(), queryDisplayInactiveColor(), queryDisplayBkgndColor());
    noticeDisplayFontChanged(queryDisplayFont());
}


void IDisplayCfgClient::noticeDisconnectedI   (cmplInterface *, bool /*pointer_valid*/)
{
    noticeDisplayColorsChanged(queryDisplayActiveColor(), queryDisplayInactiveColor(), queryDisplayBkgndColor());
    noticeDisplayFontChanged(queryDisplayFont());
}

