/***************************************************************************
                          shortcuts.h  -  description
                             -------------------
    begin                : Mon Feb 8 2009
    copyright            : (C) 2009 by Martin Witte
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

#ifndef KRADIO_SHORTCUTS_H
#define KRADIO_SHORTCUTS_H

#include <QtCore/QObject>
#include "timecontrol_interfaces.h"
#include "radio_interfaces.h"
#include "radiodevicepool_interfaces.h"
#include "frequencyradio_interfaces.h"
#include "soundstreamclient_interfaces.h"
#include "pluginbase.h"

#include <kactioncollection.h>
#include <kactioncategory.h>

class ShortcutsConfiguration;
class QTimer;


enum ShortcutID {
    SHORTCUT_DIGIT_0 = 1000,
    SHORTCUT_DIGIT_1,
    SHORTCUT_DIGIT_2,
    SHORTCUT_DIGIT_3,
    SHORTCUT_DIGIT_4,
    SHORTCUT_DIGIT_5,
    SHORTCUT_DIGIT_6,
    SHORTCUT_DIGIT_7,
    SHORTCUT_DIGIT_8,
    SHORTCUT_DIGIT_9,
    SHORTCUT_POWER_ON,
    SHORTCUT_POWER_OFF,
    SHORTCUT_PAUSE,
    SHORTCUT_RECORD_START,
    SHORTCUT_RECORD_STOP,
    SHORTCUT_VOLUME_INC,
    SHORTCUT_VOLUME_DEC,
    SHORTCUT_STATION_NEXT,
    SHORTCUT_STATION_PREV,
    SHORTCUT_FREQ_INC,
    SHORTCUT_FREQ_DEC,
    SHORTCUT_SEARCH_NEXT,
    SHORTCUT_SEARCH_PREV,
    SHORTCUT_SLEEP,
    SHORTCUT_APPLICATION_QUIT
};


class Shortcuts : public QObject,
                        public PluginBase,
                        public IRadioClient,
                        public ITimeControlClient,
                        public ISoundStreamClient,
                        public IRadioDevicePoolClient,
                        public IFrequencyRadioClient
{
Q_OBJECT
public:
    Shortcuts(const QString &instanceID, const QString &name);
    ~Shortcuts();

    virtual bool connectI (Interface *);
    virtual bool disconnectI (Interface *);

    virtual QString pluginClassName() const { return "Shortcuts"; }

//     virtual const QString &name() const { return PluginBase::name(); }
//     virtual       QString &name()       { return PluginBase::name(); }

    // PluginBase

public:
    virtual void   saveState    (      KConfigGroup &) const;
    virtual void   restoreState (const KConfigGroup &);

    virtual ConfigPageInfo  createConfigurationPage();
//     virtual AboutPageInfo   createAboutPage();

    // IRadioClient methods

RECEIVERS:
    bool noticePowerChanged(bool /*on*/)                          { return false; }
    bool noticeStationChanged (const RadioStation &, int /*idx*/) { return false; }
    bool noticeStationsChanged(const StationList &/*sl*/);
    bool noticePresetFileChanged(const QString &/*f*/)            { return false; }

    bool noticeRDSStateChanged      (bool  /*enabled*/)           { return false; }
    bool noticeRDSRadioTextChanged  (const QString &/*s*/)        { return false; }
    bool noticeRDSStationNameChanged(const QString &/*s*/)        { return false; }

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/)  { return false; }
    bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID /*id*/)  { return false; }

    // ITimeControlClient

RECEIVERS:
    bool noticeAlarmsChanged(const AlarmVector &)            { return false; }
    bool noticeAlarm(const Alarm &)                          { return false; }
    bool noticeNextAlarmChanged(const Alarm *)               { return false; }
    bool noticeCountdownStarted(const QDateTime &/*end*/)    { return false; }
    bool noticeCountdownStopped()                            { return false; }
    bool noticeCountdownZero()                               { return false; }
    bool noticeCountdownSecondsChanged(int /*n*/, bool /*resumeOnSuspend*/) { return false; }

    // IRadioDevicePoolClient

RECEIVERS:
    bool noticeActiveDeviceChanged(IRadioDevice *)           { return false; }
    bool noticeDevicesChanged(const QList<IRadioDevice*> &)  { return false; }
    bool noticeDeviceDescriptionChanged(const QString &)     { return false; }

    // IFrequencyRadioClient
RECEIVERS:
    bool noticeFrequencyChanged(float /*f*/, const FrequencyRadioStation */*s*/) { return false; }
    bool noticeMinMaxFrequencyChanged(float /*min*/, float /*max*/)              { return false; }
    bool noticeDeviceMinMaxFrequencyChanged(float/* min*/, float /*max*/)        { return false; }
    bool noticeScanStepChanged(float /*s*/)                                      { return false; }


protected slots:
    void slotKbdTimedOut();
    void slotActionTriggered(QAction *a);
    void slotStationTriggered(QAction *a);
    void slotConfigPageDestroyed(QObject *);

protected:
    void addAction(const QString &name, ShortcutID id, int key);
    void activateStation (int i);
    void updateShortcutsEditors();
    void updateShortcutsEditor(ShortcutsConfiguration *c);
    void noticePluginsChanged(const PluginList &);


    QTimer                 *m_kbdTimer;
    int                     m_addIndex;

    KActionCollection      *m_stdCollection;
    KActionCollection      *m_stationCollection;
    KActionCategory        *m_stdActions;
    KActionCategory        *m_stationActions;

    QList<ShortcutsConfiguration*>   m_ShortcutsEditors;


};



#endif
