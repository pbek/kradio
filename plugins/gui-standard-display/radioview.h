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
class QTabWidget;
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

    virtual QString pluginClassName() const { return QString::fromLatin1("RadioView"); }

    // WidgetPluginBase

public:
    virtual void   saveState    (      KConfigGroup &) const;
    virtual void   restoreState (const KConfigGroup &);
    virtual void   restoreState (const KConfigGroup &g, bool b) { WidgetPluginBase::restoreState(g, b); }


protected:
    virtual bool   connectI(Interface *i);
    virtual bool   disconnectI(Interface *i);

    virtual bool   setManager (PluginManager *);
    virtual void   unsetManager ();

    virtual void   noticeWidgetPluginShown(WidgetPluginBase *p, bool shown);

    virtual ConfigPageInfo  createConfigurationPage();

public slots:
    // connects destroy-msg with remove-function
    bool addElement    (RadioViewElement *);
    bool removeElement (QObject *);

protected:
    void selectTopWidgets();


    // IRadioClient

RECEIVERS:
    bool noticePowerChanged(bool on);
    bool noticeStationChanged (const RadioStation &, int idx);
    bool noticeStationsChanged(const StationList &sl);
    bool noticePresetFileChanged(const QString &/*f*/)           { return false; }

    bool noticeRDSStateChanged      (bool  /*enabled*/)          { return false; }
    bool noticeRDSRadioTextChanged  (const QString &/*s*/)       { return false; }
    bool noticeRDSStationNameChanged(const QString &/*s*/)       { return false; }

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID id);
    bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID id);

    // IRadioDevicePoolClient

RECEIVERS:
    bool noticeActiveDeviceChanged(IRadioDevice *rd);
    bool noticeDevicesChanged(const QList<IRadioDevice*> &)  { return false; }
    bool noticeDeviceDescriptionChanged(const QString &) { return false; }

    // ISoundStreamClient

RECEIVERS:
    void noticeConnectedI (ISoundStreamServer *s, bool pointer_valid);

    bool startRecordingWithFormat(SoundStreamID /*id*/,
                      const SoundFormat &/*proposed_format*/,
                      SoundFormat       &/*real_format*/,
                      const recordingTemplate_t     &/*filenameTemplate*/);
    bool stopRecording(SoundStreamID /*id*/);
    bool pausePlayback(SoundStreamID /*id*/);
    bool resumePlayback(SoundStreamID /*id*/);

    bool noticeSoundStreamChanged(SoundStreamID id);

    // ITimeControlClient

RECEIVERS:
    bool noticeAlarmsChanged(const AlarmVector &)     { return false; }
    bool noticeAlarm(const Alarm &)                   { return false; }
    bool noticeNextAlarmChanged(const Alarm *)        { return false; }
    bool noticeCountdownStarted(const QDateTime &end);
    bool noticeCountdownStopped();
    bool noticeCountdownZero();
    bool noticeCountdownSecondsChanged(int /*n*/, bool /*suspendOnSleep*/) { return false; }

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

    virtual void    toggleShown() { WidgetPluginBase::pToggleShown(); }
    virtual void    slotUpdateRecordingMenu();

public:
    virtual void     setVisible(bool v);

protected:
    INLINE_IMPL_DEF_noticeConnectedI(IErrorLogClient);
    INLINE_IMPL_DEF_noticeConnectedI(IRadioClient);
    INLINE_IMPL_DEF_noticeConnectedI(IRadioDevicePoolClient);
    INLINE_IMPL_DEF_noticeConnectedI(ITimeControlClient);

    virtual void showEvent(QShowEvent *);
    virtual void hideEvent(QHideEvent *);

    virtual void autoSetCaption();
    virtual void updatePauseMenuItem(bool run_query, bool known_pause_state);

    const QWidget *getWidget() const { return this; }
          QWidget *getWidget()       { return this; }

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
