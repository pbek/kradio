/***************************************************************************
                          docking.h  -  description
                             -------------------
    begin                : Mon Jan 14 2002
    copyright            : (C) 2001, 2002 by Frank Schwanz, Ernst Martin Witte
    email                : schwanz@fh-brandenburg.de, emw-kradio@nocabal.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRADIO_DOCKING_H
#define KRADIO_DOCKING_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ksystemtrayicon.h>
#include <kmenu.h>
#include <khelpmenu.h>


#include "timecontrol_interfaces.h"
#include "radio_interfaces.h"
#include "radiodevicepool_interfaces.h"
#include "stationselection_interfaces.h"
#include "pluginbase.h"
#include "soundstreamclient_interfaces.h"
#include <Qt3Support/Q3DragObject>
#include <QtCore/QTimer>

enum LeftClickAction { lcaShowHide = 0, lcaPowerOnOff = 1 };

class RadioDocking : public KSystemTrayIcon,
                     public PluginBase,
                     public IRadioClient,
                     public ITimeControlClient,
                     public IRadioDevicePoolClient,
                     public IStationSelection,
                     public ISoundStreamClient
{
Q_OBJECT
public:
    RadioDocking (const QString &instanceID, const QString &name);
    virtual ~RadioDocking();

    virtual bool connectI (Interface *);
    virtual bool disconnectI (Interface *);

    virtual QString pluginClassName() const { return "RadioDocking"; }

//     virtual const QString &name() const { return PluginBase::name(); }
//     virtual       QString &name()       { return PluginBase::name(); }


    // PluginBase

public:
    virtual void   saveState   (      KConfigGroup &) const;
    virtual void   restoreState(const KConfigGroup &);

    virtual ConfigPageInfo  createConfigurationPage();
//     virtual AboutPageInfo   createAboutPage();


    // IStationSelection

RECEIVERS:
    bool setStationSelection(const QStringList &sl);

ANSWERS:
    const QStringList & getStationSelection () const { return m_stationIDs; }


    // IRadioDevicePoolClient

RECEIVERS:
    bool noticeActiveDeviceChanged(IRadioDevice *)  { return false; }
    bool noticeDevicesChanged(const QList<IRadioDevice*> &)  { return false; }
    bool noticeDeviceDescriptionChanged(const QString &) { return false; }

    // ITimeControlClient

RECEIVERS:
    bool noticeAlarmsChanged(const AlarmVector &)   { return false; }
    bool noticeAlarm(const Alarm &)                 { return false; }
    bool noticeNextAlarmChanged(const Alarm *);
    bool noticeCountdownStarted(const QDateTime &/*end*/);
    bool noticeCountdownStopped();
    bool noticeCountdownZero();
    bool noticeCountdownSecondsChanged(int n);


    // IRadioClient

RECEIVERS:
    bool noticePowerChanged(bool on);
    bool noticeStationChanged (const RadioStation &, int idx);
    bool noticeStationsChanged(const StationList &sl);
    bool noticePresetFileChanged(const QString &/*f*/)           { return false; }

    bool noticeRDSStateChanged      (bool  /*enabled*/)          { return false; }
    bool noticeRDSRadioTextChanged  (const QString &/*s*/);
    bool noticeRDSStationNameChanged(const QString &/*s*/)       { return false; }

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/) { return false; }
    bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID /*id*/) { return false; }

    // ISoundStreamClient

RECEIVERS:
    void noticeConnectedI (ISoundStreamServer *s, bool pointer_valid);

    bool startRecordingWithFormat(SoundStreamID      /*id*/,
                                  const SoundFormat &/*proposed_format*/,
                                  SoundFormat       &/*real_format*/);
    bool stopRecording (SoundStreamID /*id*/);
    bool pausePlayback (SoundStreamID /*id*/);
    bool resumePlayback(SoundStreamID /*id*/);

    bool noticeSoundStreamChanged(SoundStreamID id);

protected:

    INLINE_IMPL_DEF_noticeConnectedI(IErrorLogClient);
    INLINE_IMPL_DEF_noticeConnectedI(IRadioClient);
    INLINE_IMPL_DEF_noticeConnectedI(ITimeControlClient);
    INLINE_IMPL_DEF_noticeConnectedI(IRadioDevicePoolClient);
    INLINE_IMPL_DEF_noticeConnectedI(IStationSelection);

protected slots:

    void slotSeekFwd();
    void slotSeekBkwd();

    void slotPower();
    void slotPause();
    void slotSleepCountdown();

    void slotMenuItemActivated(QAction *a);
    void slotRecordingMenu(QAction *a);
    void slotStartDefaultRecording();
    void slotActivated (QSystemTrayIcon::ActivationReason reason);

protected slots:

    void    buildContextMenu();
    void    slotUpdateRecordingMenu();

protected:
    void    updateTrayIcon(bool run_query_rec, bool run_query_pause, bool known_rec_state, bool known_pause_state);
    void    updatePauseMenuItem(bool run_query, bool known_pause_state);
    void    buildRecordingMenu();
    void    buildStationList();
    QString generateStationTitle() const;
    QString generateAlarmTitle  () const;

    void    ShowHideWidgetPlugins();

#warning "FIXME: port to QT4 drag and drop"
    void    dragEnterEvent(QDragEnterEvent* event);
    void    dropEvent(QDropEvent* event);

public:

    LeftClickAction getLeftClickAction() const { return m_leftClickAction; }
    void            setLeftClickAction(LeftClickAction action);

signals:
    void sigLeftClickActionChanged(LeftClickAction action);

protected:

    KMenu       *m_menu;
    KMenu       *m_recordingMenu;
    KHelpMenu    m_helpMenu;
    QStringList  m_stationIDs;

    // menu Item IDs
    QAction         *m_quitID;
    QAction         *m_titleID;
    QAction         *m_alarmID;
    QAction         *m_recordingID;
    QAction         *m_recordingMenuAction;
    QAction         *m_powerID;
    QAction         *m_pauseID;
    QAction         *m_sleepID;
    QAction         *m_seekfwID;
    QAction         *m_seekbwID;
    QMap<QString, QAction*> m_stationMenuIDs;
    QActionGroup    *m_stationsActionGroup;

    QMap<SoundStreamID, QAction*>  m_StreamID2MenuID;

    LeftClickAction                m_leftClickAction;

    QTimer                         m_menuRebuildWorkaroundTimer;
    bool                           m_inMenuAction;
    bool                           m_scheduleMenuRebuild;


    bool    m_paused;
    bool    m_recording;
    bool    m_playing;

    QList<QAction*>                m_WorkaroundRecordingMenuActionsToBeDeleted;
    QTimer                         m_WorkaroundRecordingMenuUpdate;
};


#endif