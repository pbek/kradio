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

#include <QtCore/QMutex>
#include <QtCore/QString>
#include <QtCore/QStringList>

ThreadLogging::ThreadLogging()
{
    // nothing to do
}


bool ThreadLogging::hasLog (LoggingClass cls) const
{
    QMutexLocker  locker(&m_accessLock);
    return logs.contains(cls) && logs[cls].size() > 0;
}


QList<ThreadLogging::LoggingClass> ThreadLogging::getLogClasses() const
{
    QList<ThreadLogging::LoggingClass>  keys;
    {
        QMutexLocker                        locker(&m_accessLock);
        keys = logs.keys();
    }
    return keys;
}


QStringList ThreadLogging::getLogs(LoggingClass cls, bool resetLog)
{
    QStringList   deepCopyRetVal;
    {
        QMutexLocker  locker(&m_accessLock);
        if (logs.contains(cls) && logs[cls].size()) {
            foreach (QString s, logs[cls]) {
                deepCopyRetVal.append(QString(s.constData(), s.length()));
            }
            if (resetLog) {
                logs[cls].clear();
            }
        }
    }
    return deepCopyRetVal;
}


void ThreadLogging::log(LoggingClass cls, QString logString)
{
    QMutexLocker  locker(&m_accessLock);
    if (!logs.contains(cls)) {
        logs[cls] = QStringList();
    }
    logs[cls].append(QString(logString.constData(), logString.length())); // deep copy
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
bool ThreadLoggingClient::checkLogs(ThreadLogging *threadLogger, QString logPrefix, bool resetLogs)
{
    bool hasError = false;
               checkLogs(threadLogger, ThreadLogging::LogInfo,    logPrefix, &IErrorLogClient::logInfo,    resetLogs);
               checkLogs(threadLogger, ThreadLogging::LogWarning, logPrefix, &IErrorLogClient::logWarning, resetLogs);
               checkLogs(threadLogger, ThreadLogging::LogDebug,   logPrefix, &IErrorLogClient::logDebug,   resetLogs);
    hasError = checkLogs(threadLogger, ThreadLogging::LogError,   logPrefix, &IErrorLogClient::logError,   resetLogs);
    return !hasError;
}


bool ThreadLoggingClient::checkLogs(ThreadLogging *threadLogger, ThreadLogging::LoggingClass cls, QString logPrefix, IErrorLogClient::logFunction_t logFunc, bool resetLog)
{
    IErrorLogClient *log      = getErrorLogClient();

    bool hadMessages = false;
    if (threadLogger) {
        QStringList msgList = threadLogger->getLogs(cls, resetLog);
        foreach(QString msg, msgList) {
            QStringList subList = msg.split("\n");
            foreach (QString s, subList) {
                (log->*logFunc)(logPrefix + s);
            }
        }
        hadMessages = msgList.size() > 0;
    }
    return hadMessages;
}


