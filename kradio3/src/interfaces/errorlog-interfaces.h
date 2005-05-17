/***************************************************************************
                          errorlog-interfaces.h  -  description
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

#ifndef KRADIO_ERRORLOG_INTERFACES_H
#define KRADIO_ERRORLOG_INTERFACES_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "interfaces.h"

INTERFACE(IErrorLog, IErrorLogClient)
{
public :
    IF_CON_DESTRUCTOR(IErrorLog, -1)

RECEIVERS:
    IF_RECEIVER(    logError  (const QString &)         )
    IF_RECEIVER(    logWarning(const QString &)         )
    IF_RECEIVER(    logInfo   (const QString &)         )
    IF_RECEIVER(    logDebug  (const QString &)         )
};


INTERFACE(IErrorLogClient, IErrorLog)
{
public :
    IF_CON_DESTRUCTOR(IErrorLogClient, -1)

public:
    IF_SENDER  (    sendLogError  (const QString &)     )
    IF_SENDER  (    sendLogWarning(const QString &)     )
    IF_SENDER  (    sendLogInfo   (const QString &)     )
    IF_SENDER  (    sendLogDebug  (const QString &)     )

    void logError  (const QString &s) const { sendLogError(s);   }
    void logWarning(const QString &s) const { sendLogWarning(s); }
    void logInfo   (const QString &s) const { sendLogInfo(s);    }
    void logDebug  (const QString &s) const { sendLogDebug(s);   }
};


#endif
