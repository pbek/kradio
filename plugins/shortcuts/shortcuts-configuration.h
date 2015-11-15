/***************************************************************************
                     shortcuts-configuration.h  -  description
                             -------------------
    begin                : Mon Feb 8 2009
    copyright            : (C) 2009 by Martin Witte
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

#ifndef KRADIO_SHORTCUTS_CONFIGURATION_H
#define KRADIO_SHORTCUTS_CONFIGURATION_H

#include <kshortcutseditor.h>

class ShortcutsConfiguration : public KShortcutsEditor
{
Q_OBJECT
public:
    ShortcutsConfiguration();
    ~ShortcutsConfiguration();

protected slots:

    void slotOK();
    void slotCancel();
};



#endif
