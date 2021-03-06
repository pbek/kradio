/***************************************************************************
                       alsa-sound-configuration.cpp  -  description
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

#include <QCheckBox>
#include <QGroupBox>
#include <QLayout>
#include <QScrollArea>

#include <kurlrequester.h>

#include <kcombobox.h>
#include <klocalizedstring.h>

#include "alsa-mixer-element.h"
#include "alsa-sound-configuration.h"
#include "alsa-sound.h"
#include "alsa-sound-models.h"


AlsaSoundConfiguration::AlsaSoundConfiguration (QWidget *parent, AlsaSoundDevice *dev)
 : PluginConfigPageBase(parent),
   m_SoundDevice (dev),
   m_groupMixerFrame(NULL),
   m_groupMixerFrameLayout(NULL),
   m_dirty(true),
   m_ignore_updates(false)
{
    setupUi(this);

    m_playbackDeviceModel = new AlsaSoundPlaybackDeviceModel(m_comboPlaybackDevice);
    m_comboPlaybackDevice->setModel(m_playbackDeviceModel);
    m_playbackMixerModel = new AlsaSoundPlaybackMixerModel(m_comboPlaybackMixerCard);
    m_comboPlaybackMixerCard->setModel(m_playbackMixerModel);
    m_captureDeviceModel = new AlsaSoundCaptureDeviceModel(m_comboCaptureDevice);
    m_comboCaptureDevice->setModel(m_captureDeviceModel);
    m_captureMixerModel = new AlsaSoundCaptureMixerModel(m_comboCaptureMixerCard);
    m_comboCaptureMixerCard->setModel(m_captureMixerModel);

    QObject::connect(m_comboPlaybackDevice,         QOverload<int>::of(&QComboBox::activated),            this, &AlsaSoundConfiguration::slotSetDirty);
    QObject::connect(m_comboPlaybackMixerCard,      QOverload<int>::of(&QComboBox::activated),            this, &AlsaSoundConfiguration::slotSetDirty);
    QObject::connect(m_comboCaptureDevice,          QOverload<int>::of(&QComboBox::activated),            this, &AlsaSoundConfiguration::slotSetDirty);
    QObject::connect(m_comboCaptureMixerCard,       QOverload<int>::of(&QComboBox::activated),            this, &AlsaSoundConfiguration::slotSetDirty);
    QObject::connect(chkDisablePlayback,            &QCheckBox      ::toggled,                            this, &AlsaSoundConfiguration::slotSetDirty);
    QObject::connect(chkDisableCapture,             &QCheckBox      ::toggled,                            this, &AlsaSoundConfiguration::slotSetDirty);
    QObject::connect(cbEnableSoftVolume,            &QCheckBox      ::toggled,                            this, &AlsaSoundConfiguration::slotSetDirty);
    QObject::connect(sbSoftVolumeCorrection,        QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &AlsaSoundConfiguration::slotSetDirty);

    QObject::connect(editPlaybackBufferSize,        QOverload<int>::of(&QSpinBox ::valueChanged),         this, &AlsaSoundConfiguration::slotSetDirty);
    QObject::connect(editPlaybackBufferChunkSize,   QOverload<int>::of(&QSpinBox ::valueChanged),         this, &AlsaSoundConfiguration::slotSetDirty);
    QObject::connect(editCaptureBufferSize,         QOverload<int>::of(&QSpinBox ::valueChanged),         this, &AlsaSoundConfiguration::slotSetDirty);
    QObject::connect(editCaptureBufferChunkSize,    QOverload<int>::of(&QSpinBox ::valueChanged),         this, &AlsaSoundConfiguration::slotSetDirty);
    QObject::connect(chkNonBlockingPlayback,        &QCheckBox::toggled,                                  this, &AlsaSoundConfiguration::slotSetDirty);
    QObject::connect(chkNonBlockingCapture,         &QCheckBox::toggled,                                  this, &AlsaSoundConfiguration::slotSetDirty);

    QObject::connect(editPlaybackBufferSize,        &QSpinBox ::editingFinished,                          this, &AlsaSoundConfiguration::slotPlaybackBufferSizeChanged);
    QObject::connect(editCaptureBufferSize,         &QSpinBox ::editingFinished,                          this, &AlsaSoundConfiguration::slotCaptureBufferSizeChanged);

    QObject::connect(cbEnableCaptureFormatOverride, &QCheckBox::toggled,                                  this, &AlsaSoundConfiguration::slotSetDirty);
    QObject::connect(m_cbRate,                      QOverload<int>::of(&KComboBox::currentIndexChanged),  this, &AlsaSoundConfiguration::slotSetDirty);
    QObject::connect(m_cbBits,                      QOverload<int>::of(&KComboBox::currentIndexChanged),  this, &AlsaSoundConfiguration::slotSetDirty);
    QObject::connect(m_cbSign,                      QOverload<int>::of(&KComboBox::currentIndexChanged),  this, &AlsaSoundConfiguration::slotSetDirty);
    QObject::connect(m_cbEndianness,                QOverload<int>::of(&KComboBox::currentIndexChanged),  this, &AlsaSoundConfiguration::slotSetDirty);
    QObject::connect(m_cbChannels,                  QOverload<int>::of(&KComboBox::currentIndexChanged),  this, &AlsaSoundConfiguration::slotSetDirty);

    QObject::connect(editPlaybackWorkaroundSleep,   &QSpinBox  ::editingFinished,                         this, &AlsaSoundConfiguration::slotSetDirty);
    QObject::connect(editCaptureWorkaroundSleep,    &QSpinBox  ::editingFinished,                         this, &AlsaSoundConfiguration::slotSetDirty);

    QObject::connect(m_comboPlaybackDevice,         QOverload<int>::of(&QComboBox::activated),            this, &AlsaSoundConfiguration::slotPlaybackDeviceSelected);
    QObject::connect(m_comboPlaybackMixerCard,      QOverload<int>::of(&QComboBox::activated),            this, &AlsaSoundConfiguration::slotPlaybackMixerSelected);
    QObject::connect(m_comboCaptureDevice,          QOverload<int>::of(&QComboBox::activated),            this, &AlsaSoundConfiguration::slotCaptureDeviceSelected);
    QObject::connect(m_comboCaptureMixerCard,       QOverload<int>::of(&QComboBox::activated),            this, &AlsaSoundConfiguration::slotCaptureMixerSelected);

    slotCheckSoundDevices();

    slotCancel();

    // check every 10 seconds for changed sound devices
    m_soundDevicesCheckTimer.setInterval(10000);
    m_soundDevicesCheckTimer.setSingleShot(false);
    m_soundDevicesCheckTimer.start();
    QObject::connect(&m_soundDevicesCheckTimer, &QTimer::timeout, this, &AlsaSoundConfiguration::slotCheckSoundDevices);
}


AlsaSoundConfiguration::~AlsaSoundConfiguration ()
{
}


void AlsaSoundConfiguration::slotCheckSoundDevices()
{
    QString old_pb_device = m_comboPlaybackDevice   ->itemData(m_comboPlaybackDevice   ->currentIndex()).toString();
    QString old_pb_mixer  = m_comboPlaybackMixerCard->itemData(m_comboPlaybackMixerCard->currentIndex()).toString();
    QString old_ca_device = m_comboCaptureDevice    ->itemData(m_comboCaptureDevice    ->currentIndex()).toString();
    QString old_ca_mixer  = m_comboCaptureMixerCard ->itemData(m_comboCaptureMixerCard ->currentIndex()).toString();

    if (m_playbackDeviceModel->reload()) {
        int new_pb_idx = m_comboPlaybackDevice->findData(old_pb_device);
        if (new_pb_idx >= 0) {
            m_comboPlaybackDevice->setCurrentIndex(new_pb_idx);
        }
    }

    if (m_playbackMixerModel->reload()) {
        int new_pbm_idx = m_comboPlaybackMixerCard->findData(old_pb_mixer);
        if (new_pbm_idx >= 0) {
            m_comboPlaybackMixerCard->setCurrentIndex(new_pbm_idx);
        }
    }

    if (m_captureDeviceModel->reload()) {
        int new_ca_idx = m_comboCaptureDevice->findData(old_ca_device);
        if (new_ca_idx >= 0) {
            m_comboCaptureDevice->setCurrentIndex(new_ca_idx);
        }
    }

    if (m_captureMixerModel->reload()) {
        int new_cam_idx = m_comboCaptureMixerCard->findData(old_ca_mixer);
        if (new_cam_idx >= 0) {
            m_comboCaptureMixerCard->setCurrentIndex(new_cam_idx);
        }
    }
}

void AlsaSoundConfiguration::slotPlaybackDeviceSelected(int /*comboIdx*/)
{
    // nothing todo
}


