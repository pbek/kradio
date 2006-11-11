/***************************************************************************
                          recording-monitor.h  -  description
                             -------------------
    begin                : Mo Sep 1 2003
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

#ifndef KRADIO_RECORDING_MONITOR_H
#define KRADIO_RECORDING_MONITOR_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qwidget.h>

#include "../../src/include/widgetplugins.h"
#include "../../src/include/soundstreamclient_interfaces.h"
//#include <kradio/interfaces/recording-interfaces.h>


class QLabel;
class QPushButton;
class QCheckBox;
class RecordingDataMonitor;
class KComboBox;

class RecordingMonitor : public QWidget,
                         public WidgetPluginBase,
                         public ISoundStreamClient
                         //public IRecordingClient
{
Q_OBJECT
public:

    RecordingMonitor(const QString &name);
    virtual ~RecordingMonitor();

    const QString &name() const { return PluginBase::name(); }
          QString &name()       { return PluginBase::name(); }

    virtual QString pluginClassName() const { return "RecordingMonitor"; }

    // WidgetPluginBase

public:
    virtual void   saveState (KConfig *) const;
    virtual void   restoreState (KConfig *);

    virtual bool   connectI(Interface *i);
    virtual bool   disconnectI(Interface *i);

    virtual ConfigPageInfo  createConfigurationPage();
    virtual AboutPageInfo   createAboutPage();

    // IRecordingClient

    void noticeConnectedI (ISoundStreamServer *s, bool pointer_valid);

    bool noticeSoundStreamCreated(SoundStreamID id);
    bool noticeSoundStreamClosed(SoundStreamID id);
    bool noticeSoundStreamChanged(SoundStreamID id);

    bool startRecordingWithFormat(SoundStreamID id, const SoundFormat &sf, SoundFormat &real_format);
    bool stopRecording(SoundStreamID id);

    bool noticeSoundStreamData(SoundStreamID id, const SoundFormat &sf, const char *data, size_t size, size_t &consumed_size, const SoundMetaData &md);

public slots:

    void    toggleShown() { WidgetPluginBase::pToggleShown(); }
    void    showOnOrgDesktop();
    void    show();
    void    hide();

    void    slotStartStopRecording();

    void    slotStreamSelected(int idx);

protected:

    virtual void updateRecordingButton();

    virtual void showEvent(QShowEvent *);
    virtual void hideEvent(QHideEvent *);

    const QWidget *getWidget() const { return this; }
          QWidget *getWidget()       { return this; }


protected:

    QLabel               *m_labelSize;
    QLabel               *m_labelTime;
    QLabel               *m_labelRate;
    QLabel               *m_labelFileName;
    QLabel               *m_labelStatus;
    QPushButton          *m_btnStartStop;

    KComboBox               *m_comboSoundStreamSelector;
    QMap<SoundStreamID, int> m_SoundStreamID2idx;
    QMap<int, SoundStreamID> m_idx2SoundStreamID;

    SoundStreamID            m_currentStream;
    RecordingDataMonitor    *m_dataMonitor;

    bool                     m_recording;
    QString                  m_defaultStreamDescription;
};




#endif
