/***************************************************************************
                          quickbar.cpp  -  description
                             -------------------
    begin                : Mon Feb 11 2002
    copyright            : (C) 2002 by Martin Witte / Frank Schwanz
    email                : witte@kawo1.rwth-aachen.de / schwanz@fh-brandenburg.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "quickbar.h"

QuickBar::QuickBar(QWidget *parent, RadioBase *radio, const char *name )
    : QWidget(parent,name)
{

}

QuickBar::~QuickBar()
{
}

void QuickBar::readConfig (KConfig *c)
{
}

void QuickBar::saveConfig (KConfig *c) const
{
}

void QuickBar::slotConfigChanged()
{
}