void AlsaSoundConfiguration::slotPlaybackMixerSelected(int /*comboIdx*/)
{
    // nothing todo
}


void AlsaSoundConfiguration::slotCaptureDeviceSelected(int /*comboIdx*/)
{
}


void AlsaSoundConfiguration::slotCaptureMixerSelected (int comboIdx)
{
    const QString oldCaptureMixer = m_currentCaptureMixer;
    m_currentCaptureMixer = m_comboCaptureMixerCard->itemData(comboIdx).toString();
    if (oldCaptureMixer == m_currentCaptureMixer) {
        return;
    }

    saveCaptureMixerSettings();

    // get new mixer elements
    QStringList                     vol_list,  sw_list, all_list;
    QMap<QString, AlsaMixerElement> vol_ch2id, sw_ch2id;
    AlsaSoundDevice::getCaptureMixerChannels(m_currentCaptureMixer, NULL, vol_list, vol_ch2id, sw_list, sw_ch2id, &all_list, !chkDisablePlayback->isChecked());

    // free old mixer gui stuff
    qDeleteAll(m_MixerElements);
    m_MixerElements.clear();



    if (m_groupMixerFrame) {
        delete m_groupMixerFrame; // and hopefully all associated children
    }

    m_groupMixerFrame     = new QFrame(m_groupMixerScrollView);
    m_groupMixerFrame     ->setFrameShape(QFrame::NoFrame);
    m_groupMixerFrame     ->setFrameShadow(QFrame::Plain);
    m_groupMixerFrame     ->setBackgroundRole(QPalette::Base);
    m_groupMixerFrame     ->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_groupMixerFrame     ->show();
    m_groupMixerScrollView->setWidget(m_groupMixerFrame);

    m_groupMixerFrameLayout = new QGridLayout(m_groupMixerFrame);


    int rows = 1;
    int cols = (all_list.count()+rows-1)/rows;

    int idx = 0;
    for (QList<QString>::const_iterator it = all_list.constBegin(); it != all_list.constEnd(); ++it, ++idx) {
        QAlsaMixerElement *e = new QAlsaMixerElement(m_groupMixerFrame, *it,
                                                     sw_list.contains(*it), vol_list.contains(*it));
        e->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        e->show();
//         int row = idx / cols;
//         int col = idx % cols;
        m_groupMixerFrameLayout->addWidget(e, idx / cols, idx % cols);
//         IErrorLogClient::staticLogDebug(i18n("row = %1, col = %2", row, col));
        QObject::connect(e, &QAlsaMixerElement::sigDirty, this, &AlsaSoundConfiguration::slotSetDirty);
        m_MixerElements.insert(*it, e);
    }
    restoreCaptureMixerSettings();
    m_groupMixerFrame->adjustSize();
}


