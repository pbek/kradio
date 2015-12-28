/***************************************************************************
                          thread-logging.cpp  -  description
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

#include "errorlog_interfaces.h"
#include "thread-logging.h"

ThreadLogging::ThreadLogging()
    : logs(LogLast)
{
    // nothing to do
}


bool ThreadLogging::hasLog (LoggingClass cls) const
{
    QMutexLocker  locker(&m_accessLock);
    return !logs.at(cls).isEmpty();
}


QList<ThreadLogging::LoggingClass> ThreadLogging::getLogClasses() const
{
    QList<ThreadLogging::LoggingClass>  keys;
    QMutexLocker                        locker(&m_accessLock);
    for (int i = 0; i < LogLast; ++i) {
        if (!logs.at(i).isEmpty()) {
            keys.append(static_cast<ThreadLogging::LoggingClass>(i));
        }
    }
    return keys;
}


QStringList ThreadLogging::getLogs(LoggingClass cls, bool resetLog)
{
    QStringList   deepCopyRetVal;
    QMutexLocker  locker(&m_accessLock);
    if (!logs.at(cls).isEmpty()) {
        deepCopyRetVal = logs[cls];
        if (resetLog) {
            logs[cls].clear();
        }
    }
    return deepCopyRetVal;
}


QVector<QStringList> ThreadLogging::getAllLogs(bool resetLog)
{
    QMutexLocker  locker(&m_accessLock);
    const QVector<QStringList> ret = logs;
    if (resetLog) {
        // reset the whole logs vector at once
        logs = QVector<QStringList>(LogLast);
    }
    return ret;
}


void ThreadLogging::log(LoggingClass cls, const QString &logString)
{
    QMutexLocker  locker(&m_accessLock);
    logs[cls].append(logString);
}



ThreadLoggingClient::ThreadLoggingClient()
{
    // nothing to do
}


ThreadLoggingClient::~ThreadLoggingClient()
{
    // nothing to do
}


// returns false if an error occurred
bool ThreadLoggingClient::checkLogs(ThreadLogging *threadLogger, const QString &logPrefix, bool resetLogs)
{
    if (!threadLogger) {
        return true;
    }

    const QVector<QStringList> logs = threadLogger->getAllLogs(resetLogs);

    sendLogs(logPrefix, &IErrorLogClient::logInfo, logs.at(ThreadLogging::LogInfo));
    sendLogs(logPrefix, &IErrorLogClient::logWarning, logs.at(ThreadLogging::LogWarning));
    sendLogs(logPrefix, &IErrorLogClient::logDebug, logs.at(ThreadLogging::LogDebug));
    sendLogs(logPrefix, &IErrorLogClient::logError, logs.at(ThreadLogging::LogError));

    return logs.at(ThreadLogging::LogError).isEmpty();
}


void ThreadLoggingClient::sendLogs(const QString &logPrefix, IErrorLogClient::logFunction_t logFunc, const QStringList &messages)
{
    if (messages.isEmpty()) {
        return;
    }

    IErrorLogClient *log      = getErrorLogClient();

    foreach (const QString &msg, messages) {
        const QStringList lines = msg.split(QLatin1Char('\n'));
        foreach (const QString &line, lines) {
            (log->*logFunc)(logPrefix + line);
        }
    }
}


