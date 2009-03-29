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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QtGui/QWidget>

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

    bool connectI (Interface *i);
    bool disconnectI (Interface *i);

    void noticeConnectedSoundClient   (ISoundStreamClient::thisInterface *i, bool pointer_valid);
    void noticeDisconnectedSoundClient(ISoundStreamClient::thisInterface *i, bool pointer_valid);

// IV4LCfgClient

RECEIVERS:
    bool noticeRadioDeviceChanged(const QString &s);
    bool noticePlaybackMixerChanged(const QString &soundStreamClientID, const QString &Channel);
    bool noticeCaptureMixerChanged (const QString &soundStreamClientID, const QString &Channel);
    bool noticeDeviceVolumeChanged(float v);
    bool noticeCapabilitiesChanged(const V4LCaps &c);
    bool noticeActivePlaybackChanged(bool a, bool muteCaptureChannelPlayback);
    bool noticeMuteOnPowerOffChanged(bool a);
    bool noticeForceRDSEnabledChanged(bool a);
    bool noticeVolumeZeroOnPowerOffChanged(bool a);
    bool noticeV4LVersionOverrideChanged(V4LVersion vo);

// IRadioDeviceClient

RECEIVERS:
    bool noticePowerChanged         (bool  /*on*/,         const IRadioDevice */*sender*/) { return false; }
    bool noticeStationChanged       (const RadioStation &, const IRadioDevice */*sender*/) { return false; }
    bool noticeDescriptionChanged   (const QString &,      const IRadioDevice *sender = NULL);

    bool noticeRDSStateChanged      (bool  /*enabled*/,    const IRadioDevice */*sender*/) { return false; }
    bool noticeRDSRadioTextChanged  (const QString &/*s*/, const IRadioDevice */*sender*/) { return false; }
    bool noticeRDSStationNameChanged(const QString &/*s*/, const IRadioDevice */*sender*/) { return false; }

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/, const IRadioDevice */*sender*/) { return false; }
    bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID /*id*/, const IRadioDevice */*sender*/) { return false; }

// IFrequencyRadioClient

RECEIVERS:
    bool noticeFrequencyChanged(float f, const FrequencyRadioStation *s);
    bool noticeMinMaxFrequencyChanged(float min, float max);
    bool noticeDeviceMinMaxFrequencyChanged(float min, float max);
    bool noticeScanStepChanged(float s);

// ISoundStreamClient

RECEIVERS:
    void noticeConnectedI (ISoundStreamServer *s, bool pointer_valid);

    bool noticeTrebleChanged          (SoundStreamID id, float t);
    bool noticeBassChanged            (SoundStreamID id, float b);
    bool noticeBalanceChanged         (SoundStreamID id, float b);
    bool noticeSignalMinQualityChanged(SoundStreamID id, float q);

    bool noticePlaybackChannelsChanged(const QString & /*client_id*/, const QStringList &/*channels*/);
    bool noticeCaptureChannelsChanged (const QString & /*client_id*/, const QStringList &/*channels*/);
    bool noticeSoundStreamCreated     (SoundStreamID /*id*/);


protected:

    INLINE_IMPL_DEF_noticeConnectedI(IV4LCfgClient);
    INLINE_IMPL_DEF_noticeConnectedI(IFrequencyRadioClient);
    INLINE_IMPL_DEF_noticeConnectedI(IRadioDeviceClient);

    bool eventFilter(QObject *o, QEvent *e);

protected slots:

    void selectRadioDevice();
    void slotEditRadioDeviceChanged();
    void slotComboPlaybackMixerSelected(int idx);
    void slotComboCaptureMixerSelected(int idx);

    void slotOK();
    void slotCancel();

    void guiMinFrequencyChanged(int v);
    void guiMaxFrequencyChanged(int v);

    void slotDeviceVolumeChanged (double v); // for KDoubleNumInput,  0.0..1.0
    void slotTrebleChanged (double t);       // for KDoubleNumInput,  0.0..1.0
    void slotBassChanged   (double b);       // for KDoubleNumInput,  0.0..1.0
    void slotBalanceChanged(double b);       // for KDoubleNumInput, -1.0..1.0

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
    typedef GUISimpleListHelper<QComboBox>          ChannelListHelper;

    StringListHelper  m_PlaybackMixerHelper,
                      m_CaptureMixerHelper;
    ChannelListHelper m_PlaybackChannelHelper,
                      m_CaptureChannelHelper;
};

#endif
