/***************************************************************************
                          alarm.cpp  -  description
                             -------------------
    begin                : Mon Feb 4 2002
    copyright            : (C) 2002 by Martin Witte / Frank Schwanz
    email                : emw-kradio@nocabal.de / schwanz@fh-brandenburg.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "alarm.h"

int Alarm::m_LastID = 0;

Alarm::Alarm(const QDateTime &time, bool daily, bool enabled)
    : m_time              (time),
      m_daily             (daily),
      m_weekdayMask       (0x7F),
      m_enabled           (enabled),
      m_stationID         (QString::null),
      m_volumePreset      (-1),
      m_type              (StartPlaying),
      m_recordingTemplate ("kradio-recording-%s-%Y.%m.%d-%H.%M.%S"),
      m_ID                (++m_LastID)
{
}


Alarm::Alarm ()
    : m_time              (QDateTime (QDate(1800, 1,1), QTime(0,0,0))),
      m_daily             (false),
      m_weekdayMask       (0x7F),
      m_enabled           (false),
      m_stationID         (QString::null),
      m_volumePreset      (-1),
      m_type              (StartPlaying),
      m_recordingTemplate ("kradio-recording-%s-%Y.%m.%d-%H.%M.%S"),
      m_ID                (++m_LastID)
{
}


Alarm::Alarm (const Alarm &a)
   : m_time              (a.m_time),
     m_daily             (a.m_daily),
     m_weekdayMask       (a.m_weekdayMask),
     m_enabled           (a.m_enabled),
     m_stationID         (a.m_stationID),
     m_volumePreset      (a.m_volumePreset),
     m_type              (a.m_type),
     m_recordingTemplate (a.m_recordingTemplate),
     m_ID                (a.m_ID)
{
}


Alarm::~Alarm()
{
}


QDateTime Alarm::nextAlarm(bool ignoreEnable) const
{
    QDateTime now = QDateTime::currentDateTime(),
              alarm = m_time;
    if (m_daily) {
        alarm.setDate (now.date());
        if (alarm <= now)
            alarm = alarm.addDays(1);
        while (m_weekdayMask &&
               !(m_weekdayMask & (1 << (alarm.date().dayOfWeek()-1))))
        {
            alarm = alarm.addDays(1);
        }
    }
    return (m_enabled || ignoreEnable) && (!m_daily || m_weekdayMask) ? alarm : QDateTime();
}


