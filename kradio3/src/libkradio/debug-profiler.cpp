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

#include "debug-profiler.h"

#include <qstringlist.h>

Profiler global_profiler;

Profiler::Profiler()
{
    startInternalCounter();
}


Profiler::~Profiler()
{
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
    QValueListIterator<QString> it  = keys.begin();
    QValueListIterator<QString> end = keys.end();
    for (; it != end; ++it) {
        int l = (*it).length();
        l = (((l-1) / 25) + 1) * 25;
        if (l < 50) l = 50;
        const profile_data &d = m_ProfileData[*it];
        printf(("%-"+QString::number(l)+"s: total: %3.8f (%9lli)  avg: %3.8f  min: %3.8f  max: %3.8f\n").ascii(),
               (*it).ascii(),
               (double)d.accumulatedCounter / 1.666e9,
               d.callCounter,
               (double)d.accumulatedCounter / (double)d.callCounter / 1.666e9,
               (double)d.minCounter / 1.666e9,
               (double)d.maxCounter / 1.666e9);
    }

    startInternalCounter();
}

