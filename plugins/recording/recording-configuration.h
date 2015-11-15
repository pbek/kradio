/***************************************************************************
                          recording-configuration.h  -  description
                             -------------------
    begin                : So Aug 31 2003
    copyright            : (C) 2003 by Martin Witte
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

#ifndef KRADIO_RECORDING_CONFIGURATION_H
#define KRADIO_RECORDING_CONFIGURATION_H

#include "soundformat.h"

#include "recording-config.h"
#include "reccfg_interfaces.h"
#include "ui_recording-configuration-ui.h"

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
#define FORMAT_WAV_IDX      1
#define FORMAT_AIFF_IDX     2
#define FORMAT_AU_IDX       3
#define NEXT_IDX1           4

#define FORMAT_MP3_IDX_ORG  4
#define FORMAT_OGG_IDX_ORG  5


#ifdef HAVE_LAME
    #define FORMAT_MP3_IDX      NEXT_IDX1
    #define NEXT_IDX2           (NEXT_IDX1+1)
#else
    #define FORMAT_MP3_IDX      (-1)
    #define NEXT_IDX2           NEXT_IDX1
#endif

#ifdef HAVE_OGG
    #define FORMAT_OGG_IDX      NEXT_IDX2
    #define NEXT_IDX3           (NEXT_IDX2+1)
#else
    #define FORMAT_OGG_IDX      (-1)
    #define NEXT_IDX3           NEXT_IDX2
#endif






class RecordingConfiguration : public QWidget,
                               public Ui_RecordingConfigurationUI,
                               public IRecCfgClient
{
Q_OBJECT
public :
    RecordingConfiguration (QWidget *parent);
    ~RecordingConfiguration ();

// IRecCfgClient

    bool noticeEncoderBufferChanged     (size_t BufferSize, size_t BufferCount);
    bool noticeSoundFormatChanged       (const SoundFormat &sf);
    bool noticeMP3QualityChanged        (int   q);
    bool noticeOggQualityChanged        (float q);
    bool noticeRecordingDirectoryChanged(const QString &dir, const recordingTemplate_t &templ);
    bool noticeOutputFormatChanged      (RecordingConfig::OutputFormat of);
    bool noticePreRecordingChanged      (bool enable, int seconds);
    bool noticeRecordingConfigChanged   (const RecordingConfig &cfg);

protected slots:

    void slotOK();
    void slotCancel();
    void slotSetDirty();

    void slotFormatSelectionChanged();

protected:

    void storeConfig();

    void setGUIBuffers(const RecordingConfig &c);
    void setGUIDirectories(const RecordingConfig &c);
    void setGUISoundFormat(const RecordingConfig &c);
    void setGUIOutputFormat(const RecordingConfig &c);
    void setGUIPreRecording(const RecordingConfig &c);
    void setGUIEncoderQuality(const RecordingConfig &c);

    RecordingConfig m_RecordingConfig;

    bool  m_dirty;
    bool  m_ignore_gui_updates;
};




#endif
