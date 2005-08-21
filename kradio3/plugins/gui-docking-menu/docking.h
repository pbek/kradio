/***************************************************************************
                          docking.h  -  description
                             -------------------
    begin                : Mon Jan 14 2002
    copyright            : (C) 2001, 2002 by Frank Schwanz, Ernst Martin Witte
    email                : schwanz@fh-brandenburg.de, witte@kawo1.rwth-aachen.de
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

#include <ksystemtray.h>
#include <qpixmap.h>
#include <qptrdict.h>

#include "../../src/interfaces/timecontrol_interfaces.h"
#include "../../src/interfaces/radio_interfaces.h"
#include "../../src/interfaces/radiodevicepool_interfaces.h"
#include "../../src/interfaces/stationselection_interfaces.h"
#include "../../src/libkradio/plugins.h"
#include "../../src/interfaces/soundstreamclient_interfaces.h"

enum LeftClickAction { lcaShowHide = 0, lcaPowerOnOff = 1 };

class RadioDocking : public KSystemTray,
                     public PluginBase,
                     public IRadioClient,
                     public ITimeControlClient,
                     public IRadioDevicePoolClient,
                     public IStationSelection,
                     public ISoundStreamClient
{
Q_OBJECT
public:
    RadioDocking (const QString &name);
    virtual ~RadioDocking();

    virtual bool connectI (Interface *);
    virtual bool disconnectI (Interface *);

    virtual QString pluginClassName() const { return "RadioDocking"; }

    virtual const QString &name() const { return PluginBase::name(); }
    virtual       QString &name()       { return PluginBase::name(); }


    // PluginBase

public:
    virtual void   saveState (KConfig *) const;
    virtual void   restoreState (KConfig *);

    virtual ConfigPageInfo  createConfigurationPage();
    virtual AboutPageInfo   createAboutPage();


    // IStationSelection

RECEIVERS:
    bool setStationSelection(const QStringList &sl);

ANSWERS:
    const QStringList & getStationSelection () const { return m_stationIDs; }


    // IRadioDevicePoolClient

RECEIVERS:
    bool noticeActiveDeviceChanged(IRadioDevice *)  { return false; }
    bool noticeDevicesChanged(const QPtrList<IRadioDevice> &)  { return false; }
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

    bool noticeCurrentSoundStreamIDChanged(SoundStreamID /*id*/) { return false; }

    // ISoundStreamClient

RECEIVERS:
    void noticeConnectedI (ISoundStreamServer *s, bool pointer_valid);

    bool startRecordingWithFormat(SoundStreamID /*id*/,
                      const SoundFormat &/*proposed_format*/,
                      SoundFormat       &/*real_format*/);
    bool stopRecording(SoundStreamID /*id*/);

    bool noticeSoundStreamChanged(SoundStreamID id);


protected slots:

    void slotSeekFwd();
    void slotSeekBkwd();

    void slotPower();
    void slotPause();
    void slotSleepCountdown();
    void slotShowAbout();

    void slotMenuItemActivated(int id);
    void slotRecordingMenu(int i);

protected:
    void mousePressEvent( QMouseEvent *e );

    void buildContextMenu();
    void buildRecordingMenu();
    void buildStationList();

    void noticeWidgetPluginShown(WidgetPluginBase *, bool shown);
    void noticePluginsChanged(const PluginList &);

    void showEvent(QShowEvent *) {}  // do nothing, original implementation adds "Quit" menu item

    void ShowHideWidgetPlugins();

public:

    LeftClickAction getLeftClickAction() const { return m_leftClickAction; }
    void            setLeftClickAction(LeftClickAction action);

signals:
    void sigLeftClickActionChanged(LeftClickAction action);

protected:

    KPopupMenu  *m_menu;
    KPopupMenu  *m_pluginMenu;
    KPopupMenu  *m_recordingMenu;
    QStringList  m_stationIDs;

    // menu Item IDs
    int             m_titleID;
    int             m_alarmID;
    int             m_recordingID;
    int             m_powerID;
    int             m_pauseID;
    int             m_sleepID;
    int             m_seekfwID;
    int             m_seekbwID;
    QValueList<int> m_stationMenuIDs;

    QMap<WidgetPluginBase *, int>  m_widgetPluginIDs;

    int                            m_NextRecordingMenuID;
    QMap<int, SoundStreamID>       m_MenuID2StreamID;
    QMap<SoundStreamID, int>       m_StreamID2MenuID;

    LeftClickAction                m_leftClickAction;

    QMap<QString, bool>            m_widgetsShownCache;
};


#endif
