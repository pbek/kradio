/***************************************************************************
                          pluginbase_config_page.h  -  description
                             -------------------
    begin                : Mon Feb 02 2020
    copyright            : (C) 2020 by Martin Witte
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

/////////////////////////////////////////////////////////////////////////////

#ifndef KRADIO_PLUGIN_BASE_CONFIG_PAGE_BASE_H__
#define KRADIO_PLUGIN_BASE_CONFIG_PAGE_BASE_H__

#include <QWidget>
#include "kradio-def.h"

class KRADIO5_EXPORT PluginConfigPageBase : public QWidget
{
Q_OBJECT
public:
    
    PluginConfigPageBase(QWidget         * parent = nullptr, 
                         Qt::WindowFlags   f      = Qt::WindowFlags());
    
    virtual 
    ~PluginConfigPageBase();
    
public Q_SLOTS:
    
    virtual void slotOK()       = 0;    
    virtual void slotCancel()   = 0;
    
    
Q_SIGNALS:
    
    void sigDirty();
    
};  // PluginConfigPageBase




#endif // KRADIO_PLUGIN_BASE_CONFIG_PAGE_BASE_H__
