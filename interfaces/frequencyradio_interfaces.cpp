/***************************************************************************
                          frequencyradio_interfaces.cpp  -  description
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


#include "frequencyradio_interfaces.h"
#include "radiostation.h"

// IFrequencyRadio

IF_IMPL_SENDER  (  IFrequencyRadio::notifyFrequencyChanged(float f, const FrequencyRadioStation *s),
                   noticeFrequencyChanged(f, s)                   )
IF_IMPL_SENDER  (  IFrequencyRadio::notifyMinMaxFrequencyChanged(float min, float max),
                   noticeMinMaxFrequencyChanged(min, max)         )
IF_IMPL_SENDER  (  IFrequencyRadio::notifyDeviceMinMaxFrequencyChanged(float min, float max),
                   noticeDeviceMinMaxFrequencyChanged(min, max)         )
IF_IMPL_SENDER  (  IFrequencyRadio::notifyScanStepChanged(float s),
                    noticeScanStepChanged(s)                      )

// IFrequencyRadioClient

IF_IMPL_SENDER  (  IFrequencyRadioClient::sendFrequency(float f, const FrequencyRadioStation *s),
                   setFrequency(f, s)                             )
IF_IMPL_SENDER  (  IFrequencyRadioClient::sendMinFrequency(float mf),
                   setMinFrequency(mf)                            )
IF_IMPL_SENDER  (  IFrequencyRadioClient::sendMaxFrequency(float mf),
                   setMaxFrequency(mf)                            )
IF_IMPL_SENDER  (  IFrequencyRadioClient::sendScanStep(float s),
                   setScanStep(s)                                 )

IF_IMPL_QUERY   (  float IFrequencyRadioClient::queryFrequency(),
                   getFrequency(),
                   0                                              )
IF_IMPL_QUERY   (  float IFrequencyRadioClient::queryMinFrequency(),
                   getMinFrequency(),
                   0                                              )
IF_IMPL_QUERY   (  float IFrequencyRadioClient::queryMinDeviceFrequency(),
                   getMinDeviceFrequency(),
                   0                                              )
IF_IMPL_QUERY   (  float IFrequencyRadioClient::queryMaxFrequency(),
                   getMaxFrequency(),
                   0                                              )
IF_IMPL_QUERY   (  float IFrequencyRadioClient::queryMaxDeviceFrequency(),
                   getMaxDeviceFrequency(),
                   0                                              )
IF_IMPL_QUERY   (  float IFrequencyRadioClient::queryScanStep(),
                   getScanStep(),
                   0.05                                           )

void IFrequencyRadioClient::noticeConnectedI    (cmplInterface *, bool /*pointer_valid*/)
{
    noticeFrequencyChanged(queryFrequency(), NULL);
    noticeMinMaxFrequencyChanged(queryMinFrequency(), queryMaxFrequency());
    noticeDeviceMinMaxFrequencyChanged(queryMinDeviceFrequency(), queryMaxDeviceFrequency());
    noticeScanStepChanged(queryScanStep());
}


void IFrequencyRadioClient::noticeDisconnectedI   (cmplInterface *, bool /*pointer_valid*/)
{
    noticeFrequencyChanged(queryFrequency(), NULL);
    noticeMinMaxFrequencyChanged(queryMinFrequency(), queryMaxFrequency());
    noticeDeviceMinMaxFrequencyChanged(queryMinDeviceFrequency(), queryMaxDeviceFrequency());
    noticeScanStepChanged(queryScanStep());
}
