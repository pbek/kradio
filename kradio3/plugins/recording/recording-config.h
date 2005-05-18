/***************************************************************************
                          recording-config.h  -  description
                             -------------------
    begin                : Mi Apr 30 2005
    copyright            : (C) 2005 by Martin Witte
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

#ifndef KRADIO_RECORDING_CONFIG_H
#define KRADIO_RECORDING_CONFIG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../../src/libkradio/soundformat.h"

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
    RecordingConfig (const QString &directory,
                     OutputFormat of,
                     const SoundFormat &, int mp3_q, float ogg_q);
    RecordingConfig (const RecordingConfig &c);

    void     restoreConfig(KConfig *c);
    void     saveConfig(KConfig *c) const;

    void     getSoundFileInfo(SF_INFO &info, bool input);

    void     checkFormatSettings();

public:
    unsigned int m_EncodeBufferSize;
    unsigned int m_EncodeBufferCount;

    SoundFormat  m_SoundFormat;
    int          m_mp3Quality;
    float        m_oggQuality;
    QString      m_Directory;
    OutputFormat m_OutputFormat;
};




#endif
