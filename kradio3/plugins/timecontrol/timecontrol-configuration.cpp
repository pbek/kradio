/***************************************************************************
                          timecontrol-configuration.cpp  -  description
                             -------------------
    begin                : Sam Aug 2 2003
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

#include <math.h>
#include <algorithm>
using std::sort;

#include <qdatetime.h>
#include <qlistbox.h>
#include <qcombobox.h>
#include <qdatetimeedit.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>

#include <klistbox.h>
#include <klocale.h>

#include "../../src/libkradio/stationlist.h"
#include "../../src/libkradio/alarm.h"
#include "../../src/interfaces/errorlog-interfaces.h"
#include "../../src/radio-stations/radiostation.h"

#include "timecontrol-configuration.h"

class DateTimeCmp
{
public:
    bool operator() (const Alarm &a, const Alarm &b) {
        return a.nextAlarm(true) < b.nextAlarm(true);
    }

};

TimeControlConfiguration::TimeControlConfiguration (QWidget *parent)
    : TimeControlConfigurationUI(parent),
      ITimeControlClient(),
      IRadioClient(),
      ignoreChanges(false),
      m_dirty(false)
{

    QObject::connect(checkboxAlarmDaily,    SIGNAL(toggled(bool)),               this, SLOT(slotDailyChanged(bool)));
    QObject::connect(listWeekdays,          SIGNAL(highlighted(int)),            this, SLOT(slotWeekdaysChanged()));
    QObject::connect(checkboxAlarmEnable,   SIGNAL(toggled(bool)),               this, SLOT(slotEnabledChanged(bool)));
    QObject::connect(comboStationSelection, SIGNAL(highlighted(int)),            this, SLOT(slotStationChanged(int)));
    QObject::connect(listAlarms,            SIGNAL(highlighted(int)),            this, SLOT(slotAlarmSelectChanged(int)));
    QObject::connect(editAlarmDate,         SIGNAL(valueChanged(const QDate &)), this, SLOT(slotDateChanged(const QDate &)));
    QObject::connect(editAlarmTime,         SIGNAL(valueChanged(const QTime &)), this, SLOT(slotTimeChanged(const QTime &)));
    QObject::connect(editAlarmVolume,       SIGNAL(valueChanged(int)),           this, SLOT(slotVolumeChanged(int)));
    QObject::connect(buttonAlarmNew,        SIGNAL(clicked()),                   this, SLOT(slotNewAlarm()));
    QObject::connect(buttonDeleteAlarm,     SIGNAL(clicked()),                   this, SLOT(slotDeleteAlarm()));
    QObject::connect(comboAlarmType,        SIGNAL(highlighted(int)),            this, SLOT(slotAlarmTypeChanged(int)));

    QObject::connect(checkboxAlarmDaily,    SIGNAL(toggled(bool)),               this, SLOT(slotSetDirty()));
    QObject::connect(listWeekdays,          SIGNAL(selectionChanged()),          this, SLOT(slotSetDirty()));
    QObject::connect(checkboxAlarmEnable,   SIGNAL(toggled(bool)),               this, SLOT(slotSetDirty()));
    QObject::connect(comboStationSelection, SIGNAL(activated(int)),              this, SLOT(slotSetDirty()));
    QObject::connect(editAlarmDate,         SIGNAL(valueChanged(const QDate &)), this, SLOT(slotSetDirty()));
    QObject::connect(editAlarmTime,         SIGNAL(valueChanged(const QTime &)), this, SLOT(slotSetDirty()));
    QObject::connect(editAlarmVolume,       SIGNAL(valueChanged(int)),           this, SLOT(slotSetDirty()));
    QObject::connect(buttonAlarmNew,        SIGNAL(clicked()),                   this, SLOT(slotSetDirty()));
    QObject::connect(buttonDeleteAlarm,     SIGNAL(clicked()),                   this, SLOT(slotSetDirty()));
    QObject::connect(comboAlarmType,        SIGNAL(activated(int)),              this, SLOT(slotSetDirty()));
    QObject::connect(editSleep,             SIGNAL(valueChanged(int)),           this, SLOT(slotSetDirty()));
}

TimeControlConfiguration::~TimeControlConfiguration ()
{
}

bool TimeControlConfiguration::connectI (Interface *i)
{
    bool a = ITimeControlClient::connectI(i);
    bool b = IRadioClient::connectI(i);
    return a || b;
}


bool TimeControlConfiguration::disconnectI (Interface *i)
{
    bool a = ITimeControlClient::disconnectI(i);
    bool b = IRadioClient::disconnectI(i);
    return a || b;
}


// ITimeControlClient

bool TimeControlConfiguration::noticeAlarmsChanged(const AlarmVector &sl)
{
    int idx = listAlarms->currentItem();
    int currentID = (idx >= 0 && (unsigned)idx < alarms.size()) ? alarms[idx].ID() : -1;

    alarms = sl;
    sort(alarms.begin(), alarms.end(), DateTimeCmp());

    bool oldBlock = listAlarms->signalsBlocked();
    listAlarms->blockSignals(true);

    listAlarms->clear();
    idx = -1;
    int k = 0;
    for (ciAlarmVector i = alarms.begin(); i != alarms.end(); ++i, ++k) {
        listAlarms->insertItem(i->nextAlarm(true).toString());
        if (i->ID() == currentID)
            idx = k;
    }
    listAlarms->setCurrentItem(idx);

    listAlarms->blockSignals(oldBlock);

    slotAlarmSelectChanged(idx);
    return true;
}

bool TimeControlConfiguration::noticeAlarm(const Alarm &)
{
    return false;
}

bool TimeControlConfiguration::noticeNextAlarmChanged(const Alarm *)
{
    noticeAlarmsChanged(alarms);
    return true;
}

bool TimeControlConfiguration::noticeCountdownStarted(const QDateTime &/*end*/)
{
    return false;
}

