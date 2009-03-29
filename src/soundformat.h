/***************************************************************************
                      soundformat.h  -  description
                             -------------------
    begin                : Sun Aug 1 2004
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

#ifndef KRADIO_SOUNDFORMAT_H
#define KRADIO_SOUNDFORMAT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <endian.h>
#include <QtCore/QString>
#include <kconfig.h>

class KConfigGroup;

struct KDE_EXPORT SoundFormat {
    unsigned     m_SampleRate;
    unsigned     m_Channels;
    unsigned     m_SampleBits;
    bool         m_IsSigned;
    unsigned     m_Endianess;
    QString      m_Encoding;     // "raw", "mp3", ...  (no "wav", because it's only header + raw data)

    SoundFormat(unsigned sample_rate, unsigned channels, unsigned sample_bits, bool is_signed, unsigned endianess, const QString &enc)
        : m_SampleRate(sample_rate), m_Channels(channels), m_SampleBits(sample_bits), m_IsSigned(is_signed), m_Endianess(endianess), m_Encoding(enc) {}
    SoundFormat(unsigned sample_rate, unsigned channels, unsigned sample_bits, bool is_signed, unsigned endianess)
        : m_SampleRate(sample_rate), m_Channels(channels), m_SampleBits(sample_bits), m_IsSigned(is_signed), m_Endianess(endianess), m_Encoding("raw") {}
    SoundFormat(unsigned sample_rate, unsigned channels, unsigned sample_bits, bool is_signed)
        : m_SampleRate(sample_rate), m_Channels(channels), m_SampleBits(sample_bits), m_IsSigned(is_signed), m_Endianess(BYTE_ORDER), m_Encoding("raw") {}
    SoundFormat(bool stereo)
        : m_SampleRate(44100), m_Channels(stereo ? 2 : 1), m_SampleBits(16), m_IsSigned(true), m_Endianess(BYTE_ORDER), m_Encoding("raw") {}
    SoundFormat()
        : m_SampleRate(44100), m_Channels(2), m_SampleBits(16), m_IsSigned(true), m_Endianess(BYTE_ORDER), m_Encoding("raw") {}

    bool operator == (const SoundFormat &o) const { return m_SampleRate == o.m_SampleRate &&
                                                           m_Channels   == o.m_Channels   &&
                                                           m_SampleBits == o.m_SampleBits &&
                                                           m_IsSigned   == o.m_IsSigned   &&
                                                           m_Endianess  == o.m_Endianess  &&
                                                           m_Encoding   == o.m_Encoding
                                                    ;
                                                   }
    bool operator != (const SoundFormat &o) const  { return !operator == (o); }

    int      sampleSize() const;      // size of a single sample
    int      frameSize() const;       // sampleSize * channels
    int      minValue() const;
    int      maxValue() const;

    void     restoreConfig(const QString &prefix, const KConfigGroup &c);
    void     saveConfig   (const QString &prefix,       KConfigGroup &c) const;

    int      convertSampleToInt(const char *sample, bool do_scale) const;
    void     convertIntToSample(int src, char *dst, bool is_scaled) const;
    void     convertSamplesToInts(const char *src, int  *dst, size_t n, bool do_scale) const;
    void     convertIntsToSamples(const int  *src, char *dst, size_t n, bool is_scaled) const;
    void     convertSamplesToFloat (const char   *src, float **dst, size_t n_frames) const;
    void     convertFloatsToSamples(const float **src,   char *dst, size_t n_frames) const;

    void     scaleSamples(char *_src, float scale, size_t n_frames) const;

};


#endif
