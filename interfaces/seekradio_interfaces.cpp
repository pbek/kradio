/***************************************************************************
                          seekradio_interfaces.cpp  -  description
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


#include "seekradio_interfaces.h"
#include "radiostation.h"


// ISeekRadio

IF_IMPL_SENDER  (  ISeekRadio::notifySeekStarted (bool up),
                   noticeSeekStarted (up)                         );
IF_IMPL_SENDER  (  ISeekRadio::notifySeekStopped (),
                   noticeSeekStopped ()                           );
IF_IMPL_SENDER  (  ISeekRadio::notifySeekFinished (const RadioStation &s, bool goodQuality),
                   noticeSeekFinished (s, goodQuality)            );
IF_IMPL_SENDER  (  ISeekRadio::notifyProgress (float f),
                   noticeProgress (f)                             );


// ISeekRadioClient

IF_IMPL_SENDER  (  ISeekRadioClient::sendToBeginning(),
                   toBeginning()                                  );
IF_IMPL_SENDER  (  ISeekRadioClient::sendToEnd(),
                   toEnd()                                        );
IF_IMPL_SENDER  (  ISeekRadioClient::sendStartSeek (bool up),
                   startSeek (up)                                 );
IF_IMPL_SENDER  (  ISeekRadioClient::sendStartSeekUp(),
                   startSeekUp()                                  );
IF_IMPL_SENDER  (  ISeekRadioClient::sendStartSeekDown(),
                   startSeekDown()                                );
IF_IMPL_SENDER  (  ISeekRadioClient::sendStopSeek(),
                   stopSeek()                                     );

IF_IMPL_QUERY   (  bool ISeekRadioClient::queryIsSeekRunning(),
                   isSeekRunning(),
                   false                                          );
IF_IMPL_QUERY   (  bool ISeekRadioClient::queryIsSeekUpRunning(),
                   isSeekUpRunning(),
                   false                                          );
IF_IMPL_QUERY   (  bool ISeekRadioClient::queryIsSeekDownRunning(),
                   isSeekDownRunning(),
                   false                                          );
IF_IMPL_QUERY   (  float ISeekRadioClient::queryProgress(),
                   getProgress(),
                   1.0                                            );


void ISeekRadioClient::noticeConnectedI    (cmplInterface *, bool /*pointer_valid*/)
{
    if (queryIsSeekRunning()) {
        noticeSeekStarted(queryIsSeekUpRunning());
    } else {
        noticeSeekStopped();
    }
    noticeProgress(queryProgress());
}


void ISeekRadioClient::noticeDisconnectedI   (cmplInterface *, bool /*pointer_valid*/)
{
    noticeSeekStopped();
    noticeProgress(queryProgress());
}


