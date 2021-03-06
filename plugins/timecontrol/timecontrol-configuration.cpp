/***************************************************************************
                          timecontrol-configuration.cpp  -  description
                             -------------------
    begin                : Sam Aug 2 2003
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

#include <math.h>
#include <algorithm>
using std::sort;

#include <QDateTime>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>

#include <klocalizedstring.h>
#include <QtGui/QIcon>

#include "stationlist.h"
#include "alarm.h"
#include "errorlog_interfaces.h"
#include "radiostation.h"

#include "timecontrol-configuration.h"
#include "pluginbase_config_page.h"

class DateTimeCmp
{
public:
    bool operator() (const Alarm &a, const Alarm &b) {
        return a.nextAlarm(true) < b.nextAlarm(true);
    }

};

TimeControlConfiguration::TimeControlConfiguration (QWidget *parent)
    : PluginConfigPageBase(parent),
      ITimeControlClient(),
      IRadioClient(),
      ignoreChanges(false),
      m_dirty(false),
      m_enabledAlarmTextForeground(Qt::black),
      m_disabledAlarmTextForeground(Qt::gray),
      m_defaultAlarmTextForegroundValid(false)
{
    setupUi(this);

    buttonAlarmNew   ->setIcon(QIcon::fromTheme("document-new"));
    buttonDeleteAlarm->setIcon(QIcon::fromTheme("edit-delete"));

    comboAlarmType->insertItem(Alarm::StartPlaying,   QIcon::fromTheme("kradio5_muteoff"), i18n("Start Playing"));
    comboAlarmType->insertItem(Alarm::StopPlaying,    QIcon::fromTheme("kradio5_muteon"),  i18n("Stop Playing"));
    comboAlarmType->insertItem(Alarm::StartRecording, QIcon::fromTheme("media-record"),   i18n("Start Recording"));
    comboAlarmType->insertItem(Alarm::StopRecording,  QIcon::fromTheme("kradio5_muteon"),  i18n("Stop Recording"));

    editAlarmDate->setCalendarPopup(true);
    editAlarmVolume->setSpecialValueText(i18n("unchanged"));
    listWeekdays->setSelectionMode(QAbstractItemView::MultiSelection);
    listWeekdays->addItem(i18n("Monday"));
    listWeekdays->addItem(i18n("Tuesday"));
    listWeekdays->addItem(i18n("Wednesday"));
    listWeekdays->addItem(i18n("Thursday"));
    listWeekdays->addItem(i18n("Friday"));
    listWeekdays->addItem(i18n("Saturday"));
    listWeekdays->addItem(i18n("Sunday"));


    QObject::connect(checkboxAlarmDaily,             &QCheckBox  ::toggled,                        this, &TimeControlConfiguration::slotDailyChanged);
    QObject::connect(listWeekdays,                   &QListWidget::itemSelectionChanged,           this, &TimeControlConfiguration::slotWeekdaysChanged);
    QObject::connect(checkboxAlarmEnable,            &QCheckBox  ::toggled,                        this, &TimeControlConfiguration::slotEnabledChanged);
    QObject::connect(comboStationSelection,          QOverload<int>::of(&QComboBox::highlighted),  this, &TimeControlConfiguration::slotStationChanged);
    QObject::connect(listAlarms,                     &QListWidget::currentRowChanged,              this, &TimeControlConfiguration::slotAlarmSelectChanged);
    QObject::connect(editAlarmDate,                  &QDateEdit  ::dateChanged,                    this, &TimeControlConfiguration::slotDateChanged);
    QObject::connect(editAlarmTime,                  &QTimeEdit  ::timeChanged,                    this, &TimeControlConfiguration::slotTimeChanged);
    QObject::connect(editAlarmVolume,                QOverload<int>::of(&QSpinBox ::valueChanged), this, &TimeControlConfiguration::slotVolumeChanged);
    QObject::connect(buttonAlarmNew,                 &QPushButton::clicked,                        this, &TimeControlConfiguration::slotNewAlarm);
    QObject::connect(buttonDeleteAlarm,              &QPushButton::clicked,                        this, &TimeControlConfiguration::slotDeleteAlarm);
    QObject::connect(comboAlarmType,                 QOverload<int>::of(&QComboBox::highlighted),  this, &TimeControlConfiguration::slotAlarmTypeChanged);
    QObject::connect(editRecordingTemplateFileName,  &QLineEdit::textEdited,                       this, &TimeControlConfiguration::slotRecordingTemplateFilenameChanged);
    QObject::connect(editRecordingTemplateID3Title,  &QLineEdit::textEdited,                       this, &TimeControlConfiguration::slotRecordingTemplateID3TitleChanged);
    QObject::connect(editRecordingTemplateID3Artist, &QLineEdit::textEdited,                       this, &TimeControlConfiguration::slotRecordingTemplateID3ArtistChanged);
    QObject::connect(editRecordingTemplateID3Genre,  &QLineEdit::textEdited,                       this, &TimeControlConfiguration::slotRecordingTemplateID3GenreChanged);


    QObject::connect(checkboxAlarmDaily,             &QCheckBox  ::toggled,                        this, &TimeControlConfiguration::slotSetDirty);
    QObject::connect(listWeekdays,                   &QListWidget::itemSelectionChanged,           this, &TimeControlConfiguration::slotSetDirty);
    QObject::connect(checkboxAlarmEnable,            &QCheckBox  ::toggled,                        this, &TimeControlConfiguration::slotSetDirty);
    QObject::connect(comboStationSelection,          QOverload<int>::of(&QComboBox::activated),    this, &TimeControlConfiguration::slotSetDirty);
    QObject::connect(editAlarmDate,                  &QDateEdit  ::dateChanged,                    this, &TimeControlConfiguration::slotSetDirty);
    QObject::connect(editAlarmTime,                  &QTimeEdit  ::timeChanged,                    this, &TimeControlConfiguration::slotSetDirty);
    QObject::connect(editAlarmVolume,                QOverload<int>::of(&QSpinBox ::valueChanged), this, &TimeControlConfiguration::slotSetDirty);
    QObject::connect(buttonAlarmNew,                 &QPushButton::clicked,                        this, &TimeControlConfiguration::slotSetDirty);
    QObject::connect(buttonDeleteAlarm,              &QPushButton::clicked,                        this, &TimeControlConfiguration::slotSetDirty);
    QObject::connect(comboAlarmType,                 QOverload<int>::of(&QComboBox::activated),    this, &TimeControlConfiguration::slotSetDirty);
    QObject::connect(editSleep,                      QOverload<int>::of(&QSpinBox ::valueChanged), this, &TimeControlConfiguration::slotSetDirty);
    QObject::connect(cbSuspendOnSleep,               &QCheckBox  ::toggled,                        this, &TimeControlConfiguration::slotSetDirty);
    QObject::connect(editRecordingTemplateFileName,  &QLineEdit::textEdited,                       this, &TimeControlConfiguration::slotSetDirty);
    QObject::connect(editRecordingTemplateID3Title,  &QLineEdit::textEdited,                       this, &TimeControlConfiguration::slotSetDirty);
    QObject::connect(editRecordingTemplateID3Artist, &QLineEdit::textEdited,                       this, &TimeControlConfiguration::slotSetDirty);
    QObject::connect(editRecordingTemplateID3Genre,  &QLineEdit::textEdited,                       this, &TimeControlConfiguration::slotSetDirty);
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
    int idx = listAlarms->currentRow();
    int currentID = (idx >= 0 && idx < alarms.size()) ? alarms[idx].ID() : -1;

    alarms = sl;
    sort(alarms.begin(), alarms.end(), DateTimeCmp());

    bool oldBlock = listAlarms->signalsBlocked();
    listAlarms->blockSignals(true);

    listAlarms->clear();
    idx = -1;
    int k = 0;
    for (ciAlarmVector i = alarms.constBegin(); i != alarms.constEnd(); ++i, ++k) {
        const Alarm &alarm = *i;
        QString      dateString = alarm.nextAlarm(true).toString();
        if (!alarm.isEnabled()) {
            dateString = i18nc("disabled alarm", "%1 (disabled)", dateString);
        }
        listAlarms->addItem(dateString);
        QListWidgetItem *item = listAlarms->item(listAlarms->count()-1);
        if (!m_defaultAlarmTextForegroundValid) {
            m_enabledAlarmTextForeground = item->foreground();
        }
        item->setForeground(!alarm.isEnabled() ? m_disabledAlarmTextForeground : m_enabledAlarmTextForeground);
        if (i->ID() == currentID)
            idx = k;
    }
    listAlarms->setCurrentRow(idx, QItemSelectionModel::Select);

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

bool TimeControlConfiguration::noticeCountdownSecondsChanged(int n, bool suspendOnSleep)
{
    editSleep->setValue((int)rint(n / 60));
    cbSuspendOnSleep->setChecked(suspendOnSleep);
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
    comboStationSelection->addItem(i18n("<do not change>"));
    stationIDs.push_back(QString());

    for (StationList::const_iterator i = sl.begin(); i != sl.end(); ++i) {
        comboStationSelection->addItem(QIcon((*i)->iconName()),
                                       (*i)->longName());
        stationIDs.push_back((*i)->stationID());
    }
    return true;
}


// Slots


void TimeControlConfiguration::slotDateChanged( const QDate &d )
{
    if (ignoreChanges) return;

    int idx = listAlarms->currentRow();
    if (idx >= 0 && idx < alarms.size()) {
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

    int idx = listAlarms->currentRow();
    if (idx >= 0 && idx < alarms.size()) {
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

    int idx = listAlarms->currentRow();
    if (idx >= 0 && idx < alarms.size()) {
        Alarm &a = alarms[idx];
        a.setDaily(b);

        ignoreChanges = true;
        bool oldBlock = listAlarms->signalsBlocked();
        listAlarms->blockSignals(true);
        noticeAlarmsChanged(alarms);
        listAlarms->blockSignals(oldBlock);
        ignoreChanges = false;

        editAlarmDate      ->setDisabled(b);
        labelAlarmDate     ->setDisabled(b);
        listWeekdays       ->setDisabled(!b);
        labelActiveWeekdays->setDisabled(!b);
    }
}


void TimeControlConfiguration::slotWeekdaysChanged ()
{
    if (ignoreChanges) return;

    int mask = 0;
    QList<QListWidgetItem *> sel = listWeekdays->selectedItems();
    foreach (QListWidgetItem *item, sel) {
        mask |= (1 << listWeekdays->row(item));
    }

    int idx = listAlarms->currentRow();
    if (idx >= 0 && idx < alarms.size()) {
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
    int idx = listAlarms->currentRow();
    if (idx >= 0 && idx < alarms.size()) {
        Alarm           &alarm      = alarms[idx];
        QString          dateString = alarm.nextAlarm(true).toString();
        QListWidgetItem *item       = listAlarms->item(idx);
        alarm.setEnabled(b);
        if (!alarm.isEnabled()) {
            dateString = i18nc("disabled alarm", "%1 (disabled)", dateString);
        }
        item->setForeground(!alarm.isEnabled() ? m_disabledAlarmTextForeground : m_enabledAlarmTextForeground);
        item->setText(dateString);
    }
}


void TimeControlConfiguration::slotStationChanged( int i )
{
    int idx = listAlarms->currentRow();
    if (   idx >= 0 && idx < alarms.size()
        && i >= 0 && i < stationIDs.size())
    {
        alarms[idx].setStationID( stationIDs[i] );
    }
}


void TimeControlConfiguration::slotVolumeChanged( int v )
{
    int idx = listAlarms->currentRow();
    if (idx >= 0 && idx < alarms.size()) {
        alarms[idx].setVolumePreset(0.01 * (float)v);
    }
}


void TimeControlConfiguration::slotAlarmTypeChanged(int t)
{
    int idx = listAlarms->currentRow();
    if (idx >= 0 && idx < alarms.size()) {
        alarms[idx].setAlarmType((Alarm::AlarmType)t);
        editRecordingTemplateFileName  ->setDisabled(t != Alarm::StartRecording);
        editRecordingTemplateID3Title  ->setDisabled(t != Alarm::StartRecording);
        editRecordingTemplateID3Artist ->setDisabled(t != Alarm::StartRecording);
        editRecordingTemplateID3Genre  ->setDisabled(t != Alarm::StartRecording);
        labelRecordingTemplate         ->setDisabled(t != Alarm::StartRecording);
        labelRecordingTemplateFilename ->setDisabled(t != Alarm::StartRecording);
        labelRecordingTemplateID3Title ->setDisabled(t != Alarm::StartRecording);
        labelRecordingTemplateID3Artist->setDisabled(t != Alarm::StartRecording);
        labelRecordingTemplateID3Genre ->setDisabled(t != Alarm::StartRecording);
    }
}


void TimeControlConfiguration::slotRecordingTemplateFilenameChanged(const QString &t)
{
    int idx = listAlarms->currentRow();
    if (idx >= 0 && idx < alarms.size()) {
        recordingTemplate_t tmp = alarms[idx].recordingTemplate();
        tmp.filename = t;
        alarms[idx].setRecordingTemplate(tmp);
    }
}


void TimeControlConfiguration::slotRecordingTemplateID3TitleChanged(const QString &t)
{
    int idx = listAlarms->currentRow();
    if (idx >= 0 && idx < alarms.size()) {
        recordingTemplate_t tmp = alarms[idx].recordingTemplate();
        tmp.id3Title = t;
        alarms[idx].setRecordingTemplate(tmp);
    }
}


void TimeControlConfiguration::slotRecordingTemplateID3ArtistChanged(const QString &t)
{
    int idx = listAlarms->currentRow();
    if (idx >= 0 && idx < alarms.size()) {
        recordingTemplate_t tmp = alarms[idx].recordingTemplate();
        tmp.id3Artist = t;
        alarms[idx].setRecordingTemplate(tmp);
    }
}


void TimeControlConfiguration::slotRecordingTemplateID3GenreChanged(const QString &t)
{
    int idx = listAlarms->currentRow();
    if (idx >= 0 && idx < alarms.size()) {
        recordingTemplate_t tmp = alarms[idx].recordingTemplate();
        tmp.id3Genre = t;
        alarms[idx].setRecordingTemplate(tmp);
    }
}


void TimeControlConfiguration::slotAlarmSelectChanged(int idx)
{
    if (ignoreChanges) return;
    ignoreChanges = true;

    Alarm a;
    bool  valid = false;

    if (idx >= 0 && idx < alarms.size()) {

        a = alarms[idx];
        valid = true;

    }

    editAlarmDate         ->setDisabled(!valid || a.isDaily());
    labelAlarmDate        ->setDisabled(!valid || a.isDaily());
    listWeekdays          ->setDisabled(!valid ||!a.isDaily());
    labelActiveWeekdays   ->setDisabled(!valid ||!a.isDaily());
    editAlarmTime         ->setDisabled(!valid);
    labelAlarmTime        ->setDisabled(!valid);
    labelAlarmVolume      ->setDisabled(!valid);
    editAlarmVolume       ->setDisabled(!valid);
    checkboxAlarmDaily    ->setDisabled(!valid);
    checkboxAlarmEnable   ->setDisabled(!valid);
    comboStationSelection ->setDisabled(!valid);
    labelStationSelection ->setDisabled(!valid);
    buttonDeleteAlarm     ->setDisabled(!valid);
    comboAlarmType        ->setDisabled(!valid);
    editRecordingTemplateFileName  ->setDisabled(!valid || a.alarmType() != Alarm::StartRecording);
    editRecordingTemplateID3Title  ->setDisabled(!valid || a.alarmType() != Alarm::StartRecording);
    editRecordingTemplateID3Artist ->setDisabled(!valid || a.alarmType() != Alarm::StartRecording);
    editRecordingTemplateID3Genre  ->setDisabled(!valid || a.alarmType() != Alarm::StartRecording);
    labelRecordingTemplate         ->setDisabled(!valid || a.alarmType() != Alarm::StartRecording);
    labelRecordingTemplateFilename ->setDisabled(!valid || a.alarmType() != Alarm::StartRecording);
    labelRecordingTemplateID3Title ->setDisabled(!valid || a.alarmType() != Alarm::StartRecording);
    labelRecordingTemplateID3Artist->setDisabled(!valid || a.alarmType() != Alarm::StartRecording);
    labelRecordingTemplateID3Genre ->setDisabled(!valid || a.alarmType() != Alarm::StartRecording);

    editAlarmDate         ->setDate(a.alarmTime().date());
    editAlarmTime         ->setTime(a.alarmTime().time());
    checkboxAlarmDaily    ->setChecked(a.isDaily());
    checkboxAlarmEnable   ->setChecked(a.isEnabled());
    editAlarmVolume       ->setValue((int)rint(a.volumePreset() * 100));
    comboAlarmType        ->setCurrentIndex(a.alarmType());
    editRecordingTemplateFileName ->setText(a.recordingTemplate().filename);
    editRecordingTemplateID3Title ->setText(a.recordingTemplate().id3Title);
    editRecordingTemplateID3Artist->setText(a.recordingTemplate().id3Artist);
    editRecordingTemplateID3Genre ->setText(a.recordingTemplate().id3Genre);

    int k = 0;
    const QString &sID = a.stationID();
    for (int i = 0; !k && i < (int)stationIDs.size(); ++i)
        if (stationIDs[i] == sID) k = i;
    comboStationSelection->setCurrentIndex(k);

    int m = a.weekdayMask();
    for (int i = 0; i < 7; ++i) {
        listWeekdays->item(i)->setSelected(m & (1 << i));
    }

    ignoreChanges = false;
}


void TimeControlConfiguration::slotNewAlarm()
{
    QDateTime  dt(QDateTime::currentDateTime());
    Alarm a(dt, false, true);
    alarms.push_back(a);
    listAlarms->addItem(a.alarmTime().toString());
    listAlarms->setCurrentRow(listAlarms->count() - 1, QItemSelectionModel::Select);
    noticeAlarmsChanged(alarms);
}


void TimeControlConfiguration::slotDeleteAlarm()
{
    int idx = listAlarms->currentRow();

    if (idx >= 0 && idx < alarms.size()) {
        alarms.remove(idx);
        delete listAlarms->item(idx);
        int new_idx = idx;
        if (new_idx > alarms.size()) {
            new_idx = idx - 1;
        }
        listAlarms->setCurrentRow(idx, QItemSelectionModel::Select);
        slotAlarmSelectChanged(new_idx);
    }
}


void TimeControlConfiguration::slotOK()
{
    if (m_dirty) {
        sendAlarms(alarms);
        sendCountdownSeconds(editSleep->value() * 60, cbSuspendOnSleep->isChecked());
        m_dirty = false;
    }
}

void TimeControlConfiguration::slotCancel()
{
    if (m_dirty) {
        noticeAlarmsChanged(queryAlarms());
        noticeCountdownSecondsChanged(queryCountdownSeconds(), querySuspendOnSleep());
        m_dirty = false;
    }
}

void TimeControlConfiguration::slotSetDirty()
{
    if (!ignoreChanges) {
        m_dirty = true;
    }
}