void AlsaSoundConfiguration::resetDirtyFlags()
{
    m_dirty = false;
    foreach (QAlsaMixerElement *e, m_MixerElements) {
        e->slotResetDirty();
    }
}

void AlsaSoundConfiguration::saveCaptureMixerSettings()
{
    for (QMap<QString, QAlsaMixerElement*>::iterator it = m_MixerElements.begin(); it != m_MixerElements.end(); ++it) {
        const QString     &name      = it.key();
        QString            mixerName = m_currentCaptureMixer;
        QString            id        = AlsaConfigMixerSetting::getIDString(mixerName, name);
        QAlsaMixerElement *e         = *it;
        float              vol       = e->getVolume();
        bool               use       = e->getOverride();
        bool               active    = e->getActive();
        e->slotResetDirty();
        m_MixerSettings[id] = AlsaConfigMixerSetting(mixerName, name, use, active, vol);
    }
}

void AlsaSoundConfiguration::restoreCaptureMixerSettings()
{
    for (QMap<QString, QAlsaMixerElement*>::iterator it = m_MixerElements.begin(); it != m_MixerElements.end(); ++it) {
        const QString     &name      = it.key();
        QString            mixerName = m_currentCaptureMixer;
        QString            id        = AlsaConfigMixerSetting::getIDString(mixerName, name);
        QAlsaMixerElement *e         = *it;

        if (m_MixerSettings.contains(id)) {
            const AlsaConfigMixerSetting &s = m_MixerSettings[id];
            e->setVolume(s.volume());
            e->setOverride(s.use());
            e->setActive(s.active());
            e->slotResetDirty();
        } else {
            if (name == "ADC") {
                e->setOverride(true);
                e->setActive(true);
                e->setVolume(1.0);
            }
            else if (name == "Digital") {
                e->setOverride(true);
                e->setActive(true);
                e->setVolume(1.0);
            }
            else if (name == "Wave") {
                e->setOverride(true);
                e->setActive(false);
                e->setVolume(0);
            }
            else if (name == "Capture") {
                e->setOverride(true);
                e->setActive(true);
                e->setVolume(0.01);
            }
            e->slotSetDirty();
        }
    }
}


