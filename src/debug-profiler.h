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

#include <QString>
#include <QMap>
#include <kdemacros.h>

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

class KDE_EXPORT Profiler
{
public:
    Profiler();
    virtual ~Profiler();

    void startProfile(const QString &descr);
    void stopProfile (const QString &descr);

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

    QMap<QString, profile_data> m_ProfileData;
};


class KDE_EXPORT TimeProfiler : public Profiler
{
protected:
    long long getCounter() const { return rdtsc(); }
};


class KDE_EXPORT MemProfiler : public Profiler
{
protected:
    long long getCounter() const;
};


extern KDE_EXPORT TimeProfiler global_time_profiler;
extern KDE_EXPORT MemProfiler  global_mem_profiler;



class KDE_EXPORT BlockProfiler
{
public:
    BlockProfiler(const QString &descr);
    ~BlockProfiler();

    void stop();

protected:
    QString m_Description;
};



#endif
