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

INTERFACE(IV4LCfg, IV4LCfgClient)
{
public:
	IF_CON_DESTRUCTOR(IV4LCfg, -1)

RECEIVERS:
	IF_RECEIVER(   setRadioDevice (const QString &s)                            )
	IF_RECEIVER(   setMixerDevice (const QString &s, int ch)                    )
	IF_RECEIVER(   setDevices     (const QString &r, const QString &m, int ch)  )
	
SENDERS:
	IF_SENDER  (   notifyRadioDeviceChanged(const QString &s)                   )
	IF_SENDER  (   notifyMixerDeviceChanged(const QString &s, int Channel)      )

ANSWERS:
    IF_ANSWER  (   const QString &getRadioDevice () const                       )
    IF_ANSWER  (   const QString &getMixerDevice () const                       )
    IF_ANSWER  (   int            getMixerChannel() const                       )

};



INTERFACE(IV4LCfgClient, IV4LCfg)
{
public:
	IF_CON_DESTRUCTOR(IV4LCfgClient, 1)

SENDERS:
	IF_SENDER  (   sendRadioDevice (const QString &s)                            )
	IF_SENDER  (   sendMixerDevice (const QString &s, int ch)                    )
	IF_SENDER  (   sendDevices     (const QString &r, const QString &m, int ch)  )

RECEIVERS:
	IF_RECEIVER(   noticeRadioDeviceChanged(const QString &s)                    )
	IF_RECEIVER(   noticeMixerDeviceChanged(const QString &s, int Channel)       )

QUERIES:
    IF_QUERY   (   const QString &queryRadioDevice ()                            )
    IF_QUERY   (   const QString &queryMixerDevice ()                            )
    IF_QUERY   (   int            queryMixerChannel()                            )

RECEIVERS:
	virtual void noticeConnected    (cmplInterface *);
	virtual void noticeDisconnected (cmplInterface *);
};

#endif
