/***************************************************************************
                          radioview_element.cpp  -  description
                             -------------------
    begin                : Fre Jun 20 2003
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

#include "radioview_element.h"
 
RadioViewElement::RadioViewElement (QWidget * /*parent*/, const QString & /*name*/,
                                    RadioViewClass cls)
 : myClass(cls)
{
}


RadioViewElement::~RadioViewElement()
{
}




