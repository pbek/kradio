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

#include <qobject.h>
#include <qdatetime.h>
#include <list>

/**
  *@author Martin Witte / Frank Schwanz
  */

class Alarm : public QObject  {
Q_OBJECT

protected:
	QDateTime	time;
	bool		enabled;
	bool		daily;
	bool		done;

public:
	Alarm(QObject *parent, QDateTime time, bool daily, bool enabled);
	Alarm(QObject *parent);
	virtual ~Alarm();
	
	bool		isEnabled() const { return enabled;}
	bool 		isDaily() const { return daily; }
	QDateTime   nextAlarm () const;
	QDateTime	alarmTime () const { return time; }
	
	void	setEnabled (bool enable = true) { enabled = enable; }
	void	setDaily (bool d = true) { daily = d; }
	void	setDate (const QDate &d) { time.setDate(d); }
	void	setTime (const QTime &d) { time.setTime(d); }
	
public slots:
	void poll ();

signals:
	void alarm (Alarm *);
};


typedef list<Alarm *>				AlarmList;
typedef AlarmList::iterator			iAlarmList;
typedef AlarmList::const_iterator	ciAlarmList;

#endif
