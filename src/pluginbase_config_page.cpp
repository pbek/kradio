/***************************************************************************
                          pluginbase_config_page.cpp  -  description
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

#include "pluginbase_config_page.h"

PluginConfigPageBase ::
PluginConfigPageBase(QWidget         * parent, 
                     Qt::WindowFlags   f)
  : QWidget(parent, f)
{} // CTOR
    
    
PluginConfigPageBase::
~PluginConfigPageBase()
{} // DTOR


#include "pluginbase_config_page.moc"
