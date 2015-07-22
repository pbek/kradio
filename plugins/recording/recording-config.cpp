/***************************************************************************
                          recording-config.cpp  -  description
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

#include "recording-config.h"

#include <kconfiggroup.h>
#include <sndfile.h>

RecordingConfig::RecordingConfig ()
:   m_EncodeBufferSize(256*1024),
    m_EncodeBufferCount(3),
    m_mp3Quality(7),
    m_oggQuality(1.0),
    m_Directory("/tmp"),
    m_template(),
    m_OutputFormat(outputWAV),
    m_PreRecordingEnable (false),
    m_PreRecordingSeconds(10)
{
    checkFormatSettings();
}

RecordingConfig::RecordingConfig (const QString &directory,
                                  const recordingTemplate_t &templ,
                                  OutputFormat of,
                                  const SoundFormat &sf, int mp3_q, float ogg_q)
:   m_EncodeBufferSize(256*1024),
    m_EncodeBufferCount(3),
    m_SoundFormat(sf),
    m_mp3Quality(mp3_q),
    m_oggQuality(ogg_q),
    m_Directory(directory),
    m_template(templ),
    m_OutputFormat(of),
    m_PreRecordingEnable (false),
    m_PreRecordingSeconds(10)
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
    m_template(c.m_template),
    m_OutputFormat(c.m_OutputFormat),
    m_PreRecordingEnable (false),
    m_PreRecordingSeconds(10)
{
    checkFormatSettings();
}


void  RecordingConfig::restoreConfig(const KConfigGroup &c)
{
    m_EncodeBufferSize  = c.readEntry("encodeBufferSize", 256*1024);
    m_EncodeBufferCount = c.readEntry("encodeBufferCount", 3);

    m_SoundFormat.restoreConfig("", c);
    m_Directory        = c.readEntry("directory", "/tmp");
    m_template.restoreState("template", c, "filenameTemplate");
    m_mp3Quality       = c.readEntry("mp3quality", 7);
    m_oggQuality       = c.readEntry("oggquality", 1.0);
    QString of         = c.readEntry("outputFormat", ".wav");

    if (of == ".wav")
        m_OutputFormat = outputWAV;
    else if (of == ".aiff")
        m_OutputFormat = outputAIFF;
    else if (of == ".au")
        m_OutputFormat = outputAU;
#ifdef HAVE_LAME
    else if (of == ".mp3")
        m_OutputFormat = outputMP3;
#endif
#ifdef HAVE_OGG
    else if (of == ".ogg")
        m_OutputFormat = outputOGG;
#endif
    else if (of == ".raw")
        m_OutputFormat = outputRAW;

    // if there was any unknown format
    else
        m_OutputFormat = outputWAV;

    m_PreRecordingEnable  = c.readEntry("prerecording-enable", false);
    m_PreRecordingSeconds = c.readEntry("prerecording-seconds", 10);

    checkFormatSettings();
}


void  RecordingConfig::saveConfig(KConfigGroup &c) const
{
    c.writeEntry("encodeBufferSize",  (quint64)m_EncodeBufferSize);
    c.writeEntry("encodeBufferCount", (quint64)m_EncodeBufferCount);
    m_SoundFormat.saveConfig("",     c);
    c.writeEntry("directory",        m_Directory);
    m_template.saveState("template", c);
    c.writeEntry("mp3quality",       m_mp3Quality);
    c.writeEntry("oggquality",       m_oggQuality);

    switch(m_OutputFormat) {
        case outputWAV:  c.writeEntry("outputFormat", ".wav");  break;
        case outputAIFF: c.writeEntry("outputFormat", ".aiff"); break;
        case outputAU:   c.writeEntry("outputFormat", ".au");   break;
        case outputMP3:  c.writeEntry("outputFormat", ".mp3");  break;
        case outputOGG:  c.writeEntry("outputFormat", ".ogg");  break;
        case outputRAW:  c.writeEntry("outputFormat", ".raw");  break;
        default:         c.writeEntry("outputFormat", ".wav");  break;
    }

    c.writeEntry("prerecording-enable", m_PreRecordingEnable);
    c.writeEntry("prerecording-seconds", m_PreRecordingSeconds);
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

    if (m_SoundFormat.m_Endianness == LITTLE_ENDIAN)
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
    // correct Endianness and Signs for specific formats
    switch (m_OutputFormat) {
        case outputWAV:
            m_SoundFormat.m_Endianness = LITTLE_ENDIAN;
            if (m_SoundFormat.m_SampleBits == 8)
                m_SoundFormat.m_IsSigned = false;
            // libsndfile only supports signed 16 bit samples
            if (m_SoundFormat.m_SampleBits == 16)
                m_SoundFormat.m_IsSigned = true;
            break;
        case outputAIFF:
            m_SoundFormat.m_Endianness = BIG_ENDIAN;
            // libsndfile only supports signed 16 bit samples
            if (m_SoundFormat.m_SampleBits == 16)
                m_SoundFormat.m_IsSigned = true;
            break;
        case outputAU:
            m_SoundFormat.m_Endianness = BIG_ENDIAN;
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

