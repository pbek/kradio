/***************************************************************************
                          seekhelper.cpp  -  description
                             -------------------
    begin                : Sam Mai 10 2003
    copyright            : (C) 2003 by Martin Witte
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

#include "include/seekhelper.h"

#include <kdebug.h>

SeekHelper::SeekHelper(ISeekRadio &parent)
  : m_state(off),
    m_parent(parent),
    m_SoundStreamID(SoundStreamID::InvalidID)
{
}


SeekHelper::~SeekHelper()
{
}


bool SeekHelper::connectI   (Interface *i)
{
    bool a = IRadioDeviceClient::connectI(i);
    bool b = ISoundStreamClient::connectI(i);
    return a || b;
}


bool SeekHelper::disconnectI(Interface *i)
{
    bool a = IRadioDeviceClient::disconnectI(i);
    bool b = ISoundStreamClient::disconnectI(i);
    return a || b;
}


void SeekHelper::start(const SoundStreamID &id, direction_t dir)
{
    m_SoundStreamID = id;
    if (m_state == off) {
        getData();
        m_state            = isGood() ? searchWorse : searchBest;
        m_direction        = dir;

        queryIsMuted(m_SoundStreamID, m_oldMute);
        sendMute(m_SoundStreamID, true);

        m_parent.notifySeekStarted(m_direction == up);

        step();
    }
}


void SeekHelper::stop ()
{
    if (m_state != off) {
        m_state = off;
        abort();
        sendMute(m_SoundStreamID, m_oldMute);
        m_parent.notifySeekStopped();
        m_SoundStreamID = SoundStreamID::InvalidID;
    }
}


void SeekHelper::finish ()
{
    if (m_state != off) {
        applyBest();
        const RadioStation &rs = queryCurrentStation();

        stop();
        m_parent.notifySeekFinished(rs, isGood());
    }
}


void SeekHelper::step ()
{
    if (m_state == off)
        return;

    getData();

    switch (m_state) {

        case off : break;

        case searchWorse :
            if (isWorse())
                m_state = searchBest;

            if (! nextSeekStep()) {
                stop();
            }

            break;

        case searchBest :
            if (isWorse() && bestFound()) {
                finish();
            } else {
                if (isBetter() && isGood()) {
                    rememberBest();
                }
                if (! nextSeekStep()) {
                    if (isGood() && bestFound()) {
                        finish();
                    } else {
                        stop();
                    }
                }
            }
            break;
    }
}



