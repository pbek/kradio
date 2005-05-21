/***************************************************************************
                          recording-config.cpp  -  description
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

#include "recording-config.h"

#include <sndfile.h>

RecordingConfig::RecordingConfig ()
:   m_EncodeBufferSize(256*1024),
    m_EncodeBufferCount(3),
    m_mp3Quality(7),
    m_oggQuality(1.0),
    m_Directory("/tmp"),
    m_OutputFormat(outputWAV)
{
    checkFormatSettings();
}

RecordingConfig::RecordingConfig (const QString &directory,
                                  OutputFormat of,
                                  const SoundFormat &sf, int mp3_q, float ogg_q)
:   m_EncodeBufferSize(256*1024),
    m_EncodeBufferCount(3),
    m_SoundFormat(sf),
    m_mp3Quality(mp3_q),
    m_oggQuality(ogg_q),
    m_Directory(directory),
    m_OutputFormat(of)
{
    checkFormatSettings();
}


RecordingConfig::RecordingConfig (const RecordingConfig &c)
  :
    m_EncodeBufferSize(c.m_EncodeBufferSize),
    m_EncodeBufferCount(c.m_EncodeBufferCount),
    m_SoundFormat(c.m_SoundFormat),
    m_mp3Quality(c.m_mp3Quality),
    m_oggQuality(c.m_oggQuality),
    m_Directory(c.m_Directory),
    m_OutputFormat(c.m_OutputFormat)
{
    checkFormatSettings();
}


void  RecordingConfig::restoreConfig(KConfig *c)
{
    m_EncodeBufferSize  = c->readNumEntry("encodeBufferSize", 256*1024);
    m_EncodeBufferCount = c->readNumEntry("encodeBufferCount", 3);

    m_SoundFormat.restoreConfig("", c);
    m_Directory    = c->readEntry("directory", "/tmp");
    m_mp3Quality   = c->readNumEntry("mp3quality", 7);
    m_oggQuality   = c->readDoubleNumEntry("oggquality", 1.0);
    QString of     = c->readEntry("outputFormat", ".wav");

    if (of == ".wav")
        m_OutputFormat = outputWAV;
    else if (of == ".aiff")
        m_OutputFormat = outputAIFF;
    else if (of == ".au")
        m_OutputFormat = outputAU;
#ifdef HAVE_LAME_LAME_H
    else if (of == ".mp3")
        m_OutputFormat = outputMP3;
#endif
#if defined(HAVE_VORBIS_VORBISENC_H) && defined(HAVE_OGG_OGG_H)
    else if (of == ".ogg")
        m_OutputFormat = outputOGG;
#endif
    else if (of == ".raw")
        m_OutputFormat = outputRAW;

    // if there was any unknown format
    else
        m_OutputFormat = outputWAV;

    checkFormatSettings();
}


void  RecordingConfig::saveConfig(KConfig *c) const
{
    c->writeEntry("encodeBufferSize", m_EncodeBufferSize);
    c->writeEntry("encodeBufferCount", m_EncodeBufferCount);
    m_SoundFormat.saveConfig("", c);
    c->writeEntry("directory",    m_Directory);
    c->writeEntry("mp3quality",   m_mp3Quality);
    c->writeEntry("oggquality",   m_oggQuality);

    switch(m_OutputFormat) {
        case outputWAV:  c->writeEntry("outputFormat", ".wav");  break;
        case outputAIFF: c->writeEntry("outputFormat", ".aiff"); break;
        case outputAU:   c->writeEntry("outputFormat", ".au");   break;
        case outputMP3:  c->writeEntry("outputFormat", ".mp3");  break;
        case outputOGG:  c->writeEntry("outputFormat", ".ogg");  break;
        case outputRAW:  c->writeEntry("outputFormat", ".raw");  break;
        default:         c->writeEntry("outputFormat", ".wav");  break;
    }
}


void RecordingConfig::getSoundFileInfo(SF_INFO &sinfo, bool input)
{
    checkFormatSettings();

    sinfo.samplerate = m_SoundFormat.m_SampleRate;
    sinfo.channels   = m_SoundFormat.m_Channels;
    sinfo.format     = 0;
    sinfo.seekable   = !input;

    // U8 only supported for RAW and WAV
    if (m_SoundFormat.m_SampleBits == 8) {
        if ((m_SoundFormat.m_IsSigned &&
             m_OutputFormat != outputWAV) ||
            m_OutputFormat == outputAU
        ) {
            sinfo.format |= SF_FORMAT_PCM_S8;
        } else {
            sinfo.format |= SF_FORMAT_PCM_U8;
        }
    }
    if (m_SoundFormat.m_SampleBits == 16)
        sinfo.format |= SF_FORMAT_PCM_16;

    if (m_SoundFormat.m_Endianess == LITTLE_ENDIAN)
        sinfo.format |= SF_ENDIAN_LITTLE;
    else
        sinfo.format |= SF_ENDIAN_BIG;

    if (input) {
        sinfo.format |= SF_FORMAT_RAW;
    } else {
        switch (m_OutputFormat) {
            case outputWAV:  sinfo.format |= SF_FORMAT_WAV;  break;
            case outputAIFF: sinfo.format |= SF_FORMAT_AIFF; break;
            case outputAU:   sinfo.format |= SF_FORMAT_AU;   break;
            case outputRAW:  sinfo.format |= SF_FORMAT_RAW;  break;
            default:         sinfo.format |= SF_FORMAT_WAV;  break;
        }
    }
}


void RecordingConfig::checkFormatSettings()
{
    // correct Endianess and Signs for specific formats
    switch (m_OutputFormat) {
        case outputWAV:
            m_SoundFormat.m_Endianess = LITTLE_ENDIAN;
            if (m_SoundFormat.m_SampleBits == 8)
                m_SoundFormat.m_IsSigned = false;
            // libsndfile only supports signed 16 bit samples
            if (m_SoundFormat.m_SampleBits == 16)
                m_SoundFormat.m_IsSigned = true;
            break;
        case outputAIFF:
            m_SoundFormat.m_Endianess = BIG_ENDIAN;
            // libsndfile only supports signed 16 bit samples
            if (m_SoundFormat.m_SampleBits == 16)
                m_SoundFormat.m_IsSigned = true;
            break;
        case outputAU:
            m_SoundFormat.m_Endianess = BIG_ENDIAN;
            m_SoundFormat.m_IsSigned = true;
            // libsndfile only supports signed 16 bit samples
            if (m_SoundFormat.m_SampleBits == 16)
                m_SoundFormat.m_IsSigned = true;
            break;
        case outputMP3:
            m_SoundFormat.m_IsSigned   = true;
            m_SoundFormat.m_SampleBits = 16;
            break;
        case outputOGG:
            m_SoundFormat.m_IsSigned   = true;
            m_SoundFormat.m_SampleBits = 16;
            break;
        case outputRAW:
            // libsndfile only supports signed 16 bit samples
            if (m_SoundFormat.m_SampleBits == 16)
                m_SoundFormat.m_IsSigned = true;
            break;
        default:
            break;
    }
}
