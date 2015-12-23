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

#include "debug-profiler.h"

#include <QStringList>

#include <sys/resource.h>
#include <stdio.h>

TimeProfiler global_time_profiler;
MemProfiler  global_mem_profiler;

Profiler::Profiler()
{
}


Profiler::~Profiler()
{
    m_tmpStartVal = 0;
}

void Profiler::stopInternalCounter()
{
    long long counter = getCounter();
    long long diff = counter - m_tmpStartVal;
    m_internalCounter += diff;
}

void Profiler::startInternalCounter() {
    m_tmpStartVal = getCounter();
}

void  Profiler::startProfile(const QString &descr)
{
    stopInternalCounter();

    if (m_ProfileData.contains(descr)) {
        profile_data &d = m_ProfileData[descr];
        d.startCounter = m_internalCounter;
    } else {
        m_ProfileData.insert(descr, profile_data(m_internalCounter));
    }

    startInternalCounter();
}


void Profiler::stopProfile (const QString &descr)
{
    stopInternalCounter();

    if (!descr.isNull() && m_ProfileData.contains(descr)) {
        profile_data &d = m_ProfileData[descr];
        long long diff = m_internalCounter - d.startCounter;
        d.accumulatedCounter += diff;
        if (d.maxCounter < diff)
            d.maxCounter = diff;
        if (d.minCounter > diff)
            d.minCounter = diff;
        d.callCounter++;
    }

    startInternalCounter();
}


void Profiler::printData ()
{
    stopInternalCounter();

    QStringList keys=m_ProfileData.keys();
    keys.sort();
    QList<QString>::iterator it  = keys.begin();
    QList<QString>::iterator end = keys.end();
    for (; it != end; ++it) {
        int l = (*it).length();
        l = (((l-1) / 25) + 1) * 25;
        if (l < 50) l = 50;
        const profile_data &d = m_ProfileData[*it];
        printf("%-*s: total: %3.8f (%9lli)  avg: %3.8f  min: %3.8f  max: %3.8f\n",
               l,
               qPrintable(*it),
               (double)d.accumulatedCounter / 1.666e9,
               d.callCounter,
               (double)d.accumulatedCounter / (double)d.callCounter / 1.666e9,
               (double)d.minCounter / 1.666e9,
               (double)d.maxCounter / 1.666e9);
    }

    startInternalCounter();
}


long long MemProfiler::getCounter() const
{
    struct rusage usg;
    if (getrusage(RUSAGE_SELF, &usg) == 0) {
        return usg.ru_idrss + usg.ru_isrss;
    } else {
        return 0;
    }
}


BlockProfiler::BlockProfiler(const QString &descr)
    : m_Description(descr)
{
    global_mem_profiler.startProfile(m_Description);
    global_time_profiler.startProfile(m_Description);
}

BlockProfiler::~BlockProfiler()
{
    global_time_profiler.stopProfile(m_Description);
    global_mem_profiler.stopProfile(m_Description);
}

void BlockProfiler::stop()
{
    global_time_profiler.stopProfile(m_Description);
    global_mem_profiler.stopProfile(m_Description);
    m_Description = QString::null;
}

