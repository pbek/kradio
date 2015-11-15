/***************************************************************************
                          seekradio_interfaces.h  -  description
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
 *   ISeekRadio(Client)                                                    *
 *                                                                         *
 ***************************************************************************/

#ifndef KRADIO_SEEKRADIO_INTERFACES_H
#define KRADIO_SEEKRADIO_INTERFACES_H

#include "soundstreamid.h"
#include "interfaces.h"

class RadioStation;
class FrequencyRadioStation;
class KUrl;

INTERFACE(ISeekRadio, ISeekRadioClient)
{
    friend class SeekHelper;

public :
    IF_CON_DESTRUCTOR(ISeekRadio, -1)

RECEIVERS:
    IF_RECEIVER(  toBeginning()                                  )
    IF_RECEIVER(  toEnd()                                        )
    IF_RECEIVER(  startSeek (bool up)                            )
    IF_RECEIVER(  startSeekUp()                                  )
    IF_RECEIVER(  startSeekDown()                                )
    IF_RECEIVER(  stopSeek()                                     )

SENDERS:
    IF_SENDER  (  notifySeekStarted (bool up)                    )
    IF_SENDER  (  notifySeekStopped ()                           )
    IF_SENDER  (  notifySeekFinished (const RadioStation &s, bool goodQuality)     )
    IF_SENDER  (  notifyProgress (float f)                       )

ANSWERS:
    IF_ANSWER  (  bool  isSeekRunning() const                    );
    IF_ANSWER  (  bool  isSeekUpRunning() const                  );
    IF_ANSWER  (  bool  isSeekDownRunning() const                );
    IF_ANSWER  (  float getProgress () const                     );
};


INTERFACE(ISeekRadioClient, ISeekRadio)
{
public :
    IF_CON_DESTRUCTOR(ISeekRadioClient, 1)

SENDERS:
    IF_SENDER  (  sendToBeginning()                                   )
    IF_SENDER  (  sendToEnd()                                         )
    IF_SENDER  (  sendStartSeek (bool up)                             )
    IF_SENDER  (  sendStartSeekUp()                                   )
    IF_SENDER  (  sendStartSeekDown()                                 )
    IF_SENDER  (  sendStopSeek()                                      )

RECEIVERS:
    IF_RECEIVER(  noticeSeekStarted (bool up)                         )
    IF_RECEIVER(  noticeSeekStopped ()                                )
    IF_RECEIVER(  noticeSeekFinished (const RadioStation &s, bool goodQuality)          )
    IF_RECEIVER(  noticeProgress (float f)                            )

QUERIES:
    IF_QUERY  (  bool  queryIsSeekRunning()                           )
    IF_QUERY  (  bool  queryIsSeekUpRunning()                         )
    IF_QUERY  (  bool  queryIsSeekDownRunning()                       )
    IF_QUERY  (  float queryProgress ()                               )

RECEIVERS:
    virtual void noticeConnectedI    (cmplInterface *, bool pointer_valid);
    virtual void noticeDisconnectedI (cmplInterface *, bool pointer_valid);
};

#endif
