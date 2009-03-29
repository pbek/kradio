/***************************************************************************
                       alsa-sound-configuration.h  -  description
                             -------------------
    begin                : Thu Sep 30 2004
    copyright            : (C) 2004 by Martin Witte
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

#ifndef KRADIO_ALSA_SOUND_CONFIGURATION_H
#define KRADIO_ALSA_SOUND_CONFIGURATION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QtGui/QWidget>

#include "ui_alsa-sound-configuration-ui.h"
#include "alsa-sound.h"
#include "alsa-config-mixer-setting.h"


#define RATE_48000_IDX      0
#define RATE_44100_IDX      1
#define RATE_32000_IDX      2
#define RATE_22050_IDX      3
#define RATE_11025_IDX      4

#define CHANNELS_STEREO_IDX 0
#define CHANNELS_MONO_IDX   1

#define SIGN_SIGNED_IDX     0
#define SIGN_UNSIGNED_IDX   1

#define BITS_16_IDX         0
#define BITS_8_IDX          1

#define ENDIAN_LITTLE_IDX   0
#define ENDIAN_BIG_IDX      1



class QHBoxLayout;
class QGridLayout;
class QAlsaMixerElement;
class QScrollArea;
class QFrame;

class AlsaSoundConfiguration : public QWidget,
                               public Ui_AlsaSoundConfigurationUI
{
Q_OBJECT
public :
    AlsaSoundConfiguration (QWidget *parent, AlsaSoundDevice *);
    ~AlsaSoundConfiguration ();

protected slots:

    void slotOK();
    void slotCancel();

    void slotSetDirty();

    void slotUpdateConfig();

    void slotPlaybackCardSelected(const QString &cardname);
    void slotCaptureCardSelected(const QString &cardname);
    void slotCheckSoundCards();

protected:
    int listSoundDevices(KComboBox *combobox, QMap<QString, int> *devname2dev, QMap<int, QString> *dev2devname, QMap<int, int> *dev2idx, int card, snd_pcm_stream_t stream);
    void saveCaptureMixerSettings();
    void restoreCaptureMixerSettings();

    void getCaptureSoundFormat(SoundFormat &sf) const;
    void setCaptureSoundFormat(const SoundFormat &sf);

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
    QFrame                            *m_groupMixerFrame;
    QGridLayout                       *m_groupMixerFrameLayout;
    QScrollArea                       *m_groupMixerScrollView;
    QMap<QString, QAlsaMixerElement*>  m_MixerElements;

    QMap<QString, AlsaConfigMixerSetting> m_MixerSettings;

    bool              m_dirty;
    bool              m_ignore_updates;

    QTimer            m_soundCardsCheckTimer;
};

#endif