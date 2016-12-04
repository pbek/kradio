/***************************************************************************
                     soundformat.cpp  -  description
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

#include "soundformat.h"

#include <kconfig.h>
#include <kconfiggroup.h>

#include <boost/type_traits.hpp>
#include <boost/static_assert.hpp>

#include <math.h>

int SoundFormat::sampleSize() const
{
    if (m_SampleBits <= 8)  return 1;
    if (m_SampleBits <= 16) return 2;
    if (m_SampleBits <= 32) return 4;
    if (m_SampleBits <= 48) return 6; // FIXME: verify!
    if (m_SampleBits == 64) return 8;

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


void  SoundFormat::restoreConfig(const QString &prefix, const KConfigGroup &c)
{
    m_SampleBits      = c.readEntry (prefix + "bits", 16);
    m_IsSigned        = c.readEntry(prefix + "sign", true);
    m_Channels        = c.readEntry (prefix + "channels", 2);
    m_SampleRate      = c.readEntry (prefix + "samplerate", 44100);
    bool littleEndian = c.readEntry(prefix + "littleEndian", true);
    m_Endianness = littleEndian ? LITTLE_ENDIAN : BIG_ENDIAN;
    m_Encoding        = c.readEntry(prefix + "encoding", "raw");
    m_IsPlanar        = c.readEntry(prefix + "planar",   false);
}


void  SoundFormat::saveConfig(const QString &prefix, KConfigGroup &c) const
{
    c.writeEntry(prefix + "bits",         m_SampleBits);
    c.writeEntry(prefix + "sign",         m_IsSigned);
    c.writeEntry(prefix + "channels",     m_Channels);
    c.writeEntry(prefix + "samplerate",   m_SampleRate);
    c.writeEntry(prefix + "littleEndian", m_Endianness == LITTLE_ENDIAN);
    c.writeEntry(prefix + "encoding",     m_Encoding);
    c.writeEntry(prefix + "planar",       m_IsPlanar);
}




#define  BITS(T)        (sizeof(T) << 3)
#define  IS_SIGNED(T)   (((T)(~((T)0))) < (T)0)

template<typename T, int c_bits> inline T maxValue(int var_bits = 0)
{
    int  bits     = c_bits ? c_bits     : var_bits;
    return (((((T)1) << (bits - 1 - IS_SIGNED(T))) - 1) << 1) | 1;
}


template<typename T, int c_bits> inline T minValue(int var_bits = 0)
{
    if (IS_SIGNED(T)) {
        return -maxValue<T, c_bits>(var_bits) - 1;
    } else {
        return 0;
    }
}



template<typename T, int c_bits>
inline T bias(int var_bits = 0)
{
    const bool isSigned = IS_SIGNED(T);
    const int  bits     = c_bits ? c_bits : var_bits;
    if (isSigned) {
        return 0;
    } else {
        return ((T)1) << (bits - 1);
    }
}



template<typename T, int c_fromBits>
inline T signExtend(T v, int var_fromBits = 0)
{
    const int  fromBits   = c_fromBits ? c_fromBits    : var_fromBits;
    const bool c_isSigned = IS_SIGNED(T);
    const int  c_dstBits  = BITS(T);
    const int  shift      = c_isSigned ? c_dstBits - fromBits : 0;
    if (c_isSigned && shift > 0) {
        return (v << shift) >> shift;
    } else {
        return v;
    }
}



template<typename srcT, typename dstT, int c_srcBits, int c_dstBits>
inline dstT scale(srcT v, int var_srcBits = 0, int var_dstBits = 0)
{
    const srcT biasOld     = bias<srcT, c_srcBits>(var_srcBits);
    const dstT biasNew     = bias<dstT, c_dstBits>(var_dstBits);
    const int  srcBits     = c_srcBits ? c_srcBits : var_srcBits;
    const int  dstBits     = c_dstBits ? c_dstBits : var_dstBits;

    // to avoid warnings about neg shifts
    const int  shift       = srcBits > dstBits ? srcBits - dstBits : dstBits - srcBits;

    if (srcBits > dstBits) {
        return (((srcT)v - biasOld) >> shift) + biasNew;
    } else {
        return (((dstT)v - biasOld) << shift) + biasNew;
    }
}


template<typename srcT, typename dstT, int c_srcBits, int c_dstBits>
inline dstT saturate(srcT v, int var_srcBits = 0, int var_dstBits = 0)
{
    const srcT biasOld   = bias<srcT, c_srcBits>(var_srcBits);
    const dstT biasNew   = bias<dstT, c_dstBits>(var_dstBits);

    // saturation in case destT cannot represent the full sample range
    const srcT mappedBiasDiff = bias<srcT, c_srcBits>(var_srcBits) - bias<srcT, c_dstBits>(var_dstBits);
    const srcT minDstVal = minValue<srcT, c_dstBits>(var_dstBits) + mappedBiasDiff; // value does not matter if src_bits <= dst_bits
    const srcT maxDstVal = maxValue<srcT, c_dstBits>(var_dstBits) + mappedBiasDiff; // value does not matter if src_bits <= dst_bits
    const int  src_bits  = c_srcBits ? c_srcBits : var_srcBits;
    const int  dst_bits  = c_dstBits ? c_dstBits : var_dstBits;

    srcT tmp = v;
    if (src_bits > dst_bits) {
        tmp =  qBound(minDstVal, v, maxDstVal);
    }
    return tmp + biasNew - biasOld;
}


template<typename srcT, typename dstT, int c_srcBits, int c_dstBits, bool do_scale>
class convertSample
{
public:
    inline dstT operator()(srcT src, int var_srcBits, int var_dstBits)
    {
        if (do_scale) {
            return scale   <srcT, dstT, c_srcBits, c_dstBits>(src, var_srcBits, var_dstBits);
        } else {
            return saturate<srcT, dstT, c_srcBits, c_dstBits>(src, var_srcBits, var_dstBits);
        }
    }
};


template<typename srcT, typename dstT, int c_srcBits, bool do_scale>
class convertSampleToFlt
{
public:
    inline dstT operator()(srcT src, int var_srcBits, int /*var_dstBits*/)
    {
        const srcT   biasSrc = bias<srcT, c_srcBits>(var_srcBits);
        const int    srcBits = c_srcBits ? c_srcBits : var_srcBits;
        dstT         tmp     = ((dstT)src) - (dstT)biasSrc;
        if (do_scale) {
            dstT   scale = 1.0 / ( 2.0 * (((srcT)1) << (srcBits - 2)));
            return tmp * scale;
        } else {
            return tmp;
        }
    }
};

