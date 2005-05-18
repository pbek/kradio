/***************************************************************************
                          v4lradio-configuration.h  -  description
                             -------------------
    begin                : Fre Jun 20 2003
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

#ifndef KRADIO_V4LRADIO_CONFIGURATION_H
#define KRADIO_V4LRADIO_CONFIGURATION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../../src/interfaces/radiodevice_interfaces.h"
#include "../../src/interfaces/soundstreamclient_interfaces.h"
#include "../../src/libkradio-gui/gui_list_helper.h"

#include "v4lradio-configuration-ui.h"
#include "v4lcfg_interfaces.h"

class V4LRadio;
class QWidget;

class V4LRadioConfiguration : public V4LRadioConfigurationUI,
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

    void noticeConnectedSoundClient(ISoundStreamClient::thisInterface *i, bool pointer_valid);
    void noticeDisconnectedSoundClient(ISoundStreamClient::thisInterface *i, bool pointer_valid);

// IV4LCfgClient

RECEIVERS:
    bool noticeRadioDeviceChanged(const QString &s);
    bool noticePlaybackMixerChanged(const QString &soundStreamClientID, int Channel);
    bool noticeCaptureMixerChanged (const QString &soundStreamClientID, int Channel);
    bool noticeDeviceVolumeChanged(float v);
    bool noticeCapabilitiesChanged(const V4LCaps &c);

// IRadioDeviceClient

RECEIVERS:
    bool noticePowerChanged   (bool /*on*/, const IRadioDevice */*sender = NULL*/)          { return false; }
    bool noticeStationChanged (const RadioStation &, const IRadioDevice */*sender = NULL*/) { return false; }
    bool noticeDescriptionChanged (const QString &, const IRadioDevice *sender = NULL);

    bool noticeCurrentSoundStreamIDChanged(SoundStreamID /*id*/, const IRadioDevice */*sender*/) { return false; }

// IFrequencyRadioClient

RECEIVERS:
    bool noticeFrequencyChanged(float f, const RadioStation *s);
    bool noticeMinMaxFrequencyChanged(float min, float max);
    bool noticeDeviceMinMaxFrequencyChanged(float min, float max);
    bool noticeScanStepChanged(float s);

// ISoundStreamClient

RECEIVERS:
    bool noticeTrebleChanged(SoundStreamID id, float t);
    bool noticeBassChanged(SoundStreamID id, float b);
    bool noticeBalanceChanged(SoundStreamID id, float b);
    bool noticeSignalMinQualityChanged(SoundStreamID id, float q);

    bool noticePlaybackChannelsChanged(const QString & /*client_id*/, const QMap<int, QString> &/*channels*/);
    bool noticeCaptureChannelsChanged (const QString & /*client_id*/, const QMap<int, QString> &/*channels*/);
    bool noticeSoundStreamCreated(SoundStreamID /*id*/);


protected:

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

/*    int     m_PlaybackMixerChannelMask,
            m_CaptureMixerChannelMask;*/
    bool    m_ignoreGUIChanges;

    int     m_myControlChange;
    float   m_orgTreble,
            m_orgBass,
            m_orgBalance,
            m_orgDeviceVolume;

    V4LCaps m_caps;

    typedef GUIListHelper<QComboBox, QString> StringListHelper;
    typedef GUIListHelper<QComboBox, int>     IntListHelper;

    StringListHelper  m_PlaybackMixerHelper,
                      m_CaptureMixerHelper;
    IntListHelper     m_PlaybackChannelHelper,
                      m_CaptureChannelHelper;
};

#endif
