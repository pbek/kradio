/***************************************************************************
                          seekhelper.h  -  description
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

#ifndef KRADIO_SEEKHELPER_H
#define KRADIO_SEEKHELPER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "radiodevice_interfaces.h"


class SeekHelper : public IRadioDeviceClient,
                   public IRadioSoundClient
{
public:
    typedef enum { off, searchWorse, searchBest } state_t;
    typedef enum { up, down } direction_t;

    SeekHelper(ISeekRadio &parent);
    virtual ~SeekHelper();

	virtual bool     connectI   (Interface *i);
	virtual bool     disconnectI(Interface *i);

	virtual void start(direction_t dir);
	virtual void step();
	virtual void stop();

	bool isRunning()     const { return m_state != off; }
	bool isRunningUp()   const { return m_state != off && m_direction == up; }
	bool isRunningDown() const { return m_state != off && m_direction == down; }


// IRadioDeviceClient
RECEIVERS:	
	bool noticePowerChanged   (bool /*on*/, const IRadioDevice */*sender*/)          { return false; }
	bool noticeStationChanged (const RadioStation &, const IRadioDevice */*sender*/) { return false; }
	bool noticeDescriptionChanged (const QString &, const IRadioDevice */*sender*/)  { return false; }

// IRadioSoundClient
RECEIVERS:
	bool noticeVolumeChanged(float /*v*/)        { return false; }
	bool noticeTrebleChanged(float /*v*/)        { return false; }
	bool noticeBassChanged(float /*v*/)          { return false; }
	bool noticeBalanceChanged(float /*v*/)       { return false; }
	bool noticeSignalQualityChanged(float /*q*/) { return false; }
	bool noticeSignalQualityChanged(bool/*good*/){ return false; }
	bool noticeSignalMinQualityChanged(float /*q*/){ return false; }
	bool noticeStereoChanged(bool /*s*/)         { return false; }
	bool noticeMuted(bool /*m*/)                 { return false; }
	
protected:

	virtual void finish();

	virtual void abort() = 0;
	virtual bool isGood() const = 0;
	virtual bool isBetter() const = 0;
	virtual bool isWorse() const = 0;
	virtual bool bestFound() const = 0;
	virtual void getData() = 0;
	virtual void rememberBest() = 0;
	virtual bool nextSeekStep() = 0;
	virtual void applyBest() = 0;

protected:
    state_t     m_state;
    direction_t m_direction;
    bool        m_oldMute;


    ISeekRadio  &m_parent;	
};

#endif

