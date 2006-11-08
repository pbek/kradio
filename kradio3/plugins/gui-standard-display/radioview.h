/***************************************************************************
                          radioview.h  -  description
                             -------------------
    begin                : Mit Mai 28 2003
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

#ifndef KRADIO_RADIOVIEW_H
#define KRADIO_RADIOVIEW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qobjectlist.h>

#include "../../src/interfaces/radio_interfaces.h"
#include "../../src/interfaces/radiodevicepool_interfaces.h"
#include "../../src/interfaces/soundstreamclient_interfaces.h"
#include "../../src/interfaces/timecontrol_interfaces.h"
#include "../../src/libkradio/widgetplugins.h"
#include "radioview_element.h"

class QWidgetStack;
class QToolButton;
class KComboBox;
class QTabWidget;
class KPopupMenu;



class RadioView : public QWidget,
                  public WidgetPluginBase,
                  public IRadioClient,
                  public IRadioDevicePoolClient,
                  public ISoundStreamClient,
                  public ITimeControlClient
{
Q_OBJECT
public:

    RadioView(const QString &name);
    virtual ~RadioView();

    virtual QString pluginClassName() const { return "RadioView"; }

    const QString &name() const { return PluginBase::name(); }
          QString &name()       { return PluginBase::name(); }

    // WidgetPluginBase

public:
    virtual void   saveState (KConfig *) const;
    virtual void   restoreState (KConfig *);

    virtual bool   connectI(Interface *i);
    virtual bool   disconnectI(Interface *i);

    virtual void   noticeWidgetPluginShown(WidgetPluginBase *p, bool shown);
    virtual void   noticePluginsChanged(const PluginList &);

    virtual ConfigPageInfo  createConfigurationPage();
    virtual AboutPageInfo   createAboutPage();

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

    bool noticeCurrentSoundStreamIDChanged(SoundStreamID id);

    // IRadioDevicePoolClient

RECEIVERS:
    bool noticeActiveDeviceChanged(IRadioDevice *rd);
    bool noticeDevicesChanged(const QPtrList<IRadioDevice> &)  { return false; }
    bool noticeDeviceDescriptionChanged(const QString &) { return false; }

    // ISoundStreamClient

RECEIVERS:
    void noticeConnectedI (ISoundStreamServer *s, bool pointer_valid);

    bool startRecordingWithFormat(SoundStreamID /*id*/,
                      const SoundFormat &/*proposed_format*/,
                      SoundFormat       &/*real_format*/);
    bool stopRecording(SoundStreamID /*id*/);

    bool noticeSoundStreamChanged(SoundStreamID id);

    // ITimeControlClient

RECEIVERS:
    bool noticeAlarmsChanged(const AlarmVector &)     { return false; }
    bool noticeAlarm(const Alarm &)                   { return false; }
    bool noticeNextAlarmChanged(const Alarm *)        { return false; }
    bool noticeCountdownStarted(const QDateTime &end);
    bool noticeCountdownStopped();
    bool noticeCountdownZero();
    bool noticeCountdownSecondsChanged(int)           { return false; }

protected slots:

    void slotPower (bool on);
    void slotPause();
    void slotConfigure (bool show);
    void slotRecord ();
    void slotSnooze (bool start);
    void slotSnooze (int time);
    void slotRecordingMenu(int i);
    void slotBtnPluginsClicked();
    void slotComboStationSelected(int);

    void slotConfigPageDeleted(QObject*);
    void slotElementConfigPageDeleted(QObject*);

public slots:

    void    toggleShown() { WidgetPluginBase::pToggleShown(); }
    void    showOnOrgDesktop();
    void    show();
    void    hide();

protected:
    virtual void showEvent(QShowEvent *);
    virtual void hideEvent(QHideEvent *);

    virtual void autoSetCaption();

    const QWidget *getWidget() const { return this; }
          QWidget *getWidget()       { return this; }

    void    addConfigurationTabFor(RadioViewElement *, QTabWidget *);
    void    addCommonConfigurationTab(QTabWidget *);

protected:
    bool                  enableToolbarFlag;

    QToolButton          *btnPower;
    QToolButton          *btnConfigure;
    QToolButton          *btnQuit;
    QToolButton          *btnRecording;
    QToolButton          *btnSnooze;
    QToolButton          *btnPlugins;
    KComboBox            *comboStations;

    struct ElementCfg
    {
        RadioViewElement *element;
        QObject          *cfg;
        ElementCfg()                                : element(NULL), cfg(NULL) {}
        ElementCfg(RadioViewElement *e, QObject *w) : element(e), cfg(w) {}
        ElementCfg(RadioViewElement *e)             : element(e), cfg(NULL) {}
        ElementCfg(QObject *w)                      : element(NULL), cfg(w) {}
        bool operator == (const ElementCfg &x) const;
    };

    typedef  QPtrList<RadioViewElement>         ElementList;
    typedef  QPtrListIterator<RadioViewElement> ElementListIterator;
    typedef  QValueList<ElementCfg>             ElementCfgList;
    typedef  QValueListIterator<ElementCfg>     ElementCfgListIterator;

    ElementList           elements;
    ElementCfgList        elementConfigPages;
    QObjectList           configPages;
    QWidgetStack *        widgetStacks[clsClassMAX];
    float                 maxUsability[clsClassMAX];

    IRadioDevice         *currentDevice;

    KPopupMenu                    *m_RecordingMenu;
    KPopupMenu                    *m_pauseMenu;
    KPopupMenu                    *m_SnoozeMenu;
    int                            m_NextRecordingMenuID;
    QMap<int, SoundStreamID>       m_MenuID2StreamID;
    QMap<SoundStreamID, int>       m_StreamID2MenuID;

    KPopupMenu                    *m_PluginMenu;
    QMap<WidgetPluginBase *, int>  m_Plugins2MenuID;
};




#endif
