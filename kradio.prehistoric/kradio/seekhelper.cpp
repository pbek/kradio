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

#include "seekhelper.h"

#include <kdebug.h>
 
SeekHelper::SeekHelper(ISeekRadio &parent)
  : m_state(off),
    m_parent(parent)
{
}


SeekHelper::~SeekHelper()
{
}


bool SeekHelper::connect   (Interface *i)
{
	bool a = IRadioDeviceClient::connect(i);
	bool b = IRadioSoundClient::connect(i);

    if (a) kdDebug() << "SeekHelper: IRadioDeviceClient connected\n";
    if (b) kdDebug() << "SeekHelper: IRadioSoundClient connected\n";

	return a || b;
}


bool SeekHelper::disconnect(Interface *i)
{
	bool a = IRadioDeviceClient::disconnect(i);
	bool b = IRadioSoundClient::disconnect(i);
	return a || b;
}


void SeekHelper::start(direction_t dir)
{
	if (m_state == off) {
		getData();
		m_state            = isGood() ? searchWorse : searchBest;
		m_direction        = dir;

		m_oldMute          = queryIsMuted();
		sendMute(true);
	
		m_parent.notifySeekStarted(m_direction == up);

		step();
	}
}


void SeekHelper::stop ()
{
	if (m_state != off) {
		m_state = off;
		abort();
		sendMute(m_oldMute);
		m_parent.notifySeekStopped();
	}
}


void SeekHelper::finish ()
{
	if (m_state != off) {
		applyBest();
		const RadioStation &rs = queryCurrentStation();
	
		stop();
		m_parent.notifySeekFinished(rs);
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
			} else if (isBetter() && isGood()) {
				rememberBest();
			}

			if (! nextSeekStep()) {
				if (isGood() && bestFound()) {
					finish();
				} else {
					stop();
				}
			}

			break;
	}
}



