/***************************************************************************
                          v4lradio_interfaces.h  -  description
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

#ifndef KRADIO_V4LCFG_INTERFACES_H
#define KRADIO_V4LCFG_INTERFACES_H

#include "interfaces.h"
#include "math.h"

struct V4LCaps
{
	int     version;
	QString description;

	bool    hasMute;
	
	bool    hasVolume;
	int     minVolume,  maxVolume;
	bool    hasTreble;
	int     minTreble,  maxTreble;
	bool    hasBass;
	int     minBass,    maxBass;
	bool    hasBalance;
	int     minBalance, maxBalance;

	V4LCaps();

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
};



INTERFACE(IV4LCfg, IV4LCfgClient)
{
public:
	IF_CON_DESTRUCTOR(IV4LCfg, -1)

RECEIVERS:
	IF_RECEIVER(   setRadioDevice (const QString &s)                            )
	IF_RECEIVER(   setMixerDevice (const QString &s, int ch)                    )
	IF_RECEIVER(   setDevices     (const QString &r, const QString &m, int ch)  )
	IF_RECEIVER(   setDeviceVolume(float v)                                     )
	
SENDERS:
	IF_SENDER  (   notifyRadioDeviceChanged(const QString &s)                   )
	IF_SENDER  (   notifyMixerDeviceChanged(const QString &s, int Channel)      )
	IF_SENDER  (   notifyDeviceVolumeChanged(float v)                           )
	IF_SENDER  (   notifyCapabilitiesChanged(const V4LCaps &)                   )

ANSWERS:
    IF_ANSWER  (   const QString &getRadioDevice () const                       )
    IF_ANSWER  (   const QString &getMixerDevice () const                       )
    IF_ANSWER  (   int            getMixerChannel() const                       )
    IF_ANSWER  (   float          getDeviceVolume() const                       )
    IF_ANSWER  (   const V4LCaps &getCapabilities() const                       )
};



INTERFACE(IV4LCfgClient, IV4LCfg)
{
public:
	IF_CON_DESTRUCTOR(IV4LCfgClient, 1)

SENDERS:
	IF_SENDER  (   sendRadioDevice (const QString &s)                            )
	IF_SENDER  (   sendMixerDevice (const QString &s, int ch)                    )
	IF_SENDER  (   sendDevices     (const QString &r, const QString &m, int ch)  )
	IF_SENDER  (   sendDeviceVolume(float v)                                     )

RECEIVERS:
	IF_RECEIVER(   noticeRadioDeviceChanged(const QString &s)                    )
	IF_RECEIVER(   noticeMixerDeviceChanged(const QString &s, int Channel)       )
	IF_RECEIVER(   noticeDeviceVolumeChanged(float v)                            )
	IF_RECEIVER(   noticeCapabilitiesChanged(const V4LCaps &)                   )

QUERIES:
    IF_QUERY   (   const QString &queryRadioDevice ()                            )
    IF_QUERY   (   const QString &queryMixerDevice ()                            )
    IF_QUERY   (   int            queryMixerChannel()                            )
    IF_QUERY   (   float          queryDeviceVolume()                            )
    IF_QUERY   (   const V4LCaps &queryCapabilities()                            )

RECEIVERS:
	virtual void noticeConnected    (cmplInterface *, bool /*pointer_valid*/);
	virtual void noticeDisconnected (cmplInterface *, bool /*pointer_valid*/);
};

#endif
