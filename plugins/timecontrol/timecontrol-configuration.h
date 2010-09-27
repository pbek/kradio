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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

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

    bool connectI (Interface *i);
    bool disconnectI (Interface *i);

// ITimeControlClient

    bool noticeAlarmsChanged(const AlarmVector &sl);
    bool noticeAlarm(const Alarm &);
    bool noticeNextAlarmChanged(const Alarm *);
    bool noticeCountdownStarted(const QDateTime &end);
    bool noticeCountdownStopped();
    bool noticeCountdownZero();
    bool noticeCountdownSecondsChanged(int n);

// IRadioClient

    bool noticePowerChanged(bool on);
    bool noticeStationChanged (const RadioStation &, int idx);
    bool noticeStationsChanged(const StationList &sl);
    bool noticePresetFileChanged(const QString &/*f*/)           { return false; }

    bool noticeRDSStateChanged      (bool  /*enabled*/)          { return false; }
    bool noticeRDSRadioTextChanged  (const QString &/*s*/)       { return false; }
    bool noticeRDSStationNameChanged(const QString &/*s*/)       { return false; }

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/) { return false; }
    bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID /*id*/) { return false; }

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
    void slotRecordingTemplateChanged(const QString &t);

    void slotNewAlarm();
    void slotDeleteAlarm();

    void slotOK();
    void slotCancel();
    void slotSetDirty();

protected:

    AlarmVector      alarms;
    QVector<QString> stationIDs;

    bool ignoreChanges;
    bool m_dirty;
};

#endif
