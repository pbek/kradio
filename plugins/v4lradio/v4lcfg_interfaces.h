/***************************************************************************
                          v4lradio_interfaces.h  -  description
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

#ifndef KRADIO_V4LCFG_INTERFACES_H
#define KRADIO_V4LCFG_INTERFACES_H

#include "interfaces.h"
#include "math.h"

#include <QString>
#include <QStringList>
#include <QFileInfo>

enum V4LVersion {V4L_VersionUnkown = 0,
                 V4L_Version1      = 1,
                 V4L_Version2      = 2,
                 V4L_Version_COUNT = 3
                };

extern const char* V4LVersionStrings[V4L_Version_COUNT];

struct V4LCaps
{
    bool    v4l_version_support[(int)V4L_Version_COUNT];
    QString description;
    QString deviceDescription;

    bool    hasMute;

    bool    hasVolume;
    int     minVolume,  maxVolume;
    bool    hasTreble;
    int     minTreble,  maxTreble;
    bool    hasBass;
    int     minBass,    maxBass;
    bool    hasBalance;
    int     minBalance, maxBalance;

    bool    hasRDS;

    V4LCaps();
    V4LCaps(const V4LCaps &);

    float  volumeStep()  const { return 1.0 / (float)(maxVolume  - minVolume); }
    float  trebleStep()  const { return 1.0 / (float)(maxTreble  - minTreble); }
    float  bassStep()    const { return 1.0 / (float)(maxBass    - minBass); }
    float  balanceStep() const { return 1.0 / (float)(maxBalance - minBalance); }

    void   setVolume (int min, int max) { hasVolume  = true; minVolume  = min; maxVolume  = max; }
    void   setTreble (int min, int max) { hasTreble  = true; minTreble  = min; maxTreble  = max; }
    void   setBass   (int min, int max) { hasBass    = true; minBass    = min; maxBass    = max; }
    void   setBalance(int min, int max) { hasBalance = true; minBalance = min; maxBalance = max; }

    void   unsetVolume () { hasVolume  = false; minVolume  = 0; maxVolume  = 65535; }
    void   unsetTreble () { hasTreble  = false; minTreble  = 0; maxTreble  = 65535; }
    void   unsetBass   () { hasBass    = false; minBass    = 0; maxBass    = 65535; }
    void   unsetBalance() { hasBalance = false; minBalance = 0; maxBalance = 65535; }

    int    intGetVolume (float f) const { return (int)rint(minVolume  + (maxVolume  - minVolume ) * f); }
    int    intGetTreble (float f) const { return (int)rint(minTreble  + (maxTreble  - minTreble ) * f); }
    int    intGetBass   (float f) const { return (int)rint(minBass    + (maxBass    - minBass   ) * f); }
    int    intGetBalance(float f) const { return (int)rint(minBalance + (maxBalance - minBalance) / 2.0 * (1.0 + f)); }

    float  floatGetVolume (int i) const { return (float)(i - minVolume)  * volumeStep(); }
    float  floatGetTreble (int i) const { return (float)(i - minTreble)  * trebleStep(); }
    float  floatGetBass   (int i) const { return (float)(i - minBass  )  * bassStep(); }
    float  floatGetBalance(int i) const { return (float)(i - minBalance) * balanceStep() * 2.0 - 1.0; }

    QString getDebugDescription() {
        QStringList dbgVersions;
        for (int i = 0; i < V4L_Version_COUNT; ++i) { if (v4l_version_support[i]) dbgVersions.append(QString::number(i)); }
        QStringList dbgFeatures;
        if (hasMute)    dbgFeatures.append("mute");
        if (hasVolume)  dbgFeatures.append("volume");
        if (hasBass)    dbgFeatures.append("treble");
        if (hasTreble)  dbgFeatures.append("treble");
        if (hasBalance) dbgFeatures.append("balance");
        if (hasRDS)     dbgFeatures.append("rds");
        return "V4LVersions: " + dbgVersions.join(", ") + "; Features: " + dbgFeatures.join(", ");
    }
};




struct DeviceInfo
{
    QString   path;
    QFileInfo info;
    V4LCaps   caps;
    QString   description;

    DeviceInfo(const QString &_path, const QFileInfo &_info, const V4LCaps &_caps, const QString &descr)
      : path(_path), info(_info), caps(_caps), description(descr) {}
};



INTERFACE(IV4LCfg, IV4LCfgClient)
{
public:
    IF_CON_DESTRUCTOR(IV4LCfg, -1)

RECEIVERS:
    IF_RECEIVER(   setRadioDevice  (const QString &s)                             )
    IF_RECEIVER(   setPlaybackMixer(QString soundStreamClientID, QString ch, bool force = false)  )
    IF_RECEIVER(   setCaptureMixer (QString soundStreamClientID, QString ch, bool force = false)  )
    IF_RECEIVER(   setDeviceVolume(float v)                                       )
    IF_RECEIVER(   setActivePlayback(bool enabled, bool mute_capture)             )
    IF_RECEIVER(   setMuteOnPowerOff(bool m)                                      )
    IF_RECEIVER(   setVolumeZeroOnPowerOff(bool m)                                )
    IF_RECEIVER(   setV4LVersionOverride(V4LVersion vo)                           )
    IF_RECEIVER(   setForceRDSEnabled(bool e)                                     )
    IF_RECEIVER(   setDeviceProbeAtStartup(bool e)                                )


SENDERS:
    IF_SENDER  (   notifyRadioDeviceChanged  (const QString &s)                   )
    IF_SENDER  (   notifyPlaybackMixerChanged(const QString &soundStreamClientID, const QString &Channel)      )
    IF_SENDER  (   notifyCaptureMixerChanged (const QString &soundStreamClientID, const QString &Channel)      )
    IF_SENDER  (   notifyDeviceVolumeChanged (float v)                            )
    IF_SENDER  (   notifyCapabilitiesChanged (const V4LCaps &)                    )
    IF_SENDER  (   notifyActivePlaybackChanged (bool enabled, bool mute_capture)  )
    IF_SENDER  (   notifyMuteOnPowerOffChanged (bool a)                           )
    IF_SENDER  (   notifyVolumeZeroOnPowerOffChanged (bool a)                     )
    IF_SENDER  (   notifyV4LVersionOverrideChanged(V4LVersion vo)                 )
    IF_SENDER  (   notifyForceRDSEnabledChanged(bool e)                           )
    IF_SENDER  (   notifyDeviceProbeAtStartupChanged(bool e)                      )

ANSWERS:
    IF_ANSWER  (   const QString    &getRadioDevice () const                         )
    IF_ANSWER  (   const QString    &getPlaybackMixerID () const                     )
    IF_ANSWER  (   const QString    &getCaptureMixerID () const                      )
    IF_ANSWER  (   const QString    &getPlaybackMixerChannel() const                 )
    IF_ANSWER  (   const QString    &getCaptureMixerChannel() const                  )
    IF_ANSWER  (   float             getDeviceVolume() const                         )
    IF_ANSWER  (   V4LCaps           getCapabilities(const QString &dev = QString()) const )
    IF_ANSWER  (   bool              getActivePlayback(bool &mute_capture) const     )
    IF_ANSWER  (   bool              getMuteOnPowerOff() const                       )
    IF_ANSWER  (   bool              getVolumeZeroOnPowerOff() const                 )
    IF_ANSWER  (   V4LVersion        getV4LVersionOverride() const                   )
    IF_ANSWER  (   bool              getForceRDSEnabled() const                      )
    IF_ANSWER  (   bool              getDeviceProbeAtStartup() const                 )
    IF_ANSWER  (   QList<DeviceInfo> getDeviceProposals(const QString &devdir = QString::fromLatin1("/dev/")) const )
};



INTERFACE(IV4LCfgClient, IV4LCfg)
{
public:
    IF_CON_DESTRUCTOR(IV4LCfgClient, 1)

SENDERS:
    IF_SENDER  (   sendRadioDevice (const QString &s)                            )
    IF_SENDER  (   sendPlaybackMixer(const QString &soundStreamClientID, const QString &ch, bool force = false) )
    IF_SENDER  (   sendCaptureMixer (const QString &soundStreamClientID, const QString &ch, bool force = false) )
    IF_SENDER  (   sendDeviceVolume(float v)                                     )
    IF_SENDER  (   sendActivePlayback(bool enabled, bool mute_capture)           )
    IF_SENDER  (   sendMuteOnPowerOff(bool a)                                    )
    IF_SENDER  (   sendVolumeZeroOnPowerOff(bool a)                              )
    IF_SENDER  (   sendV4LVersionOverride(V4LVersion vo)                         )
    IF_SENDER  (   sendForceRDSEnabled(bool a)                                   )
    IF_SENDER  (   sendDeviceProbeAtStartup(bool e)                              )

RECEIVERS:
    IF_RECEIVER(   noticeRadioDeviceChanged(const QString &s)                    )
    IF_RECEIVER(   noticePlaybackMixerChanged(const QString &soundStreamClientID, const QString &Channel)       )
    IF_RECEIVER(   noticeCaptureMixerChanged (const QString &soundStreamClientID, const QString &Channel)       )
    IF_RECEIVER(   noticeDeviceVolumeChanged(float v)                            )
    IF_RECEIVER(   noticeCapabilitiesChanged(const V4LCaps &)                    )
    IF_RECEIVER(   noticeActivePlaybackChanged(bool enable, bool mute_capture)   )
    IF_RECEIVER(   noticeMuteOnPowerOffChanged(bool a)                           )
    IF_RECEIVER(   noticeVolumeZeroOnPowerOffChanged(bool a)                     )
    IF_RECEIVER(   noticeV4LVersionOverrideChanged(V4LVersion vo)                )
    IF_RECEIVER(   noticeForceRDSEnabledChanged(bool a)                          )
    IF_RECEIVER(   noticeDeviceProbeAtStartupChanged(bool e)                     )

QUERIES:
    IF_QUERY   (   const QString    &queryRadioDevice ()                            )
    IF_QUERY   (   const QString    &queryPlaybackMixerID ()                        )
    IF_QUERY   (   const QString    &queryCaptureMixerID ()                         )
    IF_QUERY   (   const QString    &queryPlaybackMixerChannel()                    )
    IF_QUERY   (   const QString    &queryCaptureMixerChannel()                     )
    IF_QUERY   (   float             queryDeviceVolume()                            )
    IF_QUERY   (   V4LCaps           queryCapabilities(const QString &dev = QString()) )
    IF_QUERY   (   bool              queryActivePlayback(bool &mute_capture)        )
    IF_QUERY   (   bool              queryMuteOnPowerOff()                          )
    IF_QUERY   (   bool              queryVolumeZeroOnPowerOff()                    )
    IF_QUERY   (   V4LVersion        queryV4LVersionOverride()                      )
    IF_QUERY   (   bool              queryForceRDSEnabled()                         )
    IF_QUERY   (   bool              queryDeviceProbeAtStartup()                    )
    IF_QUERY   (   QList<DeviceInfo> queryDeviceProposals(const QString &devdir = "/dev/") )

RECEIVERS:
    virtual void noticeConnectedI    (cmplInterface *, bool /*pointer_valid*/);
    virtual void noticeDisconnectedI (cmplInterface *, bool /*pointer_valid*/);
};

#endif