void AlsaSoundConfiguration::slotOK()
{
    if (!m_dirty)
        return;

    if (m_SoundDevice) {
        SoundFormat sf;
        getCaptureSoundFormat(sf);
        m_SoundDevice->setCaptureFormatOverride(cbEnableCaptureFormatOverride->isChecked(), sf);

        m_SoundDevice->setBufferSizes(editPlaybackBufferSize     ->value() * 1024,
                                      editPlaybackBufferChunkSize->value() * 1024,
                                      editCaptureBufferSize      ->value() * 1024,
                                      editCaptureBufferChunkSize ->value() * 1024
                                     );
        m_SoundDevice->setNonBlockingFlags(chkNonBlockingPlayback->isChecked(), chkNonBlockingCapture->isChecked());
        m_SoundDevice->enablePlayback     (!chkDisablePlayback->isChecked());
        m_SoundDevice->enableCapture      (!chkDisableCapture ->isChecked());

        QString deviceName = m_comboPlaybackDevice->itemData(m_comboPlaybackDevice->currentIndex()).toString();
        m_SoundDevice->setPlaybackDevice(deviceName);

        QString mixerName = m_comboPlaybackMixerCard->itemData(m_comboPlaybackMixerCard->currentIndex()).toString();
        m_SoundDevice->setPlaybackMixer(mixerName);

        deviceName = m_comboCaptureDevice->itemData(m_comboCaptureDevice->currentIndex()).toString();
        m_SoundDevice->setCaptureDevice (deviceName);

        mixerName  = m_comboCaptureMixerCard->itemData(m_comboCaptureMixerCard->currentIndex()).toString();
        m_SoundDevice->setCaptureMixer (mixerName);

        saveCaptureMixerSettings();
        m_SoundDevice->setCaptureMixerSettings(m_MixerSettings);

        m_SoundDevice->setSoftPlaybackVolume(cbEnableSoftVolume->isChecked(), sbSoftVolumeCorrection->value());


        m_SoundDevice->setWorkaroundSleepTime(editPlaybackWorkaroundSleep->value(), editCaptureWorkaroundSleep->value());
    }
    resetDirtyFlags();
}


