/***************************************************************************
                       alsa-sound-configuration.h  -  description
                             -------------------
    begin                : Thu Sep 30 2004
    copyright            : (C) 2004 by Martin Witte
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

#ifndef KRADIO_ALSA_SOUND_CONFIGURATION_H
#define KRADIO_ALSA_SOUND_CONFIGURATION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "alsa-sound-configuration-ui.h"
#include "alsa-sound.h"
#include "alsa-config-mixer-setting.h"

class QHBoxLayout;
class QGridLayout;
class QAlsaMixerElement;
class QScrollView;
class QFrame;

class AlsaSoundConfiguration : public AlsaSoundConfigurationUI
{
Q_OBJECT
public :
    AlsaSoundConfiguration (QWidget *parent, AlsaSoundDevice *);
    ~AlsaSoundConfiguration ();

protected slots:

    void slotOK();
    void slotCancel();

    void slotUpdateConfig();

    void slotPlaybackCardSelected(const QString &cardname);
    void slotCaptureCardSelected(const QString &cardname);

protected:
    int listSoundDevices(KComboBox *combobox, QMap<QString, int> *devname2dev, QMap<int, QString> *dev2devname, QMap<int, int> *dev2idx, int card, snd_pcm_stream_t stream);
    void saveCaptureMixerSettings();
    void restoreCaptureMixerSettings();

    AlsaSoundDevice   *m_SoundDevice;
    int                m_currentCaptureCard;
    QMap<QString, int> m_name2card,
                       m_name2capturedevice,
                       m_playbackDeviceName2dev,
                       m_captureDeviceName2dev;
    QMap<int, QString> m_card2name,
                       m_dev2playbackDeviceName,
                       m_dev2captureDeviceName;
    QMap<int, int>     m_captureCard2idx,
                       m_captureDevice2idx,
                       m_playbackCard2idx,
                       m_playbackDevice2idx;
    QGridLayout                       *m_groupMixerLayout;
    QScrollView                       *m_groupMixerScrollView;
    QFrame                            *m_groupMixerSubFrame;
    QMap<QString, QAlsaMixerElement*>  m_MixerElements;

    QMap<QString, AlsaConfigMixerSetting> m_MixerSettings;
};

#endif
