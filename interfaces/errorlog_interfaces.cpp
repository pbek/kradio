/***************************************************************************
                          errorlog_interfaces.cpp  -  description
                             -------------------
    begin                : Sa Sep 13 2003
    copyright            : (C) 2003 by Martin Witte
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

#include "errorlog_interfaces.h"

#include <kdebug.h>
#include <klocalizedstring.h>
#include <QtCore/QDateTime>

IErrorLog *staticLogger = NULL;

IErrorLog::IErrorLog()
 : BaseClass(-1)
{
    if (!staticLogger)
        staticLogger = this;
}


IErrorLog::~IErrorLog()
{
    if (staticLogger == this)
        staticLogger = NULL;
}


int IErrorLogClient::sendLogError(const QString &s) const
{
    kDebug() << i18n("%1 Error: %2", QDateTime::currentDateTime().toString(Qt::ISODate), s);
    IF_SEND_MESSAGE(logError(s));
}


int IErrorLogClient::sendLogWarning(const QString &s) const
{
    kDebug() << i18n("%1 Warning: %2", QDateTime::currentDateTime().toString(Qt::ISODate), s);
    IF_SEND_MESSAGE(logWarning(s));
}


int IErrorLogClient::sendLogInfo(const QString &s) const
{
    kDebug() << i18n("%1 Information: %2", QDateTime::currentDateTime().toString(Qt::ISODate), s);
    IF_SEND_MESSAGE(logInfo(s));
}


int IErrorLogClient::sendLogDebug(const QString &s) const
{
    kDebug() << i18n("%1 Debug: %2", QDateTime::currentDateTime().toString(Qt::ISODate), s);
    IF_SEND_MESSAGE(logDebug(s));
}

void IErrorLogClient::staticLogError  (const QString &s)
{
    kDebug() << i18n("%1 Error: %2", QDateTime::currentDateTime().toString(Qt::ISODate), s);
    if (staticLogger)
        staticLogger->logError(s);
}

void IErrorLogClient::staticLogWarning(const QString &s)
{
    kDebug() << i18n("%1 Warning: %2", QDateTime::currentDateTime().toString(Qt::ISODate), s);
    if (staticLogger)
        staticLogger->logWarning(s);
}

void IErrorLogClient::staticLogInfo   (const QString &s)
{
    kDebug() << i18n("%1 Information: %2", QDateTime::currentDateTime().toString(Qt::ISODate), s);
    if (staticLogger)
        staticLogger->logInfo(s);
}

void IErrorLogClient::staticLogDebug  (const QString &s)
{
    kDebug() << i18n("%1 Debug: %2", QDateTime::currentDateTime().toString(Qt::ISODate), s);
    if (staticLogger)
        staticLogger->logDebug(s);
}

