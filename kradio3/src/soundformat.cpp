/***************************************************************************
                     soundformat.cpp  -  description
                             -------------------
    begin                : Sun Aug 1 2004
    copyright            : (C) 2004 by Martin Witte
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

#include "include/soundformat.h"

int SoundFormat::sampleSize() const
{
    if (m_SampleBits <= 8)  return 1;
    if (m_SampleBits <= 16) return 2;
    if (m_SampleBits <= 32) return 4;

    // unknown
    return -1;
}


int SoundFormat::frameSize() const
{
    return sampleSize() * m_Channels;
}


int SoundFormat::minValue() const
{
    if (!m_IsSigned) return 0;
    return -(1 << (m_SampleBits - 1));
}


int SoundFormat::maxValue() const
{
    return (1 << (m_SampleBits - m_IsSigned)) - 1;
}


void  SoundFormat::restoreConfig(const QString &prefix, KConfig *c)
{
    m_SampleBits      = c->readNumEntry (prefix + "bits", 16);
    m_IsSigned        = c->readBoolEntry(prefix + "sign", true);
    m_Channels        = c->readNumEntry (prefix + "channels", 2);
    m_SampleRate      = c->readNumEntry (prefix + "samplerate", 44100);
    bool littleEndian = c->readBoolEntry(prefix + "littleEndian", true);
    m_Endianess = littleEndian ? LITTLE_ENDIAN : BIG_ENDIAN;
    m_Encoding        = c->readEntry(prefix + "encoding", "raw");
}


void  SoundFormat::saveConfig(const QString &prefix, KConfig *c) const
{
    c->writeEntry(prefix + "bits",         m_SampleBits);
    c->writeEntry(prefix + "sign",         m_IsSigned);
    c->writeEntry(prefix + "channels",     m_Channels);
    c->writeEntry(prefix + "samplerate",   m_SampleRate);
    c->writeEntry(prefix + "littleEndian", m_Endianess == LITTLE_ENDIAN);
    c->writeEntry(prefix + "encoding",     m_Encoding);
}


int SoundFormat::convertSampleToInt(const char *sample, bool do_scale) const
{
    int size     = sampleSize();

    unsigned val = 0;
    if (m_Endianess == LITTLE_ENDIAN) {
        sample = sample + size - 1;
        for (int i = size - 1; i >= 0; --i, --sample) {
            val = (val << 8) | (unsigned char)*sample;
        }
    } else {
        for (int i = 0; i < size; ++i, ++sample) {
            val = (val << 8) | (unsigned char)*sample;
        }
    }

    int scale    = (sizeof(unsigned) << 3) - m_SampleBits;
    int signmask = do_scale ? (!m_IsSigned << ((sizeof(unsigned) << 3) - 1)) :
                              (-m_IsSigned << ((size << 3)             - 1)) ;
    if (do_scale) {
        // map to int number space
        return (val << scale) ^ signmask;
    } else {
        // do only sign extension
        if (val & signmask)
            val |= signmask;
        return val;
    }
}


void SoundFormat::convertIntToSample(int src, char *dst, bool is_scaled) const
{
    int size     = sampleSize();
    int scale    = (sizeof(unsigned) * 8) - m_SampleBits;
    int signmask = (!m_IsSigned << (sizeof(unsigned) * 8 - 1));

    unsigned val = is_scaled ? (src ^ signmask) >> scale : src;
    if (m_Endianess == LITTLE_ENDIAN) {
        for (int i = 0; i < size; ++i, ++dst) {
            (unsigned char &)*dst = val & 0xFF;
             val >>= 8;
        }
    } else {
        dst = dst - 1 + size;
        for (int i = size - 1; i >= 0; --i, --dst) {
            (unsigned char &)*dst = val & 0xFF;
             val >>= 8;
        }
    }
}


void SoundFormat::convertSamplesToInts(const char *src, int *dst, size_t n, bool do_scale) const
{
    int size  = sampleSize();
    int scale    = (sizeof(unsigned) * 8) - m_SampleBits;
    int signmask = do_scale ? (!m_IsSigned << ((sizeof(unsigned) << 3) - 1)) :
                              (-m_IsSigned << ((size << 3)             - 1)) ;
    if (m_Endianess == LITTLE_ENDIAN) {
        src = src - 1 + (size * n);
        int *end = dst;
        for (dst = dst - 1 + n; dst >= end; --dst) {
            unsigned val = 0;
            for (int i = size - 1; i >= 0; --i, --src) {
                val = (val << 8) | (unsigned char)*src;
            }
            if (do_scale) {
                *dst = (val << scale) ^ signmask;
            } else if (val & signmask) {
                *dst = val | signmask;
            }
        }
    } else {
        for (int *end = dst + n; dst < end; ++dst) {
            unsigned val = 0;
            for (int i = 0; i < size; ++i, ++src) {
                val = (val << 8) | (unsigned char)*src;
            }
            if (do_scale) {
                *dst = (val << scale) ^ signmask;
            } else if (val & signmask) {
                *dst = val | signmask;
            }
        }
    }
}


void SoundFormat::convertIntsToSamples(const int *src, char *dst, size_t n, bool is_scaled) const
{
    int size     = sampleSize();
    int scale    = (sizeof(unsigned) * 8) - m_SampleBits;
    int signmask = (!m_IsSigned << (sizeof(unsigned) * 8 - 1));

    if (m_Endianess == LITTLE_ENDIAN) {
        for (const int *end = src+n; src < end; ++src) {
            unsigned val = is_scaled ? ((unsigned)(*src ^ signmask)) >> scale : *src;
            for (int i = 0; i < size; ++i, ++dst) {
                (unsigned char &)*dst = val & 0xFF;
                val >>= 8;
            }
        }
    } else {
        dst = dst - 1 + (size * n);
        const int *end = src;
        for (src = src - 1 + n; src >= end; --src) {
            unsigned val = is_scaled ? ((unsigned)(*src ^ signmask)) >> scale : *src;
            for (int i = size - 1; i >= 0; --i, --dst) {
                (unsigned char &)*dst = val & 0xFF;
                val >>= 8;
            }
        }
    }
}


void SoundFormat::convertSamplesToFloat(const char *_src, float **_dst, size_t n) const
{
    int sample_size = sampleSize();
    int frame_size  = frameSize();
    int scale       = (sizeof(short) << 3) - m_SampleBits;
    int signmask    = !m_IsSigned << ((sizeof(short) << 3) - 1);
    int skip        = frame_size - sample_size;

    if (m_Endianess == LITTLE_ENDIAN) {
        const char *src_ch0_end = _src + frame_size * (n - 1) + sample_size - 1;
        for (unsigned ch = 0; ch < m_Channels; ++ch) {
            const char *src = src_ch0_end + sample_size * ch;
            float      *dst = _dst[ch];
            float      *end = dst;
            for (dst = dst - 1 + n; dst >= end; --dst) {
                unsigned val = 0;
                for (int i = sample_size - 1; i >= 0; --i, --src) {
                    val = (val << 8) | (unsigned char)*src;
                }
                *dst = (float)(signed short)((val << scale) ^ signmask) / 32768.0f;
                src = src - skip;
            }
        }
    } else {
        for (unsigned ch = 0; ch < m_Channels; ++ch) {
            const char *src = _src + sample_size * ch;
            float      *dst = _dst[ch];
            for (float *end = dst + n; dst < end; ++dst) {
                unsigned val = 0;
                for (int i = 0; i < sample_size; ++i, ++src) {
                    val = (val << 8) | (unsigned char)*src;
                }
                *dst = (float)(signed short)((val << scale) ^ signmask) / 32768.0f;
                src = src + skip;
            }
        }
    }
}



void SoundFormat::convertFloatsToSamples(const float **_src, char *_dst, size_t n) const
{
    int sample_size = sampleSize();
    int frame_size  = frameSize();
    int scale       = (sizeof(short) << 3) - m_SampleBits;
    int signmask    = (!m_IsSigned << (sizeof(short) << 3) - 1);
    int skip        = frame_size - sample_size;

    if (m_Endianess == LITTLE_ENDIAN) {
        for (unsigned ch = 0; ch < m_Channels; ++ch) {
            const float *src = _src[ch];
            char        *dst = _dst + ch * sample_size;
            for (const float *end = src+n; src < end; ++src) {
                unsigned val = ((  ((unsigned)(*src * 32768.0f)) ^ signmask)) >> scale;
                for (int i = 0; i < sample_size; ++i, ++dst) {
                    (unsigned char &)*dst = val & 0xFF;
                    val >>= 8;
                }
                dst = dst + skip;
            }
        }
    } else {
        char  *dst_ch0_end = _dst + frame_size * (n - 1) + sample_size - 1;
        for (unsigned ch = 0; ch < m_Channels; ++ch) {
            char        *dst = dst_ch0_end + sample_size * ch;
            const float *src = _src[ch];
            const float *end = src;
            for (src = src - 1 + n; src >= end; --src) {
                unsigned val = (( ((unsigned)(*src * 32768.0f)) ^ signmask)) >> scale;
                for (int i = sample_size - 1; i >= 0; --i, --dst) {
                    (unsigned char &)*dst = val & 0xFF;
                    val >>= 8;
                }
                dst = dst - skip;
            }
        }
    }
}


