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

#include <qcolor.h>
#include "displaycfg_interfaces.h"
 
// IDisplayCfg

IF_IMPL_SENDER  (  IDisplayCfg::notifyColorsChanged(const QColor &a, const QColor &b),
                   noticeColorsChanged(a,b)                                          )

// IDisplayCfgClient

IF_IMPL_SENDER  (  IDisplayCfgClient::sendColors(const QColor &a, const QColor &b),
                   setColors(a,b)                                                    )

IF_IMPL_QUERY   (  QColor IDisplayCfgClient::queryActiveColor(),
                   getActiveColor(),
                   QColor(20, 244, 20)                                               )

IF_IMPL_QUERY   (  QColor IDisplayCfgClient::queryBkgndColor(),
                   getBkgndColor(),
                   QColor(10, 117, 10)                                               )


void IDisplayCfgClient::noticeConnected    (cmplInterface *, bool /*pointer_valid*/)
{
	noticeColorsChanged(queryActiveColor(), queryBkgndColor());
}

void IDisplayCfgClient::noticeDisconnected   (cmplInterface *, bool /*pointer_valid*/)
{
	noticeColorsChanged(queryActiveColor(), queryBkgndColor());
}

