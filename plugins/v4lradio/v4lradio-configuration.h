/***************************************************************************
                          v4lradio-configuration.h  -  description
                             -------------------
    begin                : Fre Jun 20 2003
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

#ifndef KRADIO_V4LRADIO_CONFIGURATION_H
#define KRADIO_V4LRADIO_CONFIGURATION_H

#include <QtWidgets/QWidget>

#include "radiodevice_interfaces.h"
#include "frequencyradio_interfaces.h"
#include "soundstreamclient_interfaces.h"
#include "gui_list_helper.h"

#include "ui_v4lradio-configuration-ui.h"
#include "v4lcfg_interfaces.h"

class V4LRadio;
class QComboBox;

class V4LRadioConfiguration : public QWidget,
                              public Ui_V4LRadioConfigurationUI,
                              public IV4LCfgClient,
                              public IFrequencyRadioClient,
                              public ISoundStreamClient,
                              public IRadioDeviceClient
{
Q_OBJECT
public :
    V4LRadioConfiguration (QWidget *parent, SoundStreamID id);
    ~V4LRadioConfiguration ();

    bool connectI    (Interface *i) override;
    bool disconnectI (Interface *i) override;

    void noticeConnectedSoundClient   (ISoundStreamClient::thisInterface *i, bool pointer_valid) override;
    void noticeDisconnectedSoundClient(ISoundStreamClient::thisInterface *i, bool pointer_valid) override;

// IV4LCfgClient

RECEIVERS:
    void noticeConnectedI    (IV4LCfg *cfg, bool pointer_valid) override;
    void noticeDisconnectedI (IV4LCfg *cfg, bool pointer_valid) override;

    bool noticeRadioDeviceChanged(const QString &s) override;

    bool noticePlaybackMixerChanged(const QString &soundStreamClientID, const QString &Channel) override;
    bool noticeCaptureMixerChanged (const QString &soundStreamClientID, const QString &Channel) override;

    bool noticeDeviceVolumeChanged        (float          v)  override;
    bool noticeCapabilitiesChanged        (const V4LCaps &c)  override;
    bool noticeActivePlaybackChanged      (bool           a, bool muteCaptureChannelPlayback) override;
    bool noticeMuteOnPowerOffChanged      (bool           a)  override;
    bool noticeForceRDSEnabledChanged     (bool           a)  override;
    bool noticeDeviceProbeAtStartupChanged(bool           e)  override;
    bool noticeVolumeZeroOnPowerOffChanged(bool           a)  override;
    bool noticeV4LVersionOverrideChanged  (V4LVersion     vo) override;

// IRadioDeviceClient

RECEIVERS:
    bool noticePowerChanged         (bool  /*on*/,         const IRadioDevice */*sender*/)    override { return false; }
    bool noticeStationChanged       (const RadioStation &, const IRadioDevice */*sender*/)    override { return false; }
    bool noticeDescriptionChanged   (const QString &,      const IRadioDevice *sender = NULL) override;

    bool noticeRDSStateChanged      (bool  /*enabled*/,    const IRadioDevice */*sender*/) override { return false; }
    bool noticeRDSRadioTextChanged  (const QString &/*s*/, const IRadioDevice */*sender*/) override { return false; }
    bool noticeRDSStationNameChanged(const QString &/*s*/, const IRadioDevice */*sender*/) override { return false; }

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/, const IRadioDevice */*sender*/) override { return false; }
    bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID /*id*/, const IRadioDevice */*sender*/) override { return false; }

// IFrequencyRadioClient

RECEIVERS:
    bool noticeFrequencyChanged            (float f, const FrequencyRadioStation *s) override;
    bool noticeMinMaxFrequencyChanged      (float min, float max) override;
    bool noticeDeviceMinMaxFrequencyChanged(float min, float max) override;
    bool noticeScanStepChanged             (float s)              override;

// ISoundStreamClient

RECEIVERS:
    void noticeConnectedI (ISoundStreamServer *s, bool pointer_valid) override;

    bool noticeTrebleChanged          (SoundStreamID id, float t) override;
    bool noticeBassChanged            (SoundStreamID id, float b) override;
    bool noticeBalanceChanged         (SoundStreamID id, float b) override;
    bool noticeSignalMinQualityChanged(SoundStreamID id, float q) override;

    bool noticePlaybackChannelsChanged(const QString & /*client_id*/, const QStringList &/*channels*/) override;
    bool noticeCaptureChannelsChanged (const QString & /*client_id*/, const QStringList &/*channels*/) override;
    bool noticeSoundStreamCreated     (SoundStreamID /*id*/) override;


protected:

    INLINE_IMPL_DEF_noticeConnectedI   (IFrequencyRadioClient);
    INLINE_IMPL_DEF_noticeConnectedI   (IRadioDeviceClient);
    INLINE_IMPL_DEF_noticeDisconnectedI(IFrequencyRadioClient);
    INLINE_IMPL_DEF_noticeDisconnectedI(IRadioDeviceClient);
    INLINE_IMPL_DEF_noticeDisconnectedI(ISoundStreamClient);

    bool                             eventFilter(QObject *o, QEvent *e) override;
    void                             populateDeviceComboBox();

protected slots:

    void selectRadioDevice();
    void slotRadioDeviceIndexChanged(int idx);
    void slotEditRadioDeviceChanged();
    void slotComboPlaybackMixerSelected(int idx);
    void slotComboCaptureMixerSelected(int idx);

    void updatePlaybackMixerChannelAlternatives();
    void updateCaptureMixerChannelAlternatives();

    void slotOK();
    void slotCancel();

    void guiMinFrequencyChanged(int v);
    void guiMaxFrequencyChanged(int v);

    void slotDeviceVolumeChanged (double v); // for QDoubleSpinBox,  0.0..1.0
    void slotTrebleChanged (double t);       // for QDoubleSpinBox,  0.0..1.0
    void slotBassChanged   (double b);       // for QDoubleSpinBox,  0.0..1.0
    void slotBalanceChanged(double b);       // for QDoubleSpinBox, -1.0..1.0

    void slotDeviceVolumeChanged (int v);    // for slider, 0..65535
    void slotTrebleChanged (int t);          // for slider, 0..65535
    void slotBassChanged   (int b);          // for slider, 0..65535
    void slotBalanceChanged(int b);          // for slider, 0..65535
    void slotBalanceCenter ();

protected:

    SoundStreamID m_SoundStreamID;

    bool    m_ignoreGUIChanges;

    int     m_myControlChange;
    float   m_orgTreble,
            m_orgBass,
            m_orgBalance,
            m_orgDeviceVolume;

    V4LCaps m_caps;

    typedef GUIListHelper<QComboBox, QString>       StringListHelper;

    StringListHelper  m_PlaybackMixerHelper,
                      m_CaptureMixerHelper;
    StringListHelper  m_PlaybackChannelHelper,
                      m_CaptureChannelHelper;
};

#endif
