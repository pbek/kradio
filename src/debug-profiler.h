/***************************************************************************
                          debug-profiler.h  -  description
                             -------------------
    begin                : Sat May 28 2005
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

#ifndef KRADIO_DEBUG_PROFILER_H
#define KRADIO_DEBUG_PROFILER_H

#include <QByteArray>
#include <QMap>

#ifdef KRADIO_ENABLE_PROFILERS

#if (defined __i386__) || (defined __x86_64__)
static __inline__ unsigned long long int rdtsc()
{
    unsigned int a, d;
    asm volatile("rdtsc" : "=a" (a), "=d" (d));
    return ((unsigned long long)a) | (((unsigned long long)d) << 32);
}
#else
static __inline__ unsigned long long int rdtsc()
{
    return 0UL;
}
#endif

class KRADIO5_EXPORT Profiler
{
public:
    Profiler();
    virtual ~Profiler();

    void startProfile(const QByteArray &descr);
    void stopProfile (const QByteArray &descr);

    void printData();

protected:

    virtual long long getCounter() const = 0;

    void stopInternalCounter();
    void startInternalCounter();

    long long m_internalCounter;
    long long m_tmpStartVal;

    struct profile_data
    {
        profile_data(long long start = 0) :
            startCounter(start), accumulatedCounter(0), callCounter(0),
            minCounter(0x7FFFFFFFFFFFFFFFll), maxCounter(0) {}
        long long startCounter;
        long long accumulatedCounter;
        long long callCounter;
        long long minCounter;
        long long maxCounter;
    };

    QMap<QByteArray, profile_data> m_ProfileData;
};


class KRADIO5_EXPORT TimeProfiler : public Profiler
{
protected:
    long long getCounter() const { return rdtsc(); }
};


class KRADIO5_EXPORT MemProfiler : public Profiler
{
protected:
    long long getCounter() const;
};


extern KRADIO5_EXPORT TimeProfiler global_time_profiler;
extern KRADIO5_EXPORT MemProfiler  global_mem_profiler;



class KRADIO5_EXPORT BlockProfiler
{
public:
    BlockProfiler(const QByteArray &descr);
    ~BlockProfiler();

    void stop();

protected:
    QByteArray m_Description;
};

#else

class BlockProfiler
{
public:
    inline BlockProfiler(const QByteArray &) {}
    inline ~BlockProfiler() {}

    inline void stop() {}
};

#endif

#endif
