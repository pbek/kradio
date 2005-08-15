/***************************************************************************
                          errorlog-interfaces.cpp  -  description
                             -------------------
    begin                : Sa Sep 13 2003
    copyright            : (C) 2003 by Martin Witte
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

#include "errorlog-interfaces.h"

#include <kdebug.h>
#include <qdatetime.h>

IErrorLog *staticLogger = NULL;

IErrorLog::IErrorLog()
 : BaseClass(-1)
{
    if (!staticLogger)
        staticLogger = this;
}


int IErrorLogClient::sendLogError(const QString &s) const
{
    kdDebug() << QDateTime::currentDateTime().toString(Qt::ISODate)
              << " Error: "
              << s << endl;
    IF_SEND_MESSAGE(logError(s));
}


int IErrorLogClient::sendLogWarning(const QString &s) const
{
    kdDebug() << QDateTime::currentDateTime().toString(Qt::ISODate)
              << " Warning: "
              << s << endl;
    IF_SEND_MESSAGE(logWarning(s));
}


int IErrorLogClient::sendLogInfo(const QString &s) const
{
    kdDebug() << QDateTime::currentDateTime().toString(Qt::ISODate)
              << " Information: "
              << s << endl;
    IF_SEND_MESSAGE(logInfo(s));
}


int IErrorLogClient::sendLogDebug(const QString &s) const
{
    kdDebug() << QDateTime::currentDateTime().toString(Qt::ISODate)
              << " Debug: "
              << s << endl;
    IF_SEND_MESSAGE(logDebug(s));
}

void IErrorLogClient::staticLogError  (const QString &s)
{
    kdDebug() << QDateTime::currentDateTime().toString(Qt::ISODate)
              << " Error: "
              << s << endl;
    if (staticLogger)
        staticLogger->logError(s);
}

void IErrorLogClient::staticLogWarning(const QString &s)
{
    kdDebug() << QDateTime::currentDateTime().toString(Qt::ISODate)
              << " Warning: "
              << s << endl;
    if (staticLogger)
        staticLogger->logWarning(s);
}

void IErrorLogClient::staticLogInfo   (const QString &s)
{
    kdDebug() << QDateTime::currentDateTime().toString(Qt::ISODate)
              << " Information: "
              << s << endl;
    if (staticLogger)
        staticLogger->logInfo(s);
}

void IErrorLogClient::staticLogDebug  (const QString &s)
{
    kdDebug() << QDateTime::currentDateTime().toString(Qt::ISODate)
              << " Debug: "
              << s << endl;
    if (staticLogger)
        staticLogger->logDebug(s);
}

