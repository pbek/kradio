/***************************************************************************
                          timecontrol.cpp  -  description
                             -------------------
    begin                : Son Jan 12 2003
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

#include "timecontrol.h"
#include "timecontrol-configuration.h"
#include "pluginmanager.h"

#include <kaboutdata.h>
#include "aboutwidget.h"

//const char *AlarmListElement            = "alarmlist";
//const char *AlarmElement                = "alarm";
const char *AlarmDateElement            = "date";
const char *AlarmTimeElement            = "time";
const char *AlarmDailyElement           = "daily";
const char *AlarmEnabledElement         = "enabled";
const char *AlarmStationIDElement       = "stationID";
//const char *AlarmFrequencyElement       = "frequency";
const char *AlarmVolumeElement          = "volume";
const char *AlarmTypeElement            = "type";


TimeControl::TimeControl (const QString &n)
	: PluginBase(n, "TimeControl Plugin"),
	  m_waitingFor(NULL),
	  m_countdownSeconds(0),
	  m_alarmTimer(this),
	  m_countdownTimer(this)
{
	QObject::connect(&m_alarmTimer,     SIGNAL(timeout()), this, SLOT(slotQTimerAlarmTimeout()));
	QObject::connect(&m_countdownTimer, SIGNAL(timeout()), this, SLOT(slotQTimerCountdownTimeout()));
}


TimeControl::~TimeControl ()
{
	m_waitingFor = NULL;
}


bool TimeControl::setAlarms (const AlarmVector &al)
{
	m_waitingFor = NULL;

	m_alarms = al;
	
	slotQTimerAlarmTimeout();

	notifyAlarmsChanged(m_alarms);

	return true;
}


bool TimeControl::setCountdownSeconds(int n)
{
	int old = m_countdownSeconds;
	m_countdownSeconds = n;
	if (old != n)
		notifyCountdownSecondsChanged(n);
	return true;
}


bool TimeControl::startCountdown()
{
	m_countdownEnd = QDateTime::currentDateTime().addSecs(m_countdownSeconds);
	m_countdownTimer.start(m_countdownSeconds * 1000, true);

	notifyCountdownStarted(getCountdownEnd());
	
	return true;
}


bool TimeControl::stopCountdown()
{
	m_countdownTimer.stop();
	m_countdownEnd = QDateTime();

	notifyCountdownStopped();

	return true;
}


QDateTime TimeControl::getNextAlarmTime() const
{
	const Alarm *a = getNextAlarm();
	if (a)
		return a->nextAlarm();
	else
		return QDateTime();
}


const Alarm *TimeControl::getNextAlarm () const
{
	QDateTime now = QDateTime::currentDateTime(),
	          next;

	const Alarm *retval = NULL;

	for (ciAlarmVector i = m_alarms.begin(); i != m_alarms.end(); ++i) {
		QDateTime n = i->nextAlarm();
		if (n.isValid() && n > now && ( ! next.isValid() || n < next)) {
			next = n;
			retval = &(*i);
		}
	}

	QDateTime old = m_nextAlarm_tmp;
	m_nextAlarm_tmp = next;
	if (old != m_nextAlarm_tmp) {
		notifyNextAlarmChanged(retval);
	}

	return retval;
}


QDateTime TimeControl::getCountdownEnd () const
{
	if (m_countdownTimer.isActive())
		return m_countdownEnd;
	else
		return QDateTime();
}


void TimeControl::slotQTimerCountdownTimeout()
{
	stopCountdown();

	notifyCountdownZero();
}


void TimeControl::slotQTimerAlarmTimeout()
{
	if (m_waitingFor) {
		notifyAlarm(*m_waitingFor);
	}

	QDateTime now  = QDateTime::currentDateTime();
	Alarm const *n = getNextAlarm();
    QDateTime na   = getNextAlarmTime();

	m_waitingFor = NULL;
	
	if (na.isValid()) {
	
		int days  = now.daysTo(na);
		int msecs = now.time().msecsTo(na.time());

		if (days > 1) {
			m_alarmTimer.start(24 * 3600 * 1000, true);
			
		} else if (days >= 0) {
		
			if (days > 0)
				msecs += days * 24 * 3600 * 1000;

			if (msecs > 0) {
				m_waitingFor = n;
				m_alarmTimer.start(msecs, true);
			}
		}
	}
}


void    TimeControl::restoreState (KConfig *config)
{
    AlarmVector al;

	config->setGroup(QString("timecontrol-") + name());

	int nAlarms = config->readNumEntry ("nAlarms", 0);
	for (int idx = 1; idx <= nAlarms; ++idx) {
	
		QString num = QString().setNum(idx);
		QDateTime d = config->readDateTimeEntry(AlarmTimeElement       + num);
		bool enable = config->readBoolEntry(AlarmEnabledElement        + num, false);
		bool daily  = config->readBoolEntry(AlarmDailyElement          + num, false);
		float vol   = config->readDoubleNumEntry(AlarmVolumeElement    + num, 1);
        QString sid = config->readEntry(AlarmStationIDElement          + num, QString::null);
        int type    = config->readNumEntry(AlarmTypeElement            + num, 0);

		enable &= d.isValid();

		Alarm a ( d, daily, enable);
		a.setVolumePreset(vol);
		a.setStationID(sid);
		a.setAlarmType((Alarm::AlarmType)type);
		al.push_back(a);
	}

	setAlarms(al);
	setCountdownSeconds(config->readNumEntry("countdownSeconds", 30*60));
}


void    TimeControl::saveState    (KConfig *config) const
{
	config->setGroup(QString("timecontrol-") + name());

	config->writeEntry("nAlarms", m_alarms.size());
	int idx = 1;
	for (ciAlarmVector i = m_alarms.begin(); i != m_alarms.end(); ++i, ++idx) {
		QString num = QString().setNum(idx);
		config->writeEntry (AlarmTimeElement      + num, i->alarmTime());
		config->writeEntry (AlarmEnabledElement   + num, i->isEnabled());
		config->writeEntry (AlarmDailyElement     + num, i->isDaily());
		config->writeEntry (AlarmVolumeElement    + num, i->volumePreset());
		config->writeEntry (AlarmStationIDElement + num, i->stationID());
		config->writeEntry (AlarmTypeElement      + num, i->alarmType());
	}

	config->writeEntry("countdownSeconds",  m_countdownSeconds);
}


ConfigPageInfo TimeControl::createConfigurationPage()
{
    TimeControlConfiguration *conf = new TimeControlConfiguration(NULL);
    connect(conf);
    return ConfigPageInfo (conf, i18n("Alarms"), i18n("Setup Alarms"), "kalarm");
}


AboutPageInfo TimeControl::createAboutPage()
{
    KAboutData aboutData("kradio",
						 NULL,
                         NULL,
                         I18N_NOOP("Time Control Plugin for KRadio."
                                   "<P>"
                                   "Provides Alarms and Sleep Countdown"
                                   "<P>"),
                         KAboutData::License_GPL,
                         "(c) 2002, 2003 Martin Witte, Klas Kalass",
                         0,
                         "http://sourceforge.net/projects/kradio",
                         0);
    aboutData.addAuthor("Martin Witte",  "", "witte@kawo1.rwth-aachen.de");
    aboutData.addAuthor("Klas Kalass",   "", "klas.kalass@gmx.de");

	return AboutPageInfo(
	          new KRadioAboutWidget(aboutData, KRadioAboutWidget::AbtTabbed),
	          i18n("Alarms"),
	          i18n("Time Control Plugin"),
	          "kalarm"
		   );
}
