/***************************************************************************
                          radioview.h  -  description
                             -------------------
    begin                : Mit Mai 28 2003
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

#ifndef KRADIO_RADIOVIEW_H
#define KRADIO_RADIOVIEW_H

#include <QWidget>
#include <QTimer>
#include <khelpmenu.h>

#include "radio_interfaces.h"
#include "radiodevicepool_interfaces.h"
#include "soundstreamclient_interfaces.h"
#include "timecontrol_interfaces.h"
#include "widgetpluginbase.h"
#include "radioview_element.h"

class KComboBox;

class QStackedWidget;
class QToolButton;
class RadioViewConfiguration;

class RadioView : public QWidget,
                  public WidgetPluginBase,
                  public IRadioClient,
                  public IRadioDevicePoolClient,
                  public ISoundStreamClient,
                  public ITimeControlClient
{
Q_OBJECT
public:

    RadioView(const QString &instanceID, const QString &name);
    virtual ~RadioView();

    virtual QString pluginClassName() const override { return QString::fromLatin1("RadioView"); }

    // WidgetPluginBase

public:
    virtual void   saveState    (      KConfigGroup &) const override;
    virtual void   restoreState (const KConfigGroup &)       override;
    virtual void   restoreState (const KConfigGroup &g, bool b) override { WidgetPluginBase::restoreState(g, b); }


protected:
    virtual bool   connectI   (Interface *i)  override;
    virtual bool   disconnectI(Interface *i)  override;

    virtual bool   setManager (PluginManager *) override;
    virtual void   unsetManager () override;

    virtual void   noticeWidgetPluginShown(WidgetPluginBase *p, bool shown) override;

    virtual ConfigPageInfo  createConfigurationPage() override;

public slots:
    // connects destroy-msg with remove-function
    bool addElement    (RadioViewElement *);
    bool removeElement (QObject *);

protected:
    void selectTopWidgets();


    // IRadioClient

RECEIVERS:
    bool noticePowerChanged(bool on) override;
    bool noticeStationChanged (const RadioStation &, int idx) override;
    bool noticeStationsChanged(const StationList &sl) override;
    bool noticePresetFileChanged(const QString &/*f*/)           override { return false; }

    bool noticeRDSStateChanged      (bool  /*enabled*/)          override { return false; }
    bool noticeRDSRadioTextChanged  (const QString &/*s*/)       override { return false; }
    bool noticeRDSStationNameChanged(const QString &/*s*/)       override { return false; }

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID id) override;
    bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID id) override;

    // IRadioDevicePoolClient

RECEIVERS:
    bool noticeActiveDeviceChanged     (IRadioDevice *rd)             override;
    bool noticeDevicesChanged          (const QList<IRadioDevice*> &) override { return false; }
    bool noticeDeviceDescriptionChanged(const QString &)              override { return false; }

    // ISoundStreamClient

RECEIVERS:
    void noticeConnectedI (ISoundStreamServer *s, bool pointer_valid) override;

    bool startRecordingWithFormat(SoundStreamID /*id*/,
                      const SoundFormat &/*proposed_format*/,
                      SoundFormat       &/*real_format*/,
                      const recordingTemplate_t     &/*filenameTemplate*/) override;
    bool stopRecording (SoundStreamID /*id*/) override;
    bool pausePlayback (SoundStreamID /*id*/) override;
    bool resumePlayback(SoundStreamID /*id*/) override;

    bool noticeSoundStreamChanged(SoundStreamID id) override;

    // ITimeControlClient

RECEIVERS:
    bool noticeAlarmsChanged(const AlarmVector &)     override { return false; }
    bool noticeAlarm(const Alarm &)                   override { return false; }
    bool noticeNextAlarmChanged(const Alarm *)        override { return false; }
    bool noticeCountdownStarted(const QDateTime &end) override;
    bool noticeCountdownStopped() override;
    bool noticeCountdownZero()    override;
    bool noticeCountdownSecondsChanged(int /*n*/, bool /*suspendOnSleep*/) override { return false; }

protected slots:

    void slotPower (bool on);
    void slotPause();
    void slotConfigure (bool show);
    void slotRecord ();
    void slotSnooze (bool start);
    void slotSnooze (QAction *a);
    void slotStartDefaultRecording();
    void slotRecordingMenu(QAction *a);
    void slotComboStationSelected(int);

    void slotConfigPageDeleted(QObject*);
    void slotElementConfigPageDeleted(QObject*);

public slots:

    virtual void    toggleShown() override { WidgetPluginBase::pToggleShown(); }
    virtual void    slotUpdateRecordingMenu();

public:
    virtual void     setVisible(bool v) override;

protected:
    INLINE_IMPL_DEF_noticeConnectedI(IErrorLogClient);
    INLINE_IMPL_DEF_noticeConnectedI(IRadioClient);
    INLINE_IMPL_DEF_noticeConnectedI(IRadioDevicePoolClient);
    INLINE_IMPL_DEF_noticeConnectedI(ITimeControlClient);

    virtual void showEvent(QShowEvent *) override;
    virtual void hideEvent(QHideEvent *) override;

    virtual void autoSetCaption();
    virtual void updatePauseMenuItem(bool run_query, bool known_pause_state);

    const QWidget *getWidget() const override { return this; }
          QWidget *getWidget()       override { return this; }

    void    addConfigurationTabFor(RadioViewElement *, RadioViewConfiguration *);
    void    addCommonConfigurationTab(RadioViewConfiguration *);

protected:
    bool                  enableToolbarFlag;

    QToolButton          *btnPower;
    QToolButton          *btnConfigure;
    QToolButton          *btnQuit;
    QToolButton          *btnRecording;
    QToolButton          *btnSnooze;
    QToolButton          *btnPlugins;
    QToolButton          *btnHelp;
    KComboBox            *comboStations;


    RadioViewConfiguration        *m_ConfigPage;

    QMap<QObject*, QObject*>       m_elementConfigPages;

    QStackedWidget                *widgetStacks[clsClassMAX];
    float                          maxUsability[clsClassMAX];

    IRadioDevice                  *currentDevice;

    QMenu                         *m_RecordingMenu;
    QMenu                         *m_pauseMenu;
    QMenu                         *m_SnoozeMenu;
    QMap<SoundStreamID, QAction*>  m_StreamID2MenuID;

    QAction                       *m_recordingDefaultMenuItem;
    QAction                       *m_pauseMenuItem;


    KHelpMenu                      m_helpMenu;

    QList<QAction*>                m_WorkaroundRecordingMenuActionsToBeDeleted;
    QTimer                         m_WorkaroundRecordingMenuUpdate;
};




#endif
