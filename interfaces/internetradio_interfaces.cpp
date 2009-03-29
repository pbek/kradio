/***************************************************************************
                          internetradio_interfaces.cpp  -  description
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


#include "internetradio_interfaces.h"
#include "radiostation.h"

#include <kurl.h>


// IInternetRadio

IF_IMPL_SENDER  (  IInternetRadio::notifyURLChanged(const KUrl &u, const InternetRadioStation *irs),
                   noticeURLChanged(u, irs)                            )

// IInternetRadioClient

IF_IMPL_SENDER  (  IInternetRadioClient::sendURL(const KUrl &url, const InternetRadioStation *irs),
                   setURL(url, irs)                                    )


static KUrl emptyURL;

IF_IMPL_QUERY   (  const KUrl &IInternetRadioClient::queryURL(),
                   getURL(),
                   emptyURL                                       )

void IInternetRadioClient::noticeConnectedI    (cmplInterface *, bool /*pointer_valid*/)
{
    noticeURLChanged(queryURL(), NULL);
}


void IInternetRadioClient::noticeDisconnectedI (cmplInterface *, bool /*pointer_valid*/)
{
    noticeURLChanged(queryURL(), NULL);
}


