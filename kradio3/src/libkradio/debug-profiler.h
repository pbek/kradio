/***************************************************************************
                          debug-profiler.h  -  description
                             -------------------
    begin                : Sat May 28 2005
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

#ifndef KRADIO_DEBUG_PROFILER_H
#define KRADIO_DEBUG_PROFILER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qstring.h>
#include <qmap.h>

static __inline__ unsigned long long int rdtsc()
{
    unsigned long long int x;
    __asm__ volatile (".byte 0x0f, 0x31" : "=A"(x));
    return x;
}

class Profiler
{
public:
    Profiler();
    ~Profiler();

    void startProfile(const QString &descr);
    void stopProfile (const QString &descr);

    void printData();

protected:

    inline void stopInternalCounter()  { m_internalCounter += rdtsc() - m_tmpStartVal; }
    inline void startInternalCounter() { m_tmpStartVal = rdtsc(); }

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


extern Profiler global_profiler;



class BlockProfiler
{
public:
    BlockProfiler(const QString &descr)
        : m_Description(descr) { global_profiler.startProfile(m_Description); }
    ~BlockProfiler()
        { global_profiler.stopProfile(m_Description); }

    void stop() { global_profiler.stopProfile(m_Description); m_Description = QString::null; }

protected:
    QString m_Description;
};



#endif
