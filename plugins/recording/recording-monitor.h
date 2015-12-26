/***************************************************************************
                          recording-monitor.h  -  description
                             -------------------
    begin                : Mo Sep 1 2003
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

#ifndef KRADIO_RECORDING_MONITOR_H
#define KRADIO_RECORDING_MONITOR_H

#include <QWidget>

#include "widgetpluginbase.h"
#include "soundstreamclient_interfaces.h"
#include "radio_interfaces.h"


class QLabel;
class QPushButton;
class QCheckBox;
class RecordingDataMonitor;
class KComboBox;

class RecordingMonitor : public QWidget,
                         public WidgetPluginBase,
                         public ISoundStreamClient,
                         public IRadioClient
{
Q_OBJECT
public:

    RecordingMonitor(const QString &instanceID, const QString &name);
    virtual ~RecordingMonitor();

//     const QString &name() const { return PluginBase::name(); }
//           QString &name()       { return PluginBase::name(); }

    virtual QString pluginClassName() const { return "RecordingMonitor"; }

    // WidgetPluginBase

public:
    virtual void   saveState    (      KConfigGroup &) const;
    virtual void   restoreState (const KConfigGroup &);
    virtual void   restoreState (const KConfigGroup &g, bool b) { WidgetPluginBase::restoreState(g, b); }

    virtual bool   connectI(Interface *i);
    virtual bool   disconnectI(Interface *i);

    virtual ConfigPageInfo  createConfigurationPage();

    // ISoundStreamClient
RECEIVERS:
    void noticeConnectedI (ISoundStreamServer *s, bool pointer_valid);

    bool noticeSoundStreamCreated(SoundStreamID id);
    bool noticeSoundStreamClosed (SoundStreamID id);
    bool noticeSoundStreamChanged(SoundStreamID id);

    bool startRecordingWithFormat(SoundStreamID id, const SoundFormat &sf, SoundFormat &real_format, const recordingTemplate_t &templ);
    bool stopRecording(SoundStreamID id);

    bool noticeSoundStreamData(SoundStreamID id, const SoundFormat &sf, const char *data, size_t size, size_t &consumed_size, const SoundMetaData &md);

// IRadioClient

RECEIVERS:
    bool noticePowerChanged(bool on);
    bool noticeStationChanged (const RadioStation &, int /*idx*/){ return false; }
    bool noticeStationsChanged(const StationList &/*sl*/)        { return false; }
    bool noticePresetFileChanged(const QString &/*f*/)           { return false; }

    bool noticeRDSStateChanged      (bool  /*enabled*/)          { return false; }
    bool noticeRDSRadioTextChanged  (const QString &/*s*/)       { return false; }
    bool noticeRDSStationNameChanged(const QString &/*s*/)       { return false; }

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/) { return false; }
    bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID /*id*/) { return false; }

protected:
    INLINE_IMPL_DEF_noticeConnectedI(IErrorLogClient);
    INLINE_IMPL_DEF_noticeConnectedI(IRadioClient);

public slots:

    virtual void    toggleShown() { WidgetPluginBase::pToggleShown(); }

    virtual void    slotStartStopRecording();

    virtual void    slotStreamSelected(int idx);

public:
    virtual void    setVisible(bool v);

protected:

    virtual void    updateRecordingButton();

    virtual void    showEvent(QShowEvent *);
    virtual void    hideEvent(QHideEvent *);

    const QWidget  *getWidget() const { return this; }
          QWidget  *getWidget()       { return this; }


protected:

    QLabel               *m_labelHdrSoundStream;
//     QLabel               *m_labelHdrStatus;
    QLabel               *m_labelHdrFilename;
    QLabel               *m_labelHdrSize;
    QLabel               *m_labelHdrRate;
    QLabel               *m_labelHdrTime;

    QLabel               *m_labelSize;
    QLabel               *m_labelTime;
    QLabel               *m_labelRate;
    QLabel               *m_labelFileName;
//     QLabel               *m_labelStatus;
    QPushButton          *m_btnStartStop;

    KComboBox               *m_comboSoundStreamSelector;
    QMap<SoundStreamID, int> m_SoundStreamID2idx;
    QMap<int, SoundStreamID> m_idx2SoundStreamID;

    SoundStreamID            m_currentStream;
    RecordingDataMonitor    *m_dataMonitor;

    bool                     m_recording;
    QString                  m_defaultStreamDescription;
};

KAboutData aboutDataRecordingMonitor();



#endif
