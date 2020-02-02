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

    virtual QString pluginClassName() const override { return QString::fromLatin1("RecordingMonitor"); }

    // WidgetPluginBase

public:
    virtual void   saveState    (      KConfigGroup &) const override;
    virtual void   restoreState (const KConfigGroup &)       override;
    virtual void   restoreState (const KConfigGroup &g, bool b) override { WidgetPluginBase::restoreState(g, b); }

    virtual bool   connectI   (Interface *i) override;
    virtual bool   disconnectI(Interface *i) override;

    // ISoundStreamClient
RECEIVERS:
    void noticeConnectedI (ISoundStreamServer *s, bool pointer_valid) override;

    bool noticeSoundStreamCreated(SoundStreamID id) override;
    bool noticeSoundStreamClosed (SoundStreamID id) override;
    bool noticeSoundStreamChanged(SoundStreamID id) override;

    bool startRecordingWithFormat(SoundStreamID id, const SoundFormat &sf, SoundFormat &real_format, const recordingTemplate_t &templ) override;
    bool stopRecording(SoundStreamID id) override;

    bool noticeSoundStreamData(SoundStreamID id, const SoundFormat &sf, const char *data, size_t size, size_t &consumed_size, const SoundMetaData &md) override;

// IRadioClient

RECEIVERS:
    bool noticePowerChanged(bool on) override;
    bool noticeStationChanged (const RadioStation &, int /*idx*/) override { return false; }
    bool noticeStationsChanged(const StationList &/*sl*/)         override { return false; }
    bool noticePresetFileChanged(const QUrl &/*f*/)               override { return false; }

    bool noticeRDSStateChanged      (bool  /*enabled*/)           override { return false; }
    bool noticeRDSRadioTextChanged  (const QString &/*s*/)        override { return false; }
    bool noticeRDSStationNameChanged(const QString &/*s*/)        override { return false; }

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/) override { return false; }
    bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID /*id*/) override { return false; }

protected:
    INLINE_IMPL_DEF_noticeConnectedI(IErrorLogClient);
    INLINE_IMPL_DEF_noticeConnectedI(IRadioClient);

public slots:

    virtual void    slotStartStopRecording();

    virtual void    slotStreamSelected(int idx);

public:
    virtual void    setVisible(bool v) override;

protected:

    virtual void    updateRecordingButton();

    virtual void    showEvent(QShowEvent *) override;
    virtual void    hideEvent(QHideEvent *) override;

    const QWidget  *getWidget() const override { return this; }
          QWidget  *getWidget()       override { return this; }


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
