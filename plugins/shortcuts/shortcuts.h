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

#include <QObject>
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

    virtual bool connectI    (Interface *) override;
    virtual bool disconnectI (Interface *) override;

    virtual QString pluginClassName() const override { return QString::fromLatin1("Shortcuts"); }

    // PluginBase

public:
    virtual void   saveState    (      KConfigGroup &) const override;
    virtual void   restoreState (const KConfigGroup &)       override;

    virtual ConfigPageInfo  createConfigurationPage() override;

    // IRadioClient methods

RECEIVERS:
    bool noticePowerChanged(bool /*on*/)                          override { return false; }
    bool noticeStationChanged (const RadioStation &, int /*idx*/) override { return false; }
    bool noticeStationsChanged(const StationList &/*sl*/)         override;
    bool noticePresetFileChanged(const QUrl &/*f*/)               override { return false; }

    bool noticeRDSStateChanged      (bool  /*enabled*/)           override { return false; }
    bool noticeRDSRadioTextChanged  (const QString &/*s*/)        override { return false; }
    bool noticeRDSStationNameChanged(const QString &/*s*/)        override { return false; }

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/)  override { return false; }
    bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID /*id*/)  override { return false; }

    // ITimeControlClient

RECEIVERS:
    bool noticeAlarmsChanged(const AlarmVector &)            override { return false; }
    bool noticeAlarm(const Alarm &)                          override { return false; }
    bool noticeNextAlarmChanged(const Alarm *)               override { return false; }
    bool noticeCountdownStarted(const QDateTime &/*end*/)    override { return false; }
    bool noticeCountdownStopped()                            override { return false; }
    bool noticeCountdownZero()                               override { return false; }
    bool noticeCountdownSecondsChanged(int /*n*/, bool /*resumeOnSuspend*/) override { return false; }

    // IRadioDevicePoolClient

RECEIVERS:
    bool noticeActiveDeviceChanged(IRadioDevice *)           override { return false; }
    bool noticeDevicesChanged(const QList<IRadioDevice*> &)  override { return false; }
    bool noticeDeviceDescriptionChanged(const QString &)     override { return false; }

    // IFrequencyRadioClient
RECEIVERS:
    bool noticeFrequencyChanged(float /*f*/, const FrequencyRadioStation */*s*/) override { return false; }
    bool noticeMinMaxFrequencyChanged(float /*min*/, float /*max*/)              override { return false; }
    bool noticeDeviceMinMaxFrequencyChanged(float/* min*/, float /*max*/)        override { return false; }
    bool noticeScanStepChanged(float /*s*/)                                      override { return false; }


protected slots:
    void slotKbdTimedOut();
    void slotActionTriggered(QAction *a);
    void slotStationTriggered(QAction *a);
    void slotConfigPageDestroyed(QObject *);

protected:
    void addAction(const QString &name, ShortcutID id, const QKeySequence &keyseq);
    void activateStation (int i);
    void updateShortcutsEditors();
    void updateShortcutsEditor(ShortcutsConfiguration *c);
    void noticePluginsAdded(const PluginList &) override;


    QTimer                 *m_kbdTimer;
    int                     m_addIndex;

    KActionCollection      *m_stdCollection;
    KActionCollection      *m_stationCollection;
    KActionCategory        *m_stdActions;
    KActionCategory        *m_stationActions;

    QList<ShortcutsConfiguration*>   m_ShortcutsEditors;


}; // Shortcuts



#endif
