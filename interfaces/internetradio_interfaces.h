/***************************************************************************
                          internetradio_interfaces.h  -  description
                             -------------------
    begin                : Sun Feb 15 2009
    copyright            : (C) 2009 by Martin Witte
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

/***************************************************************************
 *                                                                         *
 *   Interfaces in this header:                                            *
 *                                                                         *
 *   IInternetRadio(Client)                                                    *
 *                                                                         *
 ***************************************************************************/

#ifndef KRADIO_INTERNETRADIO_INTERFACES_H
#define KRADIO_INTERNETRADIO_INTERFACES_H

#include "interfaces.h"

class InternetRadioStation;
class FrequencyRadioStation;
class QUrl;

INTERFACE(IInternetRadio, IInternetRadioClient)
{
public :
    IF_CON_DESTRUCTOR(IInternetRadio, -1)

RECEIVERS:
    IF_RECEIVER(  setURL(const QUrl &url, const InternetRadioStation *irs)          )

SENDERS:
    IF_SENDER  (  notifyURLChanged(const QUrl &u, const InternetRadioStation *irs)  )

ANSWERS:
    IF_ANSWER  (  const QUrl &          getURL() const                              );
};




INTERFACE(IInternetRadioClient, IInternetRadio)
{
public :
    IF_CON_DESTRUCTOR(IInternetRadioClient, 1)


SENDERS:
    IF_SENDER  (  sendURL(const QUrl &url, const InternetRadioStation *irs)           )

RECEIVERS:
    IF_RECEIVER(  noticeURLChanged(const QUrl &url, const InternetRadioStation *irs)  )

QUERIES:
    IF_QUERY   (  const QUrl &queryURL()                                              )

RECEIVERS:
    virtual void noticeConnectedI    (cmplInterface *, bool pointer_valid) override;
    virtual void noticeDisconnectedI (cmplInterface *, bool pointer_valid) override;
};





#endif
