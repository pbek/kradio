/***************************************************************************
                          frequencyradio_interfaces.h  -  description
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
 *   IFrequencyRadio(Client)                                                    *
 *                                                                         *
 ***************************************************************************/

#ifndef KRADIO_FREQUENCYRADIO_INTERFACES_H
#define KRADIO_FREQUENCYRADIO_INTERFACES_H

#include "interfaces.h"

class RadioStation;
class FrequencyRadioStation;
class KUrl;

INTERFACE(IFrequencyRadio, IFrequencyRadioClient)
{
public :
    IF_CON_DESTRUCTOR(IFrequencyRadio, -1)

RECEIVERS:
    IF_RECEIVER(  setFrequency(float f, const FrequencyRadioStation *s)                   )
    IF_RECEIVER(  setMinFrequency(float mf)                                      )
    IF_RECEIVER(  setMaxFrequency(float mf)                                      )
    IF_RECEIVER(  setScanStep(float s)                                           )

SENDERS:
    IF_SENDER  (  notifyFrequencyChanged(float f, const FrequencyRadioStation *s)         )
    IF_SENDER  (  notifyMinMaxFrequencyChanged(float min, float max)             )
    IF_SENDER  (  notifyDeviceMinMaxFrequencyChanged(float min, float max)       )
    IF_SENDER  (  notifyScanStepChanged(float s)                                 )

ANSWERS:
    IF_ANSWER  (  float        getFrequency()     const                  );
    IF_ANSWER  (  float        getMinFrequency()  const                  );
    IF_ANSWER  (  float        getMinDeviceFrequency() const             );
    IF_ANSWER  (  float        getMaxFrequency()  const                  );
    IF_ANSWER  (  float        getMaxDeviceFrequency()  const            );
    IF_ANSWER  (  float        getScanStep()      const                  );
};


INTERFACE(IFrequencyRadioClient, IFrequencyRadio)
{
public :
    IF_CON_DESTRUCTOR(IFrequencyRadioClient, 1)

SENDERS:
    IF_SENDER  (  sendFrequency(float f, const FrequencyRadioStation *s = NULL)  )
    IF_SENDER  (  sendMinFrequency(float mf)                                     )
    IF_SENDER  (  sendMaxFrequency(float mf)                                     )
    IF_SENDER  (  sendScanStep(float s)                                          )

RECEIVERS:
    IF_RECEIVER(  noticeFrequencyChanged(float f, const FrequencyRadioStation *s)         )
    IF_RECEIVER(  noticeMinMaxFrequencyChanged(float min, float max)             )
    IF_RECEIVER(  noticeDeviceMinMaxFrequencyChanged(float min, float max)       )
    IF_RECEIVER(  noticeScanStepChanged(float s)                                 )

QUERIES:
    IF_QUERY   (  float queryFrequency()                                         )
    IF_QUERY   (  float queryMinFrequency()                                      )
    IF_QUERY   (  float queryMinDeviceFrequency()                                )
    IF_QUERY   (  float queryMaxFrequency()                                      )
    IF_QUERY   (  float queryMaxDeviceFrequency()                                )
    IF_QUERY   (  float queryScanStep()                                          )

RECEIVERS:
    virtual void noticeConnectedI    (cmplInterface *, bool pointer_valid);
    virtual void noticeDisconnectedI (cmplInterface *, bool pointer_valid);
};



#endif
