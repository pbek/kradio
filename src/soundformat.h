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
    unsigned     m_Endianness;
    QString      m_Encoding;     // "raw", "mp3", ...  (no "wav", because it's only header + raw data)
    bool         m_IsPlanar;

    SoundFormat(unsigned sample_rate, unsigned channels, unsigned sample_bits, bool is_signed, unsigned endianness, const QString &enc, bool is_planar)
        : m_SampleRate(sample_rate),
          m_Channels  (channels),
          m_SampleBits(sample_bits),
          m_IsSigned  (is_signed),
          m_Endianness (endianness),
          m_Encoding  (enc),
          m_IsPlanar  (is_planar)
    {}
        
    SoundFormat(unsigned sample_rate, unsigned channels, unsigned sample_bits, bool is_signed, unsigned endianness, bool is_planar)
        : m_SampleRate(sample_rate),
          m_Channels  (channels),
          m_SampleBits(sample_bits),
          m_IsSigned  (is_signed),
          m_Endianness (endianness),
          m_Encoding  ("raw"),
          m_IsPlanar  (is_planar)
    {}
    
    SoundFormat(unsigned sample_rate, unsigned channels, unsigned sample_bits, bool is_signed, bool is_planar)
        : m_SampleRate(sample_rate),
          m_Channels  (channels),
          m_SampleBits(sample_bits),
          m_IsSigned  (is_signed),
          m_Endianness (BYTE_ORDER),
          m_Encoding  ("raw"),
          m_IsPlanar  (is_planar)
    {}
    
    SoundFormat(bool stereo, bool is_planar)
        : m_SampleRate(44100),
          m_Channels  (stereo ? 2 : 1),
          m_SampleBits(16),
          m_IsSigned  (true),
          m_Endianness (BYTE_ORDER),
          m_Encoding  ("raw"),
          m_IsPlanar  (is_planar)
    {}
        
    SoundFormat()
        : m_SampleRate(44100),
          m_Channels  (2),
          m_SampleBits(16),
          m_IsSigned  (true),
          m_Endianness (BYTE_ORDER),
          m_Encoding  ("raw"),
          m_IsPlanar  (false)
    {}

    bool operator == (const SoundFormat &o) const { return m_SampleRate == o.m_SampleRate &&
                                                           m_Channels   == o.m_Channels   &&
                                                           m_SampleBits == o.m_SampleBits &&
                                                           m_IsSigned   == o.m_IsSigned   &&
                                                           m_Endianness == o.m_Endianness  &&
                                                           m_Encoding   == o.m_Encoding   &&
                                                           m_IsPlanar   == o.m_IsPlanar
                                                    ;
                                                  }

    bool operator != (const SoundFormat &o) const  { return !operator == (o); }

    int      sampleSize() const;      // size of a single sample
    int      frameSize() const;       // sampleSize * channels
    int      minValue() const;
    int      maxValue() const;

    void     restoreConfig(const QString &prefix, const KConfigGroup &c);
    void     saveConfig   (const QString &prefix,       KConfigGroup &c) const;

    void     convertSamplesToFloatInterleaved   (const char   *src, float  *dst, size_t n_frames) const;
    void     convertFloatInterleavedToSamples   (const float  *src, char   *dst, size_t n_frames) const;
    void     convertSamplesToFloatNonInterleaved(const char   *src, float **dst, size_t n_frames) const;
    void     convertFloatNonInterleavedToSamples(const float **src, char   *dst, size_t n_frames) const;

    void     convertNonInterleavedToInterleaved (const char   *src, char   *dst, size_t n_frames) const;

    void     scaleSamples    (char *_src, float scale, size_t n_frames) const;
    void     minMaxAvgMagnitudePerChannel(const char *src, size_t n_frames, double *vmin, double *vmax, double *vavg) const;

    bool     isValid() const { return m_SampleRate > 1000 && m_Channels > 0 && m_SampleBits >= 8; }

};


#endif
