/***************************************************************************
                          v4lradio_interfaces.cpp  -  description
                             -------------------
    begin                : Sam Jun 21 2003
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

#include <linux/soundcard.h>
#include "v4lcfg_interfaces.h"
 
///////////////////////////////////////////////////////////////////////

V4LCaps::V4LCaps()
  : version(0),
    description(QString::null),
    hasMute(false),
    hasVolume(false),
    minVolume(0),
    maxVolume(65535),
    hasTreble(false),
    minTreble(0),
    maxTreble(65535),
    hasBass(false),
    minBass(0),
    maxBass(65535),
    hasBalance(false),
    minBalance(0),
    maxBalance(65535)
{
}


// IV4LCfg
 
IF_IMPL_SENDER  (   IV4LCfg::notifyRadioDeviceChanged(const QString &s),
                    noticeRadioDeviceChanged(s)
                )
IF_IMPL_SENDER  (   IV4LCfg::notifyMixerDeviceChanged(const QString &s, int Channel),
                    noticeMixerDeviceChanged(s, Channel)
                )
IF_IMPL_SENDER  (   IV4LCfg::notifyDeviceVolumeChanged(float v),
                    noticeDeviceVolumeChanged(v)
                )
IF_IMPL_SENDER  (   IV4LCfg::notifyCapabilitiesChanged(const V4LCaps &c),
                    noticeCapabilitiesChanged(c)
                )

// IV4LCfgClient

IF_IMPL_SENDER  (   IV4LCfgClient::sendRadioDevice (const QString &s),
                    setRadioDevice(s)
                )
IF_IMPL_SENDER  (   IV4LCfgClient::sendMixerDevice (const QString &s, int ch),
                    setMixerDevice(s, ch)
                )
IF_IMPL_SENDER  (   IV4LCfgClient::sendDevices     (const QString &r, const QString &m, int ch),
					setDevices(r, m, ch)
                )
IF_IMPL_SENDER  (   IV4LCfgClient::sendDeviceVolume(float v),
                    setDeviceVolume(v)
                )

static QString defaultRDev("/dev/radio");
static QString defaultMDev("/dev/mixer");

IF_IMPL_QUERY   (   const QString &IV4LCfgClient::queryRadioDevice (),
					getRadioDevice(),
					defaultMDev
                )
IF_IMPL_QUERY   (   const QString &IV4LCfgClient::queryMixerDevice (),
                    getMixerDevice(),
                    defaultMDev
                )
IF_IMPL_QUERY   (   int            IV4LCfgClient::queryMixerChannel(),
                    getMixerChannel(),
                    SOUND_MIXER_LINE
                )
IF_IMPL_QUERY   (   float IV4LCfgClient::queryDeviceVolume (),
					getDeviceVolume(),
					0.0
                )
V4LCaps emptyCaps;
IF_IMPL_QUERY   (   const V4LCaps &IV4LCfgClient::queryCapabilities(),
					getCapabilities(),
					emptyCaps
                )

void IV4LCfgClient::noticeConnected    (cmplInterface *, bool /*pointer_valid*/)
{
	noticeRadioDeviceChanged(queryRadioDevice());
	noticeMixerDeviceChanged(queryMixerDevice(), queryMixerChannel());
	noticeDeviceVolumeChanged(queryDeviceVolume());
	noticeCapabilitiesChanged(queryCapabilities());
}


void IV4LCfgClient::noticeDisconnected (cmplInterface *, bool /*pointer_valid*/)
{
	noticeRadioDeviceChanged(queryRadioDevice());
	noticeMixerDeviceChanged(queryMixerDevice(), queryMixerChannel());
	noticeDeviceVolumeChanged(queryDeviceVolume());
	noticeCapabilitiesChanged(queryCapabilities());
}


