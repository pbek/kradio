/***************************************************************************
                          radiostation.cpp  -  description
                             -------------------
    begin                : Sat Feb 2 2002
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

#include "radiostation.h"


/////////////////////////////////////////////////////////////////////////////

const UndefinedRadioStation undefinedRadioStation;

/////////////////////////////////////////////////////////////////////////////

RadioStation::RadioStation()
    : m_name(""),
      m_shortName(""),
      m_initialVolume(-1),
      m_iconName("")
{
}

RadioStation::RadioStation(const QString &name, const QString &shortName)
    : m_name(name),
      m_shortName(shortName),
      m_initialVolume(-1),
      m_iconName("")
{
}


RadioStation::RadioStation(const RadioStation &s)
    : m_name(s.m_name),
      m_shortName(s.m_shortName),
      m_initialVolume(s.m_initialVolume),
      m_iconName(s.m_iconName)
{
}


RadioStation::~RadioStation()
{
}





/////////////////////////////////////////////////////////////////////////

int UndefinedRadioStation::compare(const RadioStation &_s) const
{
	UndefinedRadioStation const *s = dynamic_cast<UndefinedRadioStation const*>(&_s);

	if (!s)
		return -1;

	return 0;

}
