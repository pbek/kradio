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
#include "radiostation.h"

Alarm::Alarm(QDateTime time, bool daily, bool enabled)
	: m_time         (time),
	  m_daily        (daily),
	  m_enabled      (enabled),
	  m_station      (undefinedRadioStation.copy()),
      m_volumePreset (-1)
{	  
}


Alarm::Alarm ()
	: m_time        (QDateTime (QDate(1800, 1,1), QTime(0,0,0))),
	  m_daily       (false),
	  m_enabled     (false),
	  m_station     (undefinedRadioStation.copy()),
	  m_volumePreset(-1)
{
}


Alarm::Alarm (const Alarm &a)
   : m_time        (a.m_time),
     m_daily       (a.m_daily),
     m_enabled     (a.m_enabled),
     m_station     (a.m_station ? a.m_station->copy() : undefinedRadioStation.copy()),
     m_volumePreset(a.m_volumePreset)
{
}


Alarm::~Alarm()
{
	delete m_station;
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


void Alarm::setStation(const RadioStation &rs)
{
	if (m_station)
		delete m_station;
	m_station = rs.copy();
}

const RadioStation &Alarm::getStation() const
{
	return m_station ? (const RadioStation&)*m_station : (const RadioStation&)undefinedRadioStation;
}
