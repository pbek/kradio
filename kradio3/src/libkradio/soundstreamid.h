/***************************************************************************
                      soundstreamid.h  -  description
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

#ifndef KRADIO_SOUNDSTREAMID_H
#define KRADIO_SOUNDSTREAMID_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

class SoundStreamID {

    SoundStreamID(int _id, int _phys_id);
public:
    SoundStreamID();
    SoundStreamID(const SoundStreamID &org);

    SoundStreamID &operator = (const SoundStreamID &id);

    static SoundStreamID createNewID();
    static SoundStreamID createNewID(const SoundStreamID &oldID);

    bool operator == (const SoundStreamID id) const { return m_ID == id.m_ID; }
    bool operator != (const SoundStreamID id) const { return m_ID != id.m_ID; }
    bool operator >  (const SoundStreamID id) const { return m_ID >  id.m_ID; }
    bool operator <  (const SoundStreamID id) const { return m_ID <  id.m_ID; }
    bool operator >= (const SoundStreamID id) const { return m_ID >= id.m_ID; }
    bool operator <= (const SoundStreamID id) const { return m_ID <= id.m_ID; }

    bool HasSamePhysicalID(const SoundStreamID &x) const { return m_PhysicalID == x.m_PhysicalID; }

    bool isValid() const { return m_ID != 0; } // m_PhysicalID is not checked!
    void invalidate();

    static const SoundStreamID InvalidID;

    int getID() const { return m_ID; }
    int getPhysicalID() const { return m_PhysicalID; }

protected:
    unsigned m_ID;
    unsigned m_PhysicalID;
    static unsigned nextID;
    static unsigned nextPhysicalID;
};


#endif