template<typename srcT, int c_srcBits, int c_dstBits, bool do_scale>
class convertSample<srcT, double, c_srcBits, c_dstBits, do_scale> : public convertSampleToFlt<srcT, double, c_srcBits, do_scale>
{};

template<typename srcT, int c_srcBits, int c_dstBits, bool do_scale>
class convertSample<srcT, float,  c_srcBits, c_dstBits, do_scale> : public convertSampleToFlt<srcT, double, c_srcBits, do_scale>
{};


template<typename srcT, typename dstT, int c_dstBits, bool do_scale>
class convertSampleFromFlt
{
public:
    inline dstT operator()(srcT src, int /*var_srcBits*/, int var_dstBits)
    {
        const dstT biasDst = bias<dstT, c_dstBits>(var_dstBits);
        const int  dstBits = c_dstBits ? c_dstBits : var_dstBits;
        srcT       tmp     = src + biasDst;

        if (do_scale) {
            srcT  scale =  2.0 * (((dstT)1) << (dstBits - 2));
            tmp *= scale;
        }
        const srcT minDst = minValue<dstT, c_dstBits>(var_dstBits);
        const srcT maxDst = maxValue<dstT, c_dstBits>(var_dstBits);
        return qBound(minDst, tmp, maxDst);
    }
};


template<typename dstT, int c_srcBits, int c_dstBits, bool do_scale>
class convertSample<double, dstT, c_srcBits, c_dstBits, do_scale> : public convertSampleFromFlt<double, dstT, c_dstBits, do_scale>
{};

template<typename dstT, int c_srcBits, int c_dstBits, bool do_scale>
class convertSample<float,  dstT, c_srcBits, c_dstBits, do_scale> : public convertSampleFromFlt<double, dstT, c_dstBits, do_scale>
{};


inline unsigned int systemEndianness()
{
    union {
        unsigned int  endiannessTest;
        unsigned char bytes[sizeof(int)];
    };
    endiannessTest = 0x12345678;
    return (bytes[0] == 0x78) ? LITTLE_ENDIAN : BIG_ENDIAN;
}


