/***************************************************************************
                          timecontro-configuration.h  -  description
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
#ifndef KRADIO_TIMECONTROL_CONFIGURATION_H
#define KRADIO_TIMECONTROL_CONFIGURATION_H

#include "radio_interfaces.h"
#include "timecontrol_interfaces.h"
#include "ui_timecontrol-configuration-ui.h"

class TimeControl;
class QWidget;

class TimeControlConfiguration : public QWidget,
                                 public Ui_TimeControlConfigurationUI,
                                 public ITimeControlClient,
                                 public IRadioClient
{
Q_OBJECT
public :
    TimeControlConfiguration (QWidget *parent);
    ~TimeControlConfiguration ();

    bool connectI    (Interface *i) override;
    bool disconnectI (Interface *i) override;

// ITimeControlClient

    bool noticeAlarmsChanged   (const AlarmVector &sl) override;
    bool noticeAlarm           (const Alarm &) override;
    bool noticeNextAlarmChanged(const Alarm *) override;
    bool noticeCountdownStarted(const QDateTime &end) override;
    bool noticeCountdownStopped() override;
    bool noticeCountdownZero   () override;
    bool noticeCountdownSecondsChanged(int n, bool suspendOnSleep) override;

// IRadioClient

    bool noticePowerChanged(bool on) override;
    bool noticeStationChanged (const RadioStation &, int idx) override;
    bool noticeStationsChanged(const StationList &sl) override;
    bool noticePresetFileChanged(const QString &/*f*/)           override { return false; }

    bool noticeRDSStateChanged      (bool  /*enabled*/)          override { return false; }
    bool noticeRDSRadioTextChanged  (const QString &/*s*/)       override { return false; }
    bool noticeRDSStationNameChanged(const QString &/*s*/)       override { return false; }

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/) override { return false; }
    bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID /*id*/) override { return false; }

protected slots:

    void slotDailyChanged (bool daily);
    void slotWeekdaysChanged ();
    void slotEnabledChanged (bool enable);
    void slotStationChanged (int idx);
    void slotAlarmSelectChanged(int idx);
    void slotDateChanged(const QDate &d);
    void slotTimeChanged(const QTime &d);
    void slotVolumeChanged(int v);
    void slotAlarmTypeChanged(int idx);
    void slotRecordingTemplateFilenameChanged (const QString &t);
    void slotRecordingTemplateID3TitleChanged (const QString &t);
    void slotRecordingTemplateID3ArtistChanged(const QString &t);
    void slotRecordingTemplateID3GenreChanged (const QString &t);

    void slotNewAlarm();
    void slotDeleteAlarm();

    void slotOK();
    void slotCancel();
    void slotSetDirty();

protected:

    AlarmVector      alarms;
    QVector<QString> stationIDs;

    bool             ignoreChanges;
    bool             m_dirty;

    QBrush           m_enabledAlarmTextForeground;
    QBrush           m_disabledAlarmTextForeground;
    bool             m_defaultAlarmTextForegroundValid;
};

#endif