bool TimeControlConfiguration::noticeCountdownStopped()
{
    return false;
}

bool TimeControlConfiguration::noticeCountdownZero()
{
    return false;
}

bool TimeControlConfiguration::noticeCountdownSecondsChanged(int n)
{
    editSleep->setValue((int)rint(n / 60));
    return false;
}


// IRadioClient

bool TimeControlConfiguration::noticePowerChanged(bool /*on*/)
{
    return false;
}

bool TimeControlConfiguration::noticeStationChanged (const RadioStation &, int /*idx*/)
{
    return false;
}

bool TimeControlConfiguration::noticeStationsChanged(const StationList &sl)
{
    comboStationSelection->clear();
    stationIDs.clear();
    comboStationSelection->insertItem(i18n("<don't change>"));
    stationIDs.push_back(QString::null);

    for (RawStationList::Iterator i(sl.all()); i.current(); ++i) {
        comboStationSelection->insertItem(i.current()->iconName(),
                                          i.current()->longName());
        stationIDs.push_back(i.current()->stationID());
    }
    return true;
}


// Slots


void TimeControlConfiguration::slotDateChanged( const QDate &d )
{
    if (ignoreChanges) return;

    int idx = listAlarms->currentItem();
    if (idx >= 0 && (unsigned)idx < alarms.size()) {
        Alarm &a = alarms[idx];
        a.setDate(d);

        ignoreChanges = true;
        bool oldBlock = listAlarms->signalsBlocked();
        listAlarms->blockSignals(true);
        noticeAlarmsChanged(alarms);
        listAlarms->blockSignals(oldBlock);
        ignoreChanges = false;
    }
}


void TimeControlConfiguration::slotTimeChanged(const QTime &t)
{
    if (ignoreChanges) return;

    int idx = listAlarms->currentItem();
    if (idx >= 0 && (unsigned)idx < alarms.size()) {
        Alarm &a = alarms[idx];
        a.setTime(t);

        ignoreChanges = true;
        bool oldBlock = listAlarms->signalsBlocked();
        listAlarms->blockSignals(true);
        noticeAlarmsChanged(alarms);
        listAlarms->blockSignals(oldBlock);
        ignoreChanges = false;
    }
}


void TimeControlConfiguration::slotDailyChanged (bool b)
{
    if (ignoreChanges) return;

    int idx = listAlarms->currentItem();
    if (idx >= 0 && (unsigned)idx < alarms.size()) {
        Alarm &a = alarms[idx];
        a.setDaily(b);

        ignoreChanges = true;
        bool oldBlock = listAlarms->signalsBlocked();
        listAlarms->blockSignals(true);
        noticeAlarmsChanged(alarms);
        listAlarms->blockSignals(oldBlock);
        ignoreChanges = false;

        editAlarmDate ->setDisabled(b);
        labelAlarmDate->setDisabled(b);
        listWeekdays  ->setDisabled(!b);
    }
}


void TimeControlConfiguration::slotWeekdaysChanged ()
{
    if (ignoreChanges) return;

    int mask = 0;
    for (int i = 0; i < 7; ++i) {
        if (listWeekdays->isSelected(i)) {
            mask |= (1 << i);
        }
    }

    int idx = listAlarms->currentItem();
    if (idx >= 0 && (unsigned)idx < alarms.size()) {
        Alarm &a = alarms[idx];
        a.setWeekdayMask(mask);

        ignoreChanges = true;
        bool oldBlock = listAlarms->signalsBlocked();
        listAlarms->blockSignals(true);
        noticeAlarmsChanged(alarms);
        listAlarms->blockSignals(oldBlock);
        ignoreChanges = false;
    }
}


