/***************************************************************************
                          recording-configuration.h  -  description
                             -------------------
    begin                : So Aug 31 2003
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

#ifndef KRADIO_RECORDING_CONFIGURATION_H
#define KRADIO_RECORDING_CONFIGURATION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "recording-configuration-ui.h"
#include "recording-interfaces.h"

#define RATE_44100_IDX      0
#define RATE_22050_IDX      1
#define RATE_11025_IDX      2

#define CHANNELS_STEREO_IDX 0
#define CHANNELS_MONO_IDX   1

#define SIGN_SIGNED_IDX     0
#define SIGN_UNSIGNED_IDX   1

#define BITS_16_IDX         0
#define BITS_8_IDX          1

#define ENDIAN_LITTLE_IDX   0
#define ENDIAN_BIG_IDX      1

#define FORMAT_WAV_IDX      0
#define FORMAT_AIFF_IDX     1
#define FORMAT_AU_IDX       2
#define FORMAT_RAW_IDX      3
#define FORMAT_MP3_IDX      4

class RecordingConfiguration : public RecordingConfigurationUI,
                               public IRecordingClient
{
Q_OBJECT
public :
    RecordingConfiguration (QWidget *parent);
    ~RecordingConfiguration ();

//    bool connectI (Interface *i);
//    bool disconnectI (Interface *i);

// IRecordingClient

    bool noticeRecordingStarted();
    bool noticeMonitoringStarted();
    bool noticeRecordingStopped();
    bool noticeMonitoringStopped();
    bool noticeRecordingConfigChanged(const RecordingConfig &);
    bool noticeRecordingContextChanged(const RecordingContext &c);

protected slots:

    void slotOK();
    void slotCancel();

    void slotFormatSelectionChanged();

protected:

};




#endif