template<unsigned int endianness>
inline bool needsEndiannessSwap()
{
    static const int cachedSystemEndianness = systemEndianness();
    return cachedSystemEndianness != endianness;
}


template<typename T, int bits>  class bswap
{
public:
    inline T operator () (T v);
};


template<typename T> class bswap<T, 8>
{
public:
    inline T operator ()(T v) {
        BOOST_STATIC_ASSERT(sizeof(T) == 1);
        BOOST_STATIC_ASSERT(boost::is_arithmetic<T>::value);
        return v;
    }
};


template<typename T> class bswap<T, 16>
{
public:
    inline T operator () (T v) {
        BOOST_STATIC_ASSERT(sizeof(T) == 2);
        BOOST_STATIC_ASSERT(boost::is_arithmetic<T>::value);
#ifdef __bswap_16
        return __bswap_16(v);
#else
        return   (((unsigned short)v & 0x00FFu) << 8)
               | (((unsigned short)v & 0xFF00u) >> 8);
#endif
    }
};


template<typename T> class bswap<T, 32>
{
public:
    inline T operator () (T v) {
        BOOST_STATIC_ASSERT(sizeof(T) == 4);
        BOOST_STATIC_ASSERT(boost::is_arithmetic<T>::value);
#ifdef __bswap_32
        return __bswap_32(v);
#else
        return   ((v & 0x000000FFu) << 24)
               | ((v & 0x0000FF00u) <<  8)
               | ((v & 0x00FF0000u) >>  8)
               | ((v & 0xFF000000u) >> 24);
#endif
    }
};


template<typename T> class bswap<T, 24>
{
public:
    inline T operator () (T v) {
        BOOST_STATIC_ASSERT(sizeof(T) == 4);
        BOOST_STATIC_ASSERT(boost::is_arithmetic<T>::value);
        return bswap<T, 32>()(v) >> 8;
    }
};


template<typename T> class bswap<T, 64>
{
public:
    inline T operator () (T v) {
        BOOST_STATIC_ASSERT(sizeof(T) == 8);
        BOOST_STATIC_ASSERT(boost::is_arithmetic<T>::value);
#ifdef __bswap_64
        return __bswap_64(v);
#else
        return   ((v & 0x00000000000000FFull) << 56)
               | ((v & 0x000000000000FF00ull) << 40)
               | ((v & 0x0000000000FF0000ull) << 24)
               | ((v & 0x00000000FF000000ull) <<  8)
               | ((v & 0x000000FF00000000ull) >>  8)
               | ((v & 0x0000FF0000000000ull) >> 24)
               | ((v & 0x00FF000000000000ull) >> 40)
               | ((v & 0xFF00000000000000ull) >> 56);
#endif
    }
};


template<typename T, int bits>
void check_bswap_static_assertions()
{
    // ensure the data is only 1, 2, 3, 4 or 8 bytes
    BOOST_STATIC_ASSERT(   (sizeof(T) == 1 && bits ==  8)
                        || (sizeof(T) == 2 && bits == 16)
                        || (sizeof(T) == 4 && bits == 24)
                        || (sizeof(T) == 4 && bits == 32)
                        || (sizeof(T) == 8 && bits == 64));

    // ensure we're only swapping arithmetic types
    BOOST_STATIC_ASSERT(boost::is_arithmetic<T>::value);
}


template<typename T, int bits, unsigned int otherEndianness>
inline T toFromSystemEndianness(T v)
{
    check_bswap_static_assertions<T, bits>();
    return !needsEndiannessSwap<otherEndianness>() ? v : bswap<T, bits>()(v);
}




/// read sample (now sign extension, scaling, unbiasing...) and proceed with sample pointer
template<typename storageT, int c_bits, unsigned int c_srcEndianness>
class readSampleRaw
{
public:
    inline storageT operator()(const char *&samplePtr, int /*var_size*/ = 0)
    {
        check_bswap_static_assertions<storageT, c_bits>();

        // 24 bits are also saved in a full 32 bit word in sample memory
        storageT tmp = *(storageT*)samplePtr;
        samplePtr += sizeof(storageT);

        // fix endianness
        tmp   = toFromSystemEndianness<storageT, c_bits, c_srcEndianness>(tmp);
        return tmp;
    }
};


