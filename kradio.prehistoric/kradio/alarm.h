/***************************************************************************
                          alarm.h  -  description
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

#ifndef ALARM_H
#define ALARM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "utils.h"
#include <qdatetime.h>
#include <vector>

/**
  *@author Martin Witte
  */

class Alarm
{
protected:
	QDateTime	  m_time;
	bool		  m_daily;
	bool		  m_enabled;
	QString       m_stationID;
	float		  m_volumePreset;  // < 0: disabled

public:
	Alarm();
	Alarm(const QDateTime &time, bool daily, bool enabled);
	Alarm(const Alarm &);
	~Alarm();
	
	bool		   isEnabled() const            { return m_enabled;   }
	bool 		   isDaily() const              { return m_daily;     }
	QDateTime	   alarmTime () const           { return m_time;      }
	QDateTime      nextAlarm (bool ignoreEnable = false) const;
	const QString &getStationID () const        { return m_stationID; }
	float	       getVolumePreset () const     { return m_volumePreset; }
	
	void	    setEnabled (bool enable = true) { m_enabled = enable; }
	void	    setDaily (bool d = true)        { m_daily = d;        }
	void	    setDate (const QDate &d)        { m_time.setDate(d);  }
	void	    setTime (const QTime &d)        { m_time.setTime(d);  }
    void        setVolumePreset(float v)        { m_volumePreset = v; }
    void        setStationID(const QString &id) { m_stationID = id;   }
};

typedef vector<Alarm>		        AlarmVector;
typedef AlarmVector::iterator		iAlarmVector;
typedef AlarmVector::const_iterator	ciAlarmVector;



#endif