void AlsaSoundConfiguration::slotCancel()
{
    if (!m_dirty)
        return;
    m_ignore_updates = true;

    QString devName = m_SoundDevice ?  m_SoundDevice->getPlaybackDeviceName() : "default";
    int     idx     = m_comboPlaybackDevice->findData(devName);
    if (idx >= 0) {
        m_comboPlaybackDevice->setCurrentIndex(idx);
    }

    QString mixerName = m_SoundDevice ?  m_SoundDevice->getPlaybackMixerName() : "default";
    idx               = m_comboPlaybackMixerCard->findData(mixerName);
    if (idx >= 0) {
        m_comboPlaybackMixerCard->setCurrentIndex(idx);
    }

    devName         = m_SoundDevice ?  m_SoundDevice->getCaptureDeviceName() : "default";
    idx             = m_comboCaptureDevice->findData(devName);
    if (idx >= 0) {
        m_comboCaptureDevice->setCurrentIndex(idx);
    }

    mixerName       = m_SoundDevice ?  m_SoundDevice->getCaptureMixerName() : "default";
    idx             = m_comboCaptureMixerCard->findData(mixerName);
    if (idx >= 0) {
        m_comboCaptureMixerCard->setCurrentIndex(idx);
        slotCaptureMixerSelected(m_comboCaptureMixerCard->currentIndex());
    }

    editPlaybackBufferSize     ->setValue  (m_SoundDevice ?  m_SoundDevice->getPlaybackBufferSize()/1024 : 96);
    editCaptureBufferSize      ->setValue  (m_SoundDevice ?  m_SoundDevice->getCaptureBufferSize ()/1024 : 96);
    chkNonBlockingPlayback     ->setChecked(m_SoundDevice ?  m_SoundDevice->isNonBlockingPlayback() : false);
    chkNonBlockingCapture      ->setChecked(m_SoundDevice ?  m_SoundDevice->isNonBlockingPlayback() : false);
    chkDisablePlayback         ->setChecked(m_SoundDevice ? !m_SoundDevice->isPlaybackEnabled()     : false);
    chkDisableCapture          ->setChecked(m_SoundDevice ? !m_SoundDevice->isCaptureEnabled()      : false);

    if (m_SoundDevice)
        m_MixerSettings = m_SoundDevice->getCaptureMixerSettings();
    else
        m_MixerSettings.clear();
    restoreCaptureMixerSettings();

    double fact = 1.0;
    bool   en   = m_SoundDevice ? m_SoundDevice->getSoftPlaybackVolume(fact) : false;
    cbEnableSoftVolume    ->setChecked(en);
    sbSoftVolumeCorrection->setValue(fact);


    SoundFormat sf;
    bool        sf_ov_enable = false;
    if (m_SoundDevice) {
        sf_ov_enable = m_SoundDevice->getCaptureFormatOverride(sf);
    }
    setCaptureSoundFormat(sf);
    cbEnableCaptureFormatOverride->setChecked(sf_ov_enable);

    m_ignore_updates = false;

    slotPlaybackBufferSizeChanged();
    slotCaptureBufferSizeChanged ();
    editPlaybackBufferChunkSize->setValue  (m_SoundDevice ?  m_SoundDevice->getPlaybackChunkSize ()/1024 : 16);
    editCaptureBufferChunkSize ->setValue  (m_SoundDevice ?  m_SoundDevice->getCaptureChunkSize  ()/1024 : 16);

    editPlaybackWorkaroundSleep->setValue  (m_SoundDevice ? m_SoundDevice->getWorkaroundPlaybackSleepTime() : 0);
    editCaptureWorkaroundSleep ->setValue  (m_SoundDevice ? m_SoundDevice->getWorkaroundCaptureSleepTime()  : 0);
    resetDirtyFlags();
}


void AlsaSoundConfiguration::slotUpdateConfig()
{
    slotSetDirty();
    slotCancel();
}

void AlsaSoundConfiguration::slotSetDirty()
{
    if (!m_dirty && !m_ignore_updates) {
        m_dirty = true;
        //emit sigDirty();
    }
}