/// read sample (now sign extension, scaling, unbiasing...) and proceed with sample pointer
template<typename storageT, unsigned int c_srcEndianness>
class readSampleRaw<storageT, 0, c_srcEndianness>
{
public:
    inline storageT operator()(const char *&samplePtr, int var_size)
    {
        storageT val = 0;
        if (c_srcEndianness == LITTLE_ENDIAN) {
            samplePtr = samplePtr + var_size - 1;
            for (int i = var_size - 1; i >= 0; --i, --samplePtr) {
                val = (val << 8) | (unsigned char)*samplePtr;
            }
            samplePtr += var_size + 1;
        } else {
            for (int i = 0; i < var_size; ++i, ++samplePtr) {
                val = (val << 8) | (unsigned char)*samplePtr;
            }
        }
        return val;
    }
};



/// read sample and proceed with sample pointer
template<typename destT, typename storageT, int c_bits, unsigned int c_srcEndianness, bool c_do_scale>
inline destT readSample(const char *&samplePtr, int var_bits = 0, int var_size = 0)
{
    // 24 bits are also saved in a full 32 bit word in sample memory
    storageT tmp = readSampleRaw<storageT, c_bits, c_srcEndianness>()(samplePtr, var_size);
    // sign extend
    tmp   = signExtend<storageT, c_bits>(tmp);

    return convertSample<storageT, destT, c_bits, BITS(destT), c_do_scale>()(tmp, var_bits, BITS(destT));
}










/// write sample and proceed with sample pointer
template<typename storageT, int c_bits, unsigned int c_dstEndianness>
class writeSampleRaw
{
public:
    inline void operator()(storageT v, char *&samplePtr, int /*var_samplesize*/ = 0)
    {
        check_bswap_static_assertions<storageT, c_bits>();

        // 24 bits are also saved in a full 32 bit word in sample memory
        *(storageT*)samplePtr = toFromSystemEndianness<storageT, c_bits, c_dstEndianness>(v);
        samplePtr += sizeof(storageT);
    }
};



/// write sample and proceed with sample pointer
template<typename storageT, unsigned int c_dstEndianness>
class writeSampleRaw<storageT, 0, c_dstEndianness>
{
public:
    inline void operator()(storageT val, char *&samplePtr, int var_samplesize)
    {
        if (c_dstEndianness == LITTLE_ENDIAN) {
            char *dst = samplePtr;
            for (int i = 0; i < var_samplesize; ++i, ++dst) {
                (unsigned char &)*dst = val & 0xFF;
                 val >>= 8;
            }
        } else {
            char *dst = samplePtr - 1 + var_samplesize;
            for (int i = var_samplesize - 1; i >= 0; --i, --dst) {
                (unsigned char &)*dst = val & 0xFF;
                val >>= 8;
            }
        }
        samplePtr += var_samplesize;
    }
};


template<typename srcT, typename storageT, int c_bits, unsigned int c_dstEndianness, bool c_was_scaled>
inline void writeSample(srcT v, char *&samplePtr, int var_bits = 0, int var_samplesize = 0)
{
    storageT  storageVal = convertSample<srcT, storageT, BITS(srcT), c_bits, c_was_scaled>()(v, BITS(srcT), var_bits);
    writeSampleRaw<storageT, c_bits, c_dstEndianness>()(storageVal, samplePtr, var_samplesize);
}




#define  resolveVariableBits(function,destT,bits,samplesize,signedness,endianness,do_scale,...)                \
        switch(bits) {                                                                                        \
            case 8:                                                                                           \
                function<destT, signedness char,       8, endianness,do_scale>(__VA_ARGS__);                   \
                break;                                                                                        \
            case 16:                                                                                          \
                function<destT, signedness short,     16, endianness,do_scale>(__VA_ARGS__);                   \
                break;                                                                                        \
            case 24:                                                                                          \
                function<destT, signedness int,       24, endianness,do_scale>(__VA_ARGS__);                   \
                break;                                                                                        \
            case 32:                                                                                          \
                function<destT, signedness int,       32, endianness,do_scale>(__VA_ARGS__);                   \
                break;                                                                                        \
            case 64:                                                                                          \
                function<destT, signedness long long, 64, endianness,do_scale>(__VA_ARGS__);                   \
                break;                                                                                        \
            default:                                                                                          \
                function<destT, signedness long long,  0, endianness,do_scale>(__VA_ARGS__, bits, samplesize); \
                break;                                                                                        \
        }

