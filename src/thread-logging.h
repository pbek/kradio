/***************************************************************************
                          thread-logging.h  -  description
                             -------------------
    begin                : Sun Feb 12 2012
    copyright            : (C) 2012 by Martin Witte
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

#ifndef __THREAD_LOGGING_H__
#define __THREAD_LOGGING_H__


#include <QMutex>
#include <QStringList>
#include <QVector>
#include <errorlog_interfaces.h>

// logging in thread
class KDE_EXPORT ThreadLogging
{
public:

    enum  LoggingClass { LogInfo = 0, LogWarning, LogDebug, LogError, LogLast /* keep it as last! */ };

    ThreadLogging();

    bool                 hasLog (LoggingClass cls)                       const;
    QStringList          getLogs(LoggingClass cls, bool resetLog = true);
    QVector<QStringList> getAllLogs(bool resetLog = true);

    QList<LoggingClass>  getLogClasses() const;

    void                 log    (LoggingClass cls, const QString &logString);

private:
    mutable QMutex                    m_accessLock;
    QVector<QStringList>              logs;
};



// receiving logs outside the thread
class KDE_EXPORT ThreadLoggingClient
{
public:
    ThreadLoggingClient();
    virtual ~ThreadLoggingClient();

    // returns false if there logs of class LogError are available
    bool   checkLogs(ThreadLogging *threadLogger, const QString &logPrefix, bool resetLogs = true);


protected:
    virtual IErrorLogClient *getErrorLogClient() = 0;


protected:
    void sendLogs(const QString &logPrefix, IErrorLogClient::logFunction_t logFunc, const QStringList &messages);
};


#endif

