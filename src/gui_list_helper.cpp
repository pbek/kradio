/***************************************************************************
                              gui_list_helper.cpp
                             -------------------
    begin                : Sun Mar 07 2010
    copyright            : (C) 2010 by Martin Witte
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gui_list_helper.h"

GUIListHelperQObjectBase::GUIListHelperQObjectBase()
{
}

GUIListHelperQObjectBase::~GUIListHelperQObjectBase()
{
}

void GUIListHelperQObjectBase::emitSigDirtyChanged(bool d)
{
    emit sigDirtyChanged(d);
}

