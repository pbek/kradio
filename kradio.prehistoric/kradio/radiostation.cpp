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

RadioStation::RadioStation(QObject *_parent, QString _name)
    : QObject (_parent, _name),
      m_frequency(0),
      m_useInQuickSelect(false),
      m_useInDockingMenu(true),
      m_initialVolume(-1)
{
}


RadioStation::RadioStation(const RadioStation &s)
    : QObject (s.parent(), s.name()),
      m_frequency(s.m_frequency),
      m_shortName(s.m_shortName),
      m_useInQuickSelect(s.m_useInQuickSelect),
      m_useInDockingMenu(s.m_useInDockingMenu),
      m_initialVolume(s.m_initialVolume),
      m_iconName(s.m_iconName)
{
}

RadioStation::RadioStation(QObject *_parent, const RadioStation &s)
    : QObject (_parent, s.name()),
      m_frequency(s.m_frequency),
      m_shortName(s.m_shortName),
      m_useInQuickSelect(s.m_useInQuickSelect),
      m_useInDockingMenu(s.m_useInDockingMenu),
      m_initialVolume(s.m_initialVolume),
      m_iconName(s.m_iconName)
{
}

RadioStation::~RadioStation()
{
}

QString RadioStation::longName() const
{
    return name();
}
