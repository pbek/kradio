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
#include "pluginbase_config_page.h"

class ShortcutsConfiguration : public PluginConfigPageBase
{
Q_OBJECT
protected:
    
    KShortcutsEditor  *m_shortCutsEditor;
    
public:
    ShortcutsConfiguration();
    virtual ~ShortcutsConfiguration();
    
    
    inline 
    KShortcutsEditor *
    editor() const { 
        return m_shortCutsEditor;         
    } // editor()
    

protected slots:

    virtual void slotOK    () override;
    virtual void slotCancel() override;
    
}; // ShortcutsConfiguration 



#endif
