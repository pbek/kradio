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

struct V4LCaps
{
	int     version;
	QString description;

	bool    hasVolume;
	bool    hasTreble;
	bool    hasBass;
	bool    hasBalance;

	V4LCaps();
	V4LCaps(int v, const QString &d,
		    bool hasVolume,
		    bool hasTreble,
		    bool hasBass,
		    bool hasBalance
		   );
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
