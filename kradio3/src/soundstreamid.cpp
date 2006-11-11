/***************************************************************************
                     soundstreamid.cpp  -  description
                             -------------------
    begin                : Sun Aug 1 2004
    copyright            : (C) 2004 by Martin Witte
    email                : witte@kawo1.rwth-aachen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "include/soundstreamid.h"

unsigned SoundStreamID::nextID = 1;
unsigned SoundStreamID::nextPhysicalID = 1;
const SoundStreamID SoundStreamID::InvalidID;

SoundStreamID::SoundStreamID()
 :  m_ID(0),
    m_PhysicalID(0)
{
}


SoundStreamID::SoundStreamID(int _id, int _phys_id)
 :  m_ID(_id),
    m_PhysicalID(_phys_id)
{
}


SoundStreamID::SoundStreamID(const SoundStreamID &org)
 :  m_ID (org.m_ID),
    m_PhysicalID(org.m_PhysicalID)
{
}


SoundStreamID &SoundStreamID::operator = (const SoundStreamID &id)
{
    m_ID = id.m_ID;
    m_PhysicalID = id.m_PhysicalID;
    return *this;
}


SoundStreamID SoundStreamID::createNewID()
{
    return SoundStreamID (nextID++, nextPhysicalID++);
}


SoundStreamID SoundStreamID::createNewID(const SoundStreamID &oldID)
{
    return SoundStreamID (nextID++, oldID.m_PhysicalID);
}


void SoundStreamID::invalidate()
{
    m_ID = 0;
    m_PhysicalID = 0;
}

