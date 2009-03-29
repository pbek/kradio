/***************************************************************************
                          shortcuts-configuration.cpp  -  description
                             -------------------
    begin                : Mon Feb 4 2002
    copyright            : (C) 2002 by Martin Witte / Frank Schwanz
    email                : emw-kradio@nocabal.de / schwanz@fh-brandenburg.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "shortcuts-configuration.h"

ShortcutsConfiguration::ShortcutsConfiguration()
    : KShortcutsEditor(NULL)
{
}


ShortcutsConfiguration::~ShortcutsConfiguration()
{
}


void ShortcutsConfiguration::slotOK()
{
    save();
    commit();
}

void ShortcutsConfiguration::slotCancel()
{
    undoChanges();
}

#include "shortcuts-configuration.moc"
