/***************************************************************************
                          timecontrol.cpp  -  description
                             -------------------
    begin                : Son Jan 12 2003
    copyright            : (C) 2003 by Martin Witte
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

#include <KAboutData>
#include <kconfig.h>
#include <solid/powermanagement.h>

#include "timecontrol.h"
#include "timecontrol-configuration.h"
#include "pluginmanager.h"

//static const char AlarmListElement[]              = "alarmlist";
//static const char AlarmElement[]                  = "alarm";
//static const char AlarmDateElement[]              = "date";
static const char AlarmTimeElement[]              = "time";
static const char AlarmDailyElement[]             = "daily";
static const char AlarmWeekdayMaskElement[]       = "weekdayMask";
static const char AlarmEnabledElement[]           = "enabled";
static const char AlarmStationIDElement[]         = "stationID";
//static const char AlarmFrequencyElement[]         = "frequency";
static const char AlarmVolumeElement[]            = "volume";
static const char AlarmTypeElement[]              = "type";
static const char AlarmRecordingTemplateElement[] = "recordingTemplate";


///////////////////////////////////////////////////////////////////////
//// plugin library functions

static KAboutData aboutData()
{
    KAboutData about("TimeControl",
                     i18nc("@title", "Alarms"),
                     KRADIO_VERSION,
                     i18nc("@title", "Time Control and Alarm Functions"),
                     KAboutLicense::LicenseKey::GPL,
                     i18nc("@info:credit", "(c) 2002-2005 Martin Witte, Klas Kalass"),
                     i18n("Provides alarms and sleep countdown."),
                     "http://sourceforge.net/projects/kradio",
                     "emw-kradio@nocabal.de");
    about.addAuthor(i18nc("@info:credit", "Martin Witte"), NULL, "emw-kradio@nocabal.de");
    about.addAuthor(i18nc("@info:credit", "Klas Kalass" ), NULL, "klas.kalass@gmx.de");
    return about;
}

KRADIO_EXPORT_PLUGIN(TimeControl, aboutData())
#include "timecontrol.moc"

///////////////////////////////////////////////////////////////////////


TimeControl::TimeControl (const QString &instanceID, const QString &n)
    : PluginBase(instanceID, n, i18n("Time Control Plugin")),
      m_waitingFor(NULL),
      m_countdownSeconds(0),
      m_suspendOnSleep(false),
      m_alarmTimer(this),
      m_countdownTimer(this)
{
    QObject::connect(&m_alarmTimer,     &QTimer::timeout, this, &TimeControl::slotQTimerAlarmTimeout);
    QObject::connect(&m_countdownTimer, &QTimer::timeout, this, &TimeControl::slotQTimerCountdownTimeout);
    
    QObject::connect(Solid::PowerManagement::notifier(), &Solid::PowerManagement::Notifier::resumingFromSuspend, this, &TimeControl::slotResumingFromSuspend);
}


TimeControl::~TimeControl ()
{
    m_waitingFor = NULL;
}

bool   TimeControl::connectI (Interface *i)
{
    bool a = ITimeControl::connectI(i);
    bool b = PluginBase::connectI(i);
    return a || b;
}

bool   TimeControl::disconnectI (Interface *i)
{
    bool a = ITimeControl::disconnectI(i);
    bool b = PluginBase::disconnectI(i);
    return a || b;
}

bool TimeControl::setAlarms (const AlarmVector &al)
{
    if (m_alarms != al) {
        m_waitingFor = NULL;

        m_alarms = al;

        updateTimers();

        notifyAlarmsChanged(m_alarms);
    }
    return true;
}


bool TimeControl::setCountdownSeconds(int n, bool suspendOnSleep)
{
    int    oldSec     = m_countdownSeconds;
    bool   oldSuspend = m_suspendOnSleep;
    m_countdownSeconds = n;
    m_suspendOnSleep   = suspendOnSleep;
    if (oldSec != n || oldSuspend != suspendOnSleep) {
        notifyCountdownSecondsChanged(m_countdownSeconds, m_suspendOnSleep);
    }
    return true;
}


bool TimeControl::startCountdown()
{
    m_countdownEnd = QDateTime::currentDateTime().addSecs(m_countdownSeconds);
    m_countdownTimer.setSingleShot(true);
    m_countdownTimer.start(m_countdownSeconds * 1000);

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
    
    if (m_suspendOnSleep) {
        Solid::PowerManagement::requestSleep(Solid::PowerManagement::SuspendState, this, SLOT(slotResumingFromSuspend()));
    }
}


void TimeControl::slotQTimerAlarmTimeout()
{
    if (m_waitingFor) {
        notifyAlarm(*m_waitingFor);
    }
    updateTimers();

}

void TimeControl::updateTimers()
{
    QDateTime now  = QDateTime::currentDateTime();
    Alarm const *n = getNextAlarm();
    QDateTime na   = getNextAlarmTime();

    m_waitingFor = NULL;

    if (na.isValid()) {

        int days  = now.daysTo(na);
        int msecs = now.time().msecsTo(na.time());

        if (days >= 1) {

            m_alarmTimer.setSingleShot(true);
            m_alarmTimer.start(24 * 3600 * 1000);

        } else if (days >= 0) {

            if (days > 0) {
                msecs += days * 24 * 3600 * 1000;
            }

            if (msecs > 0) {
                m_waitingFor = n;

                m_alarmTimer.setSingleShot(true);
                m_alarmTimer.start(msecs);
            }
        }
    }
}


void    TimeControl::restoreState (const KConfigGroup &config)
{
    PluginBase::restoreState(config);

    AlarmVector al;

    int nAlarms = config.readEntry ("nAlarms", 0);
    for (int idx = 1; idx <= nAlarms; ++idx) {

        QString   num               = QString::number(idx);
        QDateTime d                 = config.readEntry(AlarmTimeElement              + num, QDateTime());
        bool      enable            = config.readEntry(AlarmEnabledElement           + num, false);
        bool      daily             = config.readEntry(AlarmDailyElement             + num, false);
        int       weekdayMask       = config.readEntry(AlarmWeekdayMaskElement       + num, 0x7F);
        float     vol               = config.readEntry(AlarmVolumeElement            + num, 1.0);
        QString   sid               = config.readEntry(AlarmStationIDElement         + num, QString());
        int       type              = config.readEntry(AlarmTypeElement              + num, (int)Alarm::StartPlaying);

        recordingTemplate_t recordingTemplate;
        recordingTemplate.restoreState(AlarmRecordingTemplateElement + num, config, AlarmRecordingTemplateElement + num);

        enable &= d.isValid();

        Alarm a ( d, daily, enable);
        a.setVolumePreset(vol);
        a.setWeekdayMask(weekdayMask);
        a.setStationID(sid);
        a.setAlarmType((Alarm::AlarmType)type);
        a.setRecordingTemplate(recordingTemplate);
        al.push_back(a);
    }

    setAlarms(al);
    setCountdownSeconds(
        config.readEntry("countdownSeconds", 30*60),
        config.readEntry("suspendOnSleep",   false)
    );
}


void    TimeControl::saveState    (KConfigGroup &config) const
{
    PluginBase::saveState(config);

    config.writeEntry("nAlarms", m_alarms.size());
    int idx = 1;
    ciAlarmVector end = m_alarms.end();
    for (ciAlarmVector i = m_alarms.begin(); i != end; ++i, ++idx) {
        QString num = QString::number(idx);
        config.writeEntry (AlarmTimeElement              + num, i->alarmTime());
        config.writeEntry (AlarmEnabledElement           + num, i->isEnabled());
        config.writeEntry (AlarmDailyElement             + num, i->isDaily());
        config.writeEntry (AlarmWeekdayMaskElement       + num, i->weekdayMask());
        config.writeEntry (AlarmVolumeElement            + num, i->volumePreset());
        config.writeEntry (AlarmStationIDElement         + num, i->stationID());
        config.writeEntry (AlarmTypeElement              + num, (int)i->alarmType());
        i->recordingTemplate().saveState(AlarmRecordingTemplateElement + num, config);
    }

    config.writeEntry("countdownSeconds",  m_countdownSeconds);
    config.writeEntry("suspendOnSleep",    m_suspendOnSleep);
}


ConfigPageInfo TimeControl::createConfigurationPage()
{
    TimeControlConfiguration *conf = new TimeControlConfiguration(NULL);
    connectI(conf);
    return ConfigPageInfo (conf,
                           i18n("Alarms"),
                           i18n("Setup Alarms"),
                           "kradio5_kalarm"
                          );
}



void TimeControl::slotResumingFromSuspend()
{
    updateTimers();
}


