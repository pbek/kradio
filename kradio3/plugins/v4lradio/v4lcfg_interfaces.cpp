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


V4LCaps::V4LCaps(const V4LCaps &c)
  : version(c.version),
    description(c.description),
    hasMute(c.hasMute),
    hasVolume(c.hasVolume),
    minVolume(c.minVolume),
    maxVolume(c.maxVolume),
    hasTreble(c.hasTreble),
    minTreble(c.minTreble),
    maxTreble(c.maxTreble),
    hasBass(c.hasBass),
    minBass(c.minBass),
    maxBass(c.maxBass),
    hasBalance(c.hasBalance),
    minBalance(c.minBalance),
    maxBalance(c.maxBalance)
{
}


// IV4LCfg

IF_IMPL_SENDER  (   IV4LCfg::notifyRadioDeviceChanged(const QString &s),
                    noticeRadioDeviceChanged(s)
                )
IF_IMPL_SENDER  (   IV4LCfg::notifyPlaybackMixerChanged(const QString &s, const QString &Channel),
                    noticePlaybackMixerChanged(s, Channel)
                )
IF_IMPL_SENDER  (   IV4LCfg::notifyCaptureMixerChanged(const QString &s, const QString &Channel),
                    noticeCaptureMixerChanged(s, Channel)
                )
IF_IMPL_SENDER  (   IV4LCfg::notifyDeviceVolumeChanged(float v),
                    noticeDeviceVolumeChanged(v)
                )
IF_IMPL_SENDER  (   IV4LCfg::notifyCapabilitiesChanged(const V4LCaps &c),
                    noticeCapabilitiesChanged(c)
                )

IF_IMPL_SENDER  (   IV4LCfg::notifyActivePlaybackChanged(bool a),
                    noticeActivePlaybackChanged(a)
                )

IF_IMPL_SENDER  (   IV4LCfg::notifyMuteOnPowerOffChanged(bool a),
                    noticeMuteOnPowerOffChanged(a)
                )

IF_IMPL_SENDER  (   IV4LCfg::notifyVolumeZeroOnPowerOffChanged(bool a),
                    noticeVolumeZeroOnPowerOffChanged(a)
                )
// IV4LCfgClient

IF_IMPL_SENDER  (   IV4LCfgClient::sendRadioDevice (const QString &s),
                    setRadioDevice(s)
                )
IF_IMPL_SENDER  (   IV4LCfgClient::sendPlaybackMixer(const QString &s, const QString &ch),
                    setPlaybackMixer(s, ch)
                )
IF_IMPL_SENDER  (   IV4LCfgClient::sendCaptureMixer(const QString &s, const QString &ch),
                    setCaptureMixer(s, ch)
                )
IF_IMPL_SENDER  (   IV4LCfgClient::sendDeviceVolume(float v),
                    setDeviceVolume(v)
                )

IF_IMPL_SENDER  (   IV4LCfgClient::sendActivePlayback(bool a),
                    setActivePlayback(a)
                )

IF_IMPL_SENDER  (   IV4LCfgClient::sendMuteOnPowerOff(bool a),
                    setMuteOnPowerOff(a)
                )

IF_IMPL_SENDER  (   IV4LCfgClient::sendVolumeZeroOnPowerOff(bool a),
                    setVolumeZeroOnPowerOff(a)
                )

static QString defaultRDev("/dev/radio");
// static QString defaultMDev("/dev/mixer");

IF_IMPL_QUERY   (   const QString &IV4LCfgClient::queryRadioDevice (),
                    getRadioDevice(),
                    defaultRDev
                )
IF_IMPL_QUERY   (   const QString &IV4LCfgClient::queryPlaybackMixerID (),
                    getPlaybackMixerID(),
                    QString::null
                )
IF_IMPL_QUERY   (   const QString &IV4LCfgClient::queryCaptureMixerID (),
                    getCaptureMixerID(),
                    QString::null
                )

static const QString channel_line("Line");
IF_IMPL_QUERY   (   const QString &IV4LCfgClient::queryPlaybackMixerChannel(),
                    getPlaybackMixerChannel(),
                    channel_line
                )
IF_IMPL_QUERY   (   const QString &IV4LCfgClient::queryCaptureMixerChannel(),
                    getCaptureMixerChannel(),
                    channel_line
                )
IF_IMPL_QUERY   (   float IV4LCfgClient::queryDeviceVolume (),
                    getDeviceVolume(),
                    0.0
                )
IF_IMPL_QUERY   (   V4LCaps IV4LCfgClient::queryCapabilities(QString dev),
                    getCapabilities(dev),
                    V4LCaps()
                )

IF_IMPL_QUERY   (   bool IV4LCfgClient::queryActivePlayback(),
                    getActivePlayback(),
                    false
                )

IF_IMPL_QUERY   (   bool IV4LCfgClient::queryMuteOnPowerOff(),
                    getMuteOnPowerOff(),
                    false
                )

IF_IMPL_QUERY   (   bool IV4LCfgClient::queryVolumeZeroOnPowerOff(),
                    getVolumeZeroOnPowerOff(),
                    false
                )

void IV4LCfgClient::noticeConnectedI    (cmplInterface *, bool /*pointer_valid*/)
{
    noticeRadioDeviceChanged(queryRadioDevice());
    noticePlaybackMixerChanged(queryPlaybackMixerID(), queryPlaybackMixerChannel());
    noticeCaptureMixerChanged (queryCaptureMixerID(),  queryCaptureMixerChannel());
    noticeDeviceVolumeChanged(queryDeviceVolume());
    noticeCapabilitiesChanged(queryCapabilities());
    noticeActivePlaybackChanged(queryActivePlayback());
    noticeMuteOnPowerOffChanged(queryMuteOnPowerOff());
    noticeVolumeZeroOnPowerOffChanged(queryVolumeZeroOnPowerOff());
}


void IV4LCfgClient::noticeDisconnectedI (cmplInterface *, bool /*pointer_valid*/)
{
    noticeRadioDeviceChanged(queryRadioDevice());
    noticePlaybackMixerChanged(queryPlaybackMixerID(), queryPlaybackMixerChannel());
    noticeCaptureMixerChanged (queryCaptureMixerID(),  queryCaptureMixerChannel());
    noticeDeviceVolumeChanged(queryDeviceVolume());
    noticeCapabilitiesChanged(queryCapabilities());
    noticeActivePlaybackChanged(queryActivePlayback());
    noticeMuteOnPowerOffChanged(queryMuteOnPowerOff());
    noticeVolumeZeroOnPowerOffChanged(queryVolumeZeroOnPowerOff());
}