#define  resolveVariableSignednessAndBits(function,destT,bits,samplesize,is_signed,endianness,do_scale,...)  \
    if (!is_signed) {                                                                                       \
        resolveVariableBits(function,destT,bits,samplesize,unsigned,endianness,do_scale,__VA_ARGS__)         \
    } else {                                                                                                \
        resolveVariableBits(function,destT,bits,samplesize,signed,  endianness,do_scale,__VA_ARGS__)         \
    }

#define  resolveVariableEndiannessSignednessAndBits(function,destT,bits,samplesize,is_signed,endianness,do_scale,...)     \
    if (endianness == LITTLE_ENDIAN) {                                                                                   \
        resolveVariableSignednessAndBits(function,destT,bits,samplesize,is_signed,LITTLE_ENDIAN,do_scale,__VA_ARGS__)   \
    } else {                                                                                                            \
        resolveVariableSignednessAndBits(function,destT,bits,samplesize,is_signed,BIG_ENDIAN,   do_scale,__VA_ARGS__)   \
    }



template<typename dstT, typename storageT, int c_bits, unsigned int c_srcEndianness, bool c_do_scale>
inline void convertSamplesToTypeInterleaved(const char *src, dstT *dst, size_t n_samples, int var_bits = 0, int var_samplesize = 0)
{
    for (size_t i = 0; i < n_samples; ++i) {
        dst[i] = readSample<dstT, storageT, c_bits, c_srcEndianness, c_do_scale>(src, var_bits, var_samplesize);
    }
}


template<typename dstT, typename storageT, int c_bits, unsigned int c_srcEndianness, bool c_do_scale>
inline void convertSamplesToTypeNonInterleaved(const char *src, dstT **dst, size_t n_channels, size_t n_frames, int var_bits = 0, int var_samplesize = 0)
{
    for (size_t frm = 0; frm < n_frames; ++frm) {
        for (size_t ch = 0; ch < n_channels; ++ch) {
            dst[ch][frm] = readSample<dstT, storageT, c_bits, c_srcEndianness, c_do_scale>(src, var_bits, var_samplesize);
        }
    }
}


template<typename srcT, typename storageT, int c_bits, unsigned int c_dstEndianness, bool c_was_scaled>
inline void convertTypeInterleavedToSamples(const srcT *src, char *dst, size_t n_samples, int var_bits = 0, int var_samplesize = 0)
{
    for (size_t i = 0; i < n_samples; ++i) {
        writeSample<srcT, storageT, c_bits, c_dstEndianness, c_was_scaled>(src[i], dst, var_bits, var_samplesize);
    }
}


template<typename srcT, typename storageT, int c_bits, unsigned int c_dstEndianness, bool c_was_scaled>
inline void convertTypeNonInterleavedToSamples(const srcT **src, char *dst, size_t n_channels, size_t n_frames, int var_bits = 0, int var_samplesize = 0)
{
    for (size_t frm = 0; frm < n_frames; ++frm) {
        for (size_t ch = 0; ch < n_channels; ++ch) {
            writeSample<srcT, storageT, c_bits, c_dstEndianness, c_was_scaled>(src[ch][frm], dst, var_bits, var_samplesize);
        }
    }
}


void SoundFormat::convertSamplesToFloatInterleaved(const char *src, float *dst, size_t n_frames) const
{
    resolveVariableEndiannessSignednessAndBits(::convertSamplesToTypeInterleaved, float, m_SampleBits, sampleSize(), m_IsSigned, m_Endianness, true, src, dst, n_frames * m_Channels);
}


void SoundFormat::convertFloatInterleavedToSamples(const float *src, char *dst, size_t n_frames) const
{
    resolveVariableEndiannessSignednessAndBits(::convertTypeInterleavedToSamples, float, m_SampleBits, sampleSize(), m_IsSigned, m_Endianness, true, src, dst, n_frames * m_Channels);
}

void SoundFormat::convertSamplesToFloatNonInterleaved(const char *src, float **dst, size_t n_frames) const
{
    resolveVariableEndiannessSignednessAndBits(::convertSamplesToTypeNonInterleaved, float, m_SampleBits, sampleSize(), m_IsSigned, m_Endianness, true, src, dst, m_Channels, n_frames);
}