void AlsaSoundConfiguration::setCaptureSoundFormat(const SoundFormat &sf)
{
    m_ignore_updates = true;

    int idx_Rate      = RATE_44100_IDX;
    int idx_Bits      = BITS_16_IDX;
    int idx_Sign      = SIGN_SIGNED_IDX;
    int idx_Channels  = CHANNELS_STEREO_IDX;
    int idx_Endianness = ENDIAN_LITTLE_IDX;

    switch(sf.m_SampleRate) {
        case 48000 : idx_Rate = RATE_48000_IDX; break;
        case 44100 : idx_Rate = RATE_44100_IDX; break;
        case 32000 : idx_Rate = RATE_32000_IDX; break;
        case 22050 : idx_Rate = RATE_22050_IDX; break;
        case 11025 : idx_Rate = RATE_11025_IDX; break;
    }

    switch(sf.m_SampleBits) {
        case 8  : idx_Bits = BITS_8_IDX; break;
        case 16 : idx_Bits = BITS_16_IDX; break;
    }

    idx_Sign = sf.m_IsSigned ? SIGN_SIGNED_IDX : SIGN_UNSIGNED_IDX;

    switch(sf.m_Channels) {
        case 2: idx_Channels = CHANNELS_STEREO_IDX; break;
        case 1: idx_Channels = CHANNELS_MONO_IDX; break;
    }

    switch(sf.m_Endianness) {
        case LITTLE_ENDIAN: idx_Endianness = ENDIAN_LITTLE_IDX; break;
        case BIG_ENDIAN:    idx_Endianness = ENDIAN_BIG_IDX;    break;
    }

    m_cbRate      ->setCurrentIndex(idx_Rate);
    m_cbBits      ->setCurrentIndex(idx_Bits);
    m_cbSign      ->setCurrentIndex(idx_Sign);
    m_cbChannels  ->setCurrentIndex(idx_Channels);
    m_cbEndianness ->setCurrentIndex(idx_Endianness);

    m_ignore_updates = false;
}


void AlsaSoundConfiguration::getCaptureSoundFormat(SoundFormat &sf) const
{
    int idx_Rate      = m_cbRate      ->currentIndex();
    int idx_Bits      = m_cbBits      ->currentIndex();
    int idx_Sign      = m_cbSign      ->currentIndex();
    int idx_Channels  = m_cbChannels  ->currentIndex();
    int idx_Endianness = m_cbEndianness ->currentIndex();

    switch(idx_Rate) {
        case RATE_48000_IDX : sf.m_SampleRate = 48000; break;
        case RATE_44100_IDX : sf.m_SampleRate = 44100; break;
        case RATE_32000_IDX : sf.m_SampleRate = 32000; break;
        case RATE_22050_IDX : sf.m_SampleRate = 22050; break;
        case RATE_11025_IDX : sf.m_SampleRate = 11025; break;
        default             : sf.m_SampleRate = 44100; break;
    }

    switch(idx_Bits) {
        case BITS_8_IDX  : sf.m_SampleBits = 8;  break;
        case BITS_16_IDX : sf.m_SampleBits = 16; break;
        default          : sf.m_SampleBits = 16; break;
    }

    switch(idx_Sign) {
        case SIGN_SIGNED_IDX   : sf.m_IsSigned = true;  break;
        case SIGN_UNSIGNED_IDX : sf.m_IsSigned = false; break;
        default                : sf.m_IsSigned = true;  break;
    }

    switch(idx_Channels) {
        case CHANNELS_STEREO_IDX : sf.m_Channels = 2; break;
        case CHANNELS_MONO_IDX   : sf.m_Channels = 1; break;
        default                  : sf.m_Channels = 2; break;
    }

    switch(idx_Endianness) {
        case ENDIAN_LITTLE_IDX : sf.m_Endianness = LITTLE_ENDIAN; break;
        case ENDIAN_BIG_IDX    : sf.m_Endianness = BIG_ENDIAN;    break;
        default                : sf.m_Endianness = BYTE_ORDER;    break;
    }
}


QString AlsaSoundConfiguration::condenseALSADeviceDescription(const AlsaSoundDeviceMetaData &md)
{
    QString res = md.cardDescription();
    if (res.length()) res += ", ";
    res += md.deviceVerboseDescription();
    return res;
}


void AlsaSoundConfiguration::slotPlaybackBufferSizeChanged()
{
    if (!m_ignore_updates) {
        editPlaybackBufferChunkSize->setMaximum(editPlaybackBufferSize->value() / 3);
    }
}


void AlsaSoundConfiguration::slotCaptureBufferSizeChanged ()
{
    if (!m_ignore_updates) {
        editCaptureBufferChunkSize->setMaximum(editCaptureBufferSize->value() / 3);
    }
}