void TimeControlConfiguration::slotEnabledChanged( bool b)
{
    int idx = listAlarms->currentItem();
    if (idx >= 0 && (unsigned)idx < alarms.size()) {
        alarms[idx].setEnabled(b);
    }
}


void TimeControlConfiguration::slotStationChanged( int i )
{
    int idx = listAlarms->currentItem();
    if (   idx >= 0 && (unsigned)idx < alarms.size()
        && i >= 0 && (unsigned)i < stationIDs.size())
    {
        alarms[idx].setStationID( stationIDs[i] );
    }
}


void TimeControlConfiguration::slotVolumeChanged( int v )
{
    int idx = listAlarms->currentItem();
    if (idx >= 0 && (unsigned)idx < alarms.size()) {
        alarms[idx].setVolumePreset(0.01 * (float)v);
    }
}


void TimeControlConfiguration::slotAlarmTypeChanged(int t)
{
    int idx = listAlarms->currentItem();
    if (idx >= 0 && (unsigned)idx < alarms.size()) {
        alarms[idx].setAlarmType((Alarm::AlarmType)t);
    }
}


void TimeControlConfiguration::slotAlarmSelectChanged(int idx)
{
    if (ignoreChanges) return;
    ignoreChanges = true;

    Alarm a;
    bool  valid = false;

    if (idx >= 0 && (unsigned)idx < alarms.size()) {

        a = alarms[idx];
        valid = true;

    }

    editAlarmDate        ->setDisabled(!valid || a.isDaily());
    labelAlarmDate       ->setDisabled(!valid || a.isDaily());
    listWeekdays         ->setDisabled(!valid ||!a.isDaily());
    editAlarmTime        ->setDisabled(!valid);
    labelAlarmTime       ->setDisabled(!valid);
    labelAlarmVolume     ->setDisabled(!valid);
    editAlarmVolume      ->setDisabled(!valid);
    checkboxAlarmDaily   ->setDisabled(!valid);
    checkboxAlarmEnable  ->setDisabled(!valid);
    comboStationSelection->setDisabled(!valid);
    labelStationSelection->setDisabled(!valid);
    buttonDeleteAlarm    ->setDisabled(!valid);
    comboAlarmType       ->setDisabled(!valid);

    editAlarmDate        ->setDate(a.alarmTime().date());
    editAlarmTime        ->setTime(a.alarmTime().time());
    checkboxAlarmDaily   ->setChecked(a.isDaily());
    checkboxAlarmEnable  ->setChecked(a.isEnabled());
    editAlarmVolume      ->setValue((int)rint(a.volumePreset() * 100));
    comboAlarmType       ->setCurrentItem(a.alarmType());

    int k = 0;
    const QString &sID = a.stationID();
    for (int i = 0; !k && i < (int)stationIDs.size(); ++i)
        if (stationIDs[i] == sID) k = i;
    comboStationSelection->setCurrentItem(k);

    int m = a.weekdayMask();
    for (int i = 0; i < 7; ++i) {
        listWeekdays->setSelected(i, m & (1 << i));
    }

    ignoreChanges = false;
}


void TimeControlConfiguration::slotNewAlarm()
{
    QDateTime  dt(QDateTime::currentDateTime());
    Alarm a(dt, false, false);
    alarms.push_back(a);
    listAlarms->insertItem(a.alarmTime().toString());
    listAlarms->setSelected(listAlarms->count() - 1, true);
    noticeAlarmsChanged(alarms);
}


void TimeControlConfiguration::slotDeleteAlarm()
{
    int idx = listAlarms->currentItem();

    if (idx >= 0 && (unsigned)idx < alarms.size()) {
        // unfortunately a function vector<>::erase(idx) does not exist
        iAlarmVector i = alarms.begin();
        for (int k = 0; k < idx; ++k)
            ++i;
        if (i != alarms.end())
            alarms.erase(i);
        listAlarms->removeItem(idx);
    }
}


void TimeControlConfiguration::slotOK()
{
    if (m_dirty) {
        sendAlarms(alarms);
        sendCountdownSeconds(editSleep->value() * 60);
        m_dirty = false;
    }
}

void TimeControlConfiguration::slotCancel()
{
    if (m_dirty) {
        noticeAlarmsChanged(queryAlarms());
        noticeCountdownSecondsChanged(queryCountdownSeconds());
        m_dirty = false;
    }
}

void TimeControlConfiguration::slotSetDirty()
{
    if (!ignoreChanges) {
        m_dirty = true;
    }
}


#include "timecontrol-configuration.moc"
