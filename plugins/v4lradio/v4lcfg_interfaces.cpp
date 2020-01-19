/***************************************************************************
                          v4lradio_interfaces.cpp  -  description
                             -------------------
    begin                : Sam Jun 21 2003
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

#include <linux/soundcard.h>
#include "v4lcfg_interfaces.h"

const char* V4LVersionStrings[V4L_Version_COUNT] = { "V4L Unknown", "V4L1", "V4L2" };

///////////////////////////////////////////////////////////////////////

V4LCaps::V4LCaps()
  : description(QString()),
    deviceDescription(QString()),
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
    maxBalance(65535),
    hasRDS(false)
{
    for (int i = 0; i < V4L_Version_COUNT; ++i) {
        v4l_version_support[i] = false;
    }
}


V4LCaps::V4LCaps(const V4LCaps &c)
  : description(c.description),
    deviceDescription(c.deviceDescription),
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
    maxBalance(c.maxBalance),
    hasRDS(c.hasRDS)
{
    for (int i = 0; i < V4L_Version_COUNT; ++i) {
        v4l_version_support[i] = c.v4l_version_support[i];
    }
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

IF_IMPL_SENDER  (   IV4LCfg::notifyActivePlaybackChanged(bool a, bool muteCaptureChannelPlayback),
                    noticeActivePlaybackChanged(a, muteCaptureChannelPlayback)
                )

IF_IMPL_SENDER  (   IV4LCfg::notifyMuteOnPowerOffChanged(bool a),
                    noticeMuteOnPowerOffChanged(a)
                )

IF_IMPL_SENDER  (   IV4LCfg::notifyVolumeZeroOnPowerOffChanged(bool a),
                    noticeVolumeZeroOnPowerOffChanged(a)
                )
IF_IMPL_SENDER  (   IV4LCfg::notifyV4LVersionOverrideChanged(V4LVersion vo),
                    noticeV4LVersionOverrideChanged(vo)
                )

IF_IMPL_SENDER  (   IV4LCfg::notifyForceRDSEnabledChanged(bool a),
                    noticeForceRDSEnabledChanged(a)
                )

IF_IMPL_SENDER  (   IV4LCfg::notifyDeviceProbeAtStartupChanged(bool e),
                    noticeDeviceProbeAtStartupChanged(e)
                )

// IV4LCfgClient

IF_IMPL_SENDER  (   IV4LCfgClient::sendRadioDevice (const QString &s),
                    setRadioDevice(s)
                )
IF_IMPL_SENDER  (   IV4LCfgClient::sendPlaybackMixer(const QString &s, const QString &ch, bool force),
                    setPlaybackMixer(s, ch, force)
                )
IF_IMPL_SENDER  (   IV4LCfgClient::sendCaptureMixer(const QString &s, const QString &ch, bool force),
                    setCaptureMixer(s, ch, force)
                )
IF_IMPL_SENDER  (   IV4LCfgClient::sendDeviceVolume(float v),
                    setDeviceVolume(v)
                )

IF_IMPL_SENDER  (   IV4LCfgClient::sendActivePlayback(bool a, bool muteCaptureChannelPlayback),
                    setActivePlayback(a, muteCaptureChannelPlayback)
                )

IF_IMPL_SENDER  (   IV4LCfgClient::sendMuteOnPowerOff(bool a),
                    setMuteOnPowerOff(a)
                )

IF_IMPL_SENDER  (   IV4LCfgClient::sendVolumeZeroOnPowerOff(bool a),
                    setVolumeZeroOnPowerOff(a)
                )
IF_IMPL_SENDER  (   IV4LCfgClient::sendV4LVersionOverride(V4LVersion vo),
                    setV4LVersionOverride(vo)
                )

IF_IMPL_SENDER  (   IV4LCfgClient::sendForceRDSEnabled(bool a),
                    setForceRDSEnabled(a)
                )

IF_IMPL_SENDER  (   IV4LCfgClient::sendDeviceProbeAtStartup(bool e),
                    setDeviceProbeAtStartup(e)
                )



static QString defaultRDev("/dev/radio");
// static QString defaultMDev("/dev/mixer");

IF_IMPL_QUERY   (   const QString &IV4LCfgClient::queryRadioDevice (),
                    getRadioDevice(),
                    defaultRDev
                )
static QString nullMixerID;
IF_IMPL_QUERY   (   const QString &IV4LCfgClient::queryPlaybackMixerID (),
                    getPlaybackMixerID(),
                    nullMixerID
                )
IF_IMPL_QUERY   (   const QString &IV4LCfgClient::queryCaptureMixerID (),
                    getCaptureMixerID(),
                    nullMixerID
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
IF_IMPL_QUERY   (   V4LCaps IV4LCfgClient::queryCapabilities(const QString &dev),
                    getCapabilities(dev),
                    V4LCaps()
                )

IF_IMPL_QUERY   (   bool IV4LCfgClient::queryActivePlayback(bool &muteCaptureChannelPlayback),
                    getActivePlayback(muteCaptureChannelPlayback),
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

IF_IMPL_QUERY   (   V4LVersion IV4LCfgClient::queryV4LVersionOverride(),
                    getV4LVersionOverride(),
                    V4L_VersionUnkown
                )

IF_IMPL_QUERY   (   bool IV4LCfgClient::queryForceRDSEnabled(),
                    getForceRDSEnabled(),
                    false
                )

IF_IMPL_QUERY   (   bool IV4LCfgClient::queryDeviceProbeAtStartup(),
                    getDeviceProbeAtStartup(),
                    true
                )

IF_IMPL_QUERY   (   QList<DeviceInfo> IV4LCfgClient::queryDeviceProposals(const QString &devdir),
                    getDeviceProposals(devdir),
                    QList<DeviceInfo>()
                )

void IV4LCfgClient::noticeConnectedI    (cmplInterface *, bool /*pointer_valid*/)
{
    noticeRadioDeviceChanged         (queryRadioDevice());
    noticePlaybackMixerChanged       (queryPlaybackMixerID(), queryPlaybackMixerChannel());
    noticeCaptureMixerChanged        (queryCaptureMixerID(),  queryCaptureMixerChannel());
    noticeDeviceVolumeChanged        (queryDeviceVolume());
    noticeCapabilitiesChanged        (queryCapabilities());
    bool muteCaptureChannelPlayback = false;
    bool activepb                   = queryActivePlayback(muteCaptureChannelPlayback);
    noticeActivePlaybackChanged      (activepb, muteCaptureChannelPlayback);
    noticeMuteOnPowerOffChanged      (queryMuteOnPowerOff());
    noticeVolumeZeroOnPowerOffChanged(queryVolumeZeroOnPowerOff());
    noticeForceRDSEnabledChanged     (queryForceRDSEnabled());
    noticeDeviceProbeAtStartupChanged(queryDeviceProbeAtStartup());
}


void IV4LCfgClient::noticeDisconnectedI (cmplInterface *, bool /*pointer_valid*/)
{
    noticeRadioDeviceChanged         (queryRadioDevice());
    noticePlaybackMixerChanged       (queryPlaybackMixerID(), queryPlaybackMixerChannel());
    noticeCaptureMixerChanged        (queryCaptureMixerID(),  queryCaptureMixerChannel());
    noticeDeviceVolumeChanged        (queryDeviceVolume());
    noticeCapabilitiesChanged        (queryCapabilities());
    bool muteCaptureChannelPlayback = false;
    bool activepb                   = queryActivePlayback(muteCaptureChannelPlayback);
    noticeActivePlaybackChanged      (activepb, muteCaptureChannelPlayback);
    noticeMuteOnPowerOffChanged      (queryMuteOnPowerOff());
    noticeVolumeZeroOnPowerOffChanged(queryVolumeZeroOnPowerOff());
    noticeForceRDSEnabledChanged     (queryForceRDSEnabled());
    noticeDeviceProbeAtStartupChanged(queryDeviceProbeAtStartup());
}


