/***************************************************************************
                          alarm.cpp  -  description
                             -------------------
    begin                : Mon Feb 4 2002
    copyright            : (C) 2002 by Martin Witte / Frank Schwanz
    email                : witte@kawo1.rwth-aachen.de / schwanz@fh-brandenburg.de
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

Alarm::Alarm(const QDateTime &time, bool daily, bool enabled)
	: m_time         (time),
	  m_daily        (daily),
	  m_enabled      (enabled),
	  m_stationID    (""),
      m_volumePreset (-1),
      m_type         (Start),
      m_record       (false)
{	  
}


Alarm::Alarm ()
	: m_time        (QDateTime (QDate(1800, 1,1), QTime(0,0,0))),
	  m_daily       (false),
	  m_enabled     (false),
	  m_stationID   (""),
	  m_volumePreset(-1),
      m_type         (Start),
      m_record       (false)
{
}


Alarm::Alarm (const Alarm &a)
   : m_time        (a.m_time),
     m_daily       (a.m_daily),
     m_enabled     (a.m_enabled),
     m_stationID   (a.m_stationID),
     m_volumePreset(a.m_volumePreset),
     m_type        (a.m_type),
     m_record      (a.m_record)
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
	}
	return m_enabled || ignoreEnable ? alarm : QDateTime();
}


