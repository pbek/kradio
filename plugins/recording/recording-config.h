/***************************************************************************
                          recording-config.h  -  description
                             -------------------
    begin                : Mi Apr 30 2005
    copyright            : (C) 2005 by Martin Witte
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

#ifndef KRADIO_RECORDING_CONFIG_H
#define KRADIO_RECORDING_CONFIG_H

#include "soundformat.h"
#include "recording_template.h"

class KConfig;
struct SF_INFO;

class RecordingConfig
{
public:
    enum OutputFormat {
        outputWAV,
        outputAIFF,
        outputAU,
        outputMP3,
        outputOGG,
        outputRAW
    };

public:
    RecordingConfig ();
    RecordingConfig (const QString             &directory,
                     const recordingTemplate_t &templ,
                     OutputFormat               of,
                     const SoundFormat         &sf,
                     int                        mp3_q,
                     float                      ogg_q);
    RecordingConfig (const RecordingConfig &c);

    void     saveConfig   (      KConfigGroup &c) const;
    void     restoreConfig(const KConfigGroup &c);

    void     getSoundFileInfo(SF_INFO &info, bool input);

    void     checkFormatSettings();

public:
    size_t              m_EncodeBufferSize;
    size_t              m_EncodeBufferCount;

    SoundFormat         m_SoundFormat;
    int                 m_mp3Quality;
    float               m_oggQuality;
    QString             m_Directory;
    recordingTemplate_t m_template;
    OutputFormat        m_OutputFormat;

    bool                m_PreRecordingEnable;
    int                 m_PreRecordingSeconds;
};




#endif