void SoundFormat::convertFloatNonInterleavedToSamples(const float **src, char *dst, size_t n_frames) const
{
    resolveVariableEndiannessSignednessAndBits(::convertTypeNonInterleavedToSamples, float, m_SampleBits, sampleSize(), m_IsSigned, m_Endianness, true, src, dst, m_Channels, n_frames);
}



template<typename storageT>
void convertNonInterleavedToInterleaved(const char *_src, char *_dst, size_t n_frames, size_t n_channels)
{
    const storageT *src = (const storageT*)_src;
    storageT       *dst = (storageT*)      _dst;
    for(size_t iChannel = 0; iChannel < n_channels; ++iChannel) {
        storageT *curDst = dst + iChannel;
        for (size_t iSample = 0; iSample < n_frames; ++iSample) {
            *curDst = *(src++);
            curDst += n_channels;
        }
    }
}


void SoundFormat::convertNonInterleavedToInterleaved(const char *src, char *dst, size_t n_frames) const
{
    switch(sampleSize()) {
        case 1:
            ::convertNonInterleavedToInterleaved<unsigned char>     (src, dst, n_frames, m_Channels);
            break;
        case 2:
            ::convertNonInterleavedToInterleaved<unsigned short>    (src, dst, n_frames, m_Channels);
            break;
        case 4:
            ::convertNonInterleavedToInterleaved<unsigned int>      (src, dst, n_frames, m_Channels);
            break;
        case 8:
            ::convertNonInterleavedToInterleaved<unsigned long long>(src, dst, n_frames, m_Channels);
            break;
        default:
            // FIXME: do sth here
            break;
    }
}





template<typename tmpT, typename storageT, int c_bits, unsigned int c_endianness, bool do_scale_dummy>
inline void scaleSamples(char *_src, float scale, size_t n, int var_bits = 0, int var_sampleSize = 0)
{
    const char   *src = _src;
    char         *dst = _src;
    for (size_t i = 0; i < n; ++i) {
        tmpT tmp = readSample<tmpT, storageT, c_bits, c_endianness, false>(src, var_bits, var_sampleSize);
        tmp *= scale;
        writeSample<tmpT, storageT, c_bits, c_endianness, false>(tmp, dst, var_bits, var_sampleSize);
    }
}


void SoundFormat::scaleSamples(char *_src, float scale, size_t n_frames) const
{
    resolveVariableEndiannessSignednessAndBits(::scaleSamples, double, m_SampleBits, sampleSize(), m_IsSigned, m_Endianness, false, _src, scale, n_frames * m_Channels);
}



template<typename dstT, typename storageT, int c_bits, unsigned int c_endianness, bool do_scale>
inline void minMaxAvgMagnitudePerChannel(const char *src, size_t channels, size_t n_frames, dstT *vmin, dstT *vmax, dstT *vavg, int var_bits = 0, int var_sampleSize = 0)
{
    size_t n_samples = n_frames * channels;
    for (size_t ch = 0; ch < channels; ++ch) {
        dstT     v    = readSample<dstT, storageT, c_bits, c_endianness, do_scale>(src, var_bits, var_sampleSize);
        dstT     absV = v < 0 ? -v : v;
        vmin[ch] = absV;
        vmax[ch] = absV;
        vavg[ch] = absV;
    }
    size_t ch = 0;
    for (size_t i = channels; i < n_samples; ++i) {
        dstT     v    = readSample<dstT, storageT, c_bits, c_endianness, do_scale>(src, var_bits, var_sampleSize);
        dstT     absV = v < 0 ? -v : v;
        vmin[ch]  = std::min(vmin[ch], absV);
        vmax[ch]  = std::max(vmax[ch], absV);
        vavg[ch] += absV;
        if (++ch >= channels) {
            ch -= channels;
        }
    }
    for (size_t ch = 0; ch < channels; ++ch) {
        vavg[ch] /= n_frames;
    }
}


void SoundFormat::minMaxAvgMagnitudePerChannel(const char *src, size_t n_frames, double *vmin, double *vmax, double *vavg) const
{
    resolveVariableEndiannessSignednessAndBits(::minMaxAvgMagnitudePerChannel, double, m_SampleBits, sampleSize(), m_IsSigned, m_Endianness, false, src, m_Channels, n_frames, vmin, vmax, vavg);
}
