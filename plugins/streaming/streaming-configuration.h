/***************************************************************************
                       oss-sound-configuration.h  -  description
                             -------------------
    begin                : Thu Sep 30 2004
    copyright            : (C) 2004 by Martin Witte
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

#ifndef KRADIO_STREAMING_CONFIGURATION_H
#define KRADIO_STREAMING_CONFIGURATION_H

class QColorGroup;
#include "ui_streaming-configuration-ui.h"
#include "streaming.h"



#define RATE_48000_IDX      0
#define RATE_44100_IDX      1
#define RATE_32000_IDX      2
#define RATE_22050_IDX      3
#define RATE_11025_IDX      4

#define CHANNELS_STEREO_IDX 0
#define CHANNELS_MONO_IDX   1

#define SIGN_SIGNED_IDX     0
#define SIGN_UNSIGNED_IDX   1

#define BITS_16_IDX         0
#define BITS_8_IDX          1

#define ENDIAN_LITTLE_IDX   0
#define ENDIAN_BIG_IDX      1

#define FORMAT_RAW_IDX      0


class StreamingConfiguration : public QWidget,
                               public Ui_StreamingConfigurationUI
{
Q_OBJECT
public :
    StreamingConfiguration (QWidget *parent, StreamingDevice *streamer);
    ~StreamingConfiguration ();

protected slots:

    void slotOK();
    void slotCancel();

    void slotUpdateConfig();



    void slotNewPlaybackChannel();
    void slotDeletePlaybackChannel();
    void slotUpPlaybackChannel();
    void slotDownPlaybackChannel();

    void slotNewCaptureChannel();
    void slotDeleteCaptureChannel();
    void slotUpCaptureChannel();
    void slotDownCaptureChannel();

    void slotPlaybackSelectionChanged();
    void slotCaptureSelectionChanged();

    void slotUpdateSoundFormat();
    void slotSetDirty();

protected:

    void setStreamOptions(const SoundFormat &sf, int BufferSize);
    void getStreamOptions(SoundFormat &sf, int &BufferSize) const ;


    QList<SoundFormat>      m_PlaybackSoundFormats, m_CaptureSoundFormats;
    QList<int>              m_PlaybackBufferSizes,  m_CaptureBufferSizes;

    bool                    m_ignore_updates;
    bool                    m_dirty;
    StreamingDevice        *m_StreamingDevice;

};

#endif
