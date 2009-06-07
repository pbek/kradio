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

#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QLayout>
#include <QtGui/QScrollArea>

#include <kurlrequester.h>
#include <knuminput.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <ktabwidget.h>
#include <klocale.h>

#include "alsa-mixer-element.h"
#include "alsa-sound-configuration.h"
#include "alsa-sound.h"


AlsaSoundConfiguration::AlsaSoundConfiguration (QWidget *parent, AlsaSoundDevice *dev)
 : QWidget(parent),
   m_SoundDevice (dev),
   m_groupMixerFrame(NULL),
   m_groupMixerFrameLayout(NULL),
   m_groupMixerScrollView(NULL),
   m_dirty(true),
   m_ignore_updates(false)
{
    setupUi(this);

    QObject::connect(m_comboPlaybackCard,    SIGNAL(activated(int)),       this, SLOT(slotSetDirty()));
    QObject::connect(m_comboCaptureCard,     SIGNAL(activated(int)),       this, SLOT(slotSetDirty()));
    QObject::connect(m_comboPlaybackDevice,  SIGNAL(activated(int)),       this, SLOT(slotSetDirty()));
    QObject::connect(m_comboCaptureDevice,   SIGNAL(activated(int)),       this, SLOT(slotSetDirty()));
    QObject::connect(editBufferSize,         SIGNAL(valueChanged(int)),    this, SLOT(slotSetDirty()));
    QObject::connect(chkDisablePlayback,     SIGNAL(toggled(bool)),        this, SLOT(slotSetDirty()));
    QObject::connect(chkDisableCapture,      SIGNAL(toggled(bool)),        this, SLOT(slotSetDirty()));
    QObject::connect(cbEnableSoftVolume,     SIGNAL(toggled(bool)),        this, SLOT(slotSetDirty()));
    QObject::connect(sbSoftVolumeCorrection, SIGNAL(valueChanged(double)), this, SLOT(slotSetDirty()));

    QObject::connect(cbEnableCaptureFormatOverride, SIGNAL(toggled(bool)),            this, SLOT(slotSetDirty()));
    QObject::connect(m_cbRate,                      SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetDirty()));
    QObject::connect(m_cbBits,                      SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetDirty()));
    QObject::connect(m_cbSign,                      SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetDirty()));
    QObject::connect(m_cbEndianess,                 SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetDirty()));
    QObject::connect(m_cbChannels,                  SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetDirty()));



    QObject::connect(m_comboPlaybackCard, SIGNAL(activated(const QString &)),
                     this, SLOT(slotPlaybackCardSelected(const QString &)));
    QObject::connect(m_comboCaptureCard, SIGNAL(activated(const QString &)),
                     this, SLOT(slotCaptureCardSelected(const QString &)));

    if (!m_groupMixer->layout())
        new QGridLayout(m_groupMixer);

    QLayout *l = m_groupMixer->layout();

    m_groupMixerScrollView = new QScrollArea(m_groupMixer);
    m_groupMixerScrollView->setFrameShape(QFrame::StyledPanel);
    m_groupMixerScrollView->setFrameShadow(QFrame::Sunken);
    m_groupMixerScrollView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_groupMixerScrollView->setWidgetResizable(true);
    m_groupMixerScrollView->show();

    l->addWidget(m_groupMixerScrollView);

    slotCheckSoundCards();

    slotCancel();

    // check every 10 seconds for changed sound cards
    m_soundCardsCheckTimer.setInterval(10000);
    m_soundCardsCheckTimer.setSingleShot(false);
    m_soundCardsCheckTimer.start();
    QObject::connect(&m_soundCardsCheckTimer, SIGNAL(timeout()), this, SLOT(slotCheckSoundCards()));
}


AlsaSoundConfiguration::~AlsaSoundConfiguration ()
{
}


void AlsaSoundConfiguration::slotCheckSoundCards()
{
//     int     old_pb_idx = m_comboPlaybackCard->currentIndex();
//     int     old_ca_idx = m_comboCaptureCard ->currentIndex();
    QString old_pb_txt = m_comboPlaybackCard->currentText();
    QString old_ca_txt = m_comboCaptureCard ->currentText();

    m_name2card.clear();
    m_card2name.clear();
    m_comboPlaybackCard->clear();
    m_comboCaptureCard ->clear();
    m_playbackCard2idx.clear();
    m_captureCard2idx .clear();
    int card = -1;
    int ret  = 0;
    int idx_playback = 0;
    int idx_capture  = 0;
    while ((ret = snd_card_next(&card)) == 0) {
        char *name = NULL;
        if (card >= 0 && snd_card_get_longname(card, &name) == 0) {
            if (name) {
                m_name2card[name] = card;
                m_card2name[card] = name;
                if (listSoundDevices(NULL, NULL, NULL, NULL, card, SND_PCM_STREAM_PLAYBACK)) {
                    m_comboPlaybackCard->addItem(name);
                    m_playbackCard2idx[card] = idx_playback++;
                }
                if (listSoundDevices(NULL, NULL, NULL, NULL, card, SND_PCM_STREAM_CAPTURE)) {
                    m_comboCaptureCard->addItem(name);
                    m_captureCard2idx[card]  = idx_capture++;
                }
            }
        } else {
            break;
        }
    }

    int new_pb_idx = m_comboPlaybackCard->findText(old_pb_txt);
    if (new_pb_idx >= 0) {
        m_comboPlaybackCard->setCurrentIndex(new_pb_idx);
    } else if (new_pb_idx < 0) {
        slotPlaybackCardSelected(m_comboPlaybackCard->currentText());
        slotSetDirty();
    }
    int new_ca_idx = m_comboCaptureCard->findText(old_ca_txt);
    if (new_ca_idx >= 0) {
        m_comboCaptureCard->setCurrentIndex(new_ca_idx);
    } else if (new_ca_idx < 0) {
        slotCaptureCardSelected(m_comboCaptureCard->currentText());
        slotSetDirty();
    }
}

void AlsaSoundConfiguration::slotPlaybackCardSelected(const QString &cardname)
{
    if (!m_name2card.contains(cardname))
        return;

    listSoundDevices(m_comboPlaybackDevice, &m_playbackDeviceName2dev, &m_dev2playbackDeviceName, &m_playbackDevice2idx, m_name2card[cardname], SND_PCM_STREAM_PLAYBACK);
}


void AlsaSoundConfiguration::slotCaptureCardSelected(const QString &cardname)
{
    if (!m_name2card.contains(cardname))
        return;

    saveCaptureMixerSettings();

    listSoundDevices(m_comboCaptureDevice, &m_captureDeviceName2dev, &m_dev2captureDeviceName, &m_captureDevice2idx, m_name2card[cardname], SND_PCM_STREAM_CAPTURE);

    m_currentCaptureCard = m_name2card[cardname];

    QStringList                     vol_list,  sw_list, all_list;
    QMap<QString, AlsaMixerElement> vol_ch2id, sw_ch2id;
    AlsaSoundDevice::getCaptureMixerChannels(m_name2card[cardname], NULL, vol_list, vol_ch2id, sw_list, sw_ch2id, &all_list);

    for (QMap<QString, QAlsaMixerElement*>::iterator it = m_MixerElements.begin(); it != m_MixerElements.end(); ++it) {
        delete *it;
    }
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
    for (QList<QString>::const_iterator it = all_list.begin(); it != all_list.end(); ++it, ++idx) {
        QAlsaMixerElement *e = new QAlsaMixerElement(m_groupMixerFrame, *it,
                                                     sw_list.contains(*it), vol_list.contains(*it));
        e->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        e->show();
//         int row = idx / cols;
//         int col = idx % cols;
        m_groupMixerFrameLayout->addWidget(e, idx / cols, idx % cols);
//         IErrorLogClient::staticLogDebug(i18n("row = %1, col = %2", row, col));
        QObject::connect(e, SIGNAL(sigDirty()), this, SLOT(slotSetDirty()));
        m_MixerElements.insert(*it, e);
    }
    restoreCaptureMixerSettings();
    m_groupMixerFrame->adjustSize();
}

void AlsaSoundConfiguration::saveCaptureMixerSettings()
{
    for (QMap<QString, QAlsaMixerElement*>::iterator it = m_MixerElements.begin(); it != m_MixerElements.end(); ++it) {
        const QString     &name = it.key();
        int                card = m_currentCaptureCard;
        QString            id   = AlsaConfigMixerSetting::getIDString(card, name);
        QAlsaMixerElement *e    = *it;
        float              vol    = e->getVolume();
        bool               use    = e->getOverride();
        bool               active = e->getActive();
        e->slotResetDirty();
        m_MixerSettings[id] = AlsaConfigMixerSetting(card,name,use,active,vol);
    }
}

void AlsaSoundConfiguration::restoreCaptureMixerSettings()
{
    for (QMap<QString, QAlsaMixerElement*>::iterator it = m_MixerElements.begin(); it != m_MixerElements.end(); ++it) {
        const QString     &name = it.key();
        int                card = m_currentCaptureCard;
        QString            id   = AlsaConfigMixerSetting::getIDString(card, name);
        QAlsaMixerElement *e    = *it;

        if (m_MixerSettings.contains(id)) {
            const AlsaConfigMixerSetting &s = m_MixerSettings[id];
            e->setVolume(s.m_volume);
            e->setOverride(s.m_use);
            e->setActive(s.m_active);
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

int AlsaSoundConfiguration::listSoundDevices(KComboBox *combobox, QMap<QString, int> *devname2dev, QMap<int, QString> *dev2devname, QMap<int, int> *dev2idx, int card, snd_pcm_stream_t stream)
{
    snd_ctl_t           *handle = NULL;
    int                  dev  = -1;
    snd_ctl_card_info_t *info = NULL;
    snd_pcm_info_t      *pcminfo = NULL;

    snd_ctl_card_info_alloca(&info);
    snd_pcm_info_alloca     (&pcminfo);

    QString ctlname = "hw:"+QString::number(card);

    if (combobox)
        combobox->clear();
    if (devname2dev)
        devname2dev->clear();
    if (dev2devname)
        dev2devname->clear();
    if (dev2idx)
        dev2idx->clear();

    int count = 0;

    if (snd_ctl_open (&handle, ctlname.toLocal8Bit(), 0) == 0) {
        if (snd_ctl_card_info(handle, info) == 0) {

            dev = -1;
            while (1) {
                if (snd_ctl_pcm_next_device(handle, &dev) < 0) {
                    //logError("snd_ctl_pcm_next_device");
                }
                if (dev < 0)
                    break;
                snd_pcm_info_set_device(pcminfo, dev);
                snd_pcm_info_set_subdevice(pcminfo, 0);
                snd_pcm_info_set_stream(pcminfo, stream);
                int err = 0;
                if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
                    if (err != -ENOENT) {
                        //logError(i18n("control digital audio info (%1): %2", card, snd_strerror(err)));
                    }
                    continue;
                }
                const char *dev_name = snd_pcm_info_get_name(pcminfo);
                QString devname = i18nc("context-card-plus-device-number", "%1 device %2", dev_name, dev);
                if (combobox)
                    combobox->addItem(devname);
                if (devname2dev)
                    (*devname2dev)[devname] = dev;
                if (dev2devname)
                    (*dev2devname)[dev] = devname;
                if (dev2idx)
                    (*dev2idx)[dev] = count;
                ++count;
            }
        }
        snd_ctl_close(handle);
    }
    return count;
}

void AlsaSoundConfiguration::slotOK()
{
    if (!m_dirty)
        return;

    if (m_SoundDevice) {
        SoundFormat sf;
        getCaptureSoundFormat(sf);
        m_SoundDevice->setCaptureFormatOverride(cbEnableCaptureFormatOverride->isChecked(), sf);

        m_SoundDevice->setBufferSize        ( editBufferSize    ->value() * 1024);
        m_SoundDevice->enablePlayback       (!chkDisablePlayback->isChecked());
        m_SoundDevice->enableCapture        (!chkDisableCapture ->isChecked());

        int card   = m_name2card[m_comboPlaybackCard->currentText()];
        int device = m_playbackDeviceName2dev[m_comboPlaybackDevice->currentText()];
        m_SoundDevice->setPlaybackDevice( card, device);

        card       = m_name2card[m_comboCaptureCard->currentText()];
        device     = m_captureDeviceName2dev[m_comboCaptureDevice->currentText()];
        m_SoundDevice->setCaptureDevice ( card, device);

        saveCaptureMixerSettings();
        m_SoundDevice->setCaptureMixerSettings(m_MixerSettings);

        m_SoundDevice->setSoftPlaybackVolume(cbEnableSoftVolume->isChecked(), sbSoftVolumeCorrection->value());



    }
    m_dirty = false;
}


void AlsaSoundConfiguration::slotCancel()
{
    if (!m_dirty)
        return;
    m_ignore_updates = true;

    int card = m_SoundDevice ?  m_SoundDevice->getPlaybackCard()   : 0;
    int dev  = m_SoundDevice ?  m_SoundDevice->getPlaybackDevice() : 0;
    m_comboPlaybackCard  ->setCurrentIndex(m_playbackCard2idx[card]);
    slotPlaybackCardSelected(m_comboPlaybackCard->currentText());
    m_comboPlaybackDevice->setCurrentIndex(m_playbackDevice2idx[dev]);

    card = m_SoundDevice ?  m_SoundDevice->getCaptureCard()   : 0;
    dev  = m_SoundDevice ?  m_SoundDevice->getCaptureDevice() : 0;
    m_comboCaptureCard  ->setCurrentIndex(m_captureCard2idx[card]);
    slotCaptureCardSelected(m_comboCaptureCard->currentText());
    m_comboCaptureDevice->setCurrentIndex(m_captureDevice2idx[dev]);

    //IErrorLogClient::staticLogDebug(QString("capture: card = %1(%2), dev = %3").arg(card).arg(m_captureCard2idx[card]).arg(dev));

    editBufferSize    ->setValue  (m_SoundDevice ?  m_SoundDevice->getBufferSize()/1024 : 4);
    chkDisablePlayback->setChecked(m_SoundDevice ? !m_SoundDevice->isPlaybackEnabled()  : false);
    chkDisableCapture ->setChecked(m_SoundDevice ? !m_SoundDevice->isCaptureEnabled()   : false);

    //IErrorLogClient::staticLogDebug(QString("capture: card = %1").arg(m_comboCaptureCard->currentText()));


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
    m_dirty = false;
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
    int idx_Endianess = ENDIAN_LITTLE_IDX;

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

    switch(sf.m_IsSigned) {
        case true  : idx_Sign = SIGN_SIGNED_IDX; break;
        case false : idx_Sign = SIGN_UNSIGNED_IDX; break;
    }

    switch(sf.m_Channels) {
        case 2: idx_Channels = CHANNELS_STEREO_IDX; break;
        case 1: idx_Channels = CHANNELS_MONO_IDX; break;
    }

    switch(sf.m_Endianess) {
        case LITTLE_ENDIAN: idx_Endianess = ENDIAN_LITTLE_IDX; break;
        case BIG_ENDIAN:    idx_Endianess = ENDIAN_BIG_IDX;    break;
    }

    m_cbRate      ->setCurrentIndex(idx_Rate);
    m_cbBits      ->setCurrentIndex(idx_Bits);
    m_cbSign      ->setCurrentIndex(idx_Sign);
    m_cbChannels  ->setCurrentIndex(idx_Channels);
    m_cbEndianess ->setCurrentIndex(idx_Endianess);

    m_ignore_updates = false;
}


void AlsaSoundConfiguration::getCaptureSoundFormat(SoundFormat &sf) const
{
    int idx_Rate      = m_cbRate      ->currentIndex();
    int idx_Bits      = m_cbBits      ->currentIndex();
    int idx_Sign      = m_cbSign      ->currentIndex();
    int idx_Channels  = m_cbChannels  ->currentIndex();
    int idx_Endianess = m_cbEndianess ->currentIndex();

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

    switch(idx_Endianess) {
        case ENDIAN_LITTLE_IDX : sf.m_Endianess = LITTLE_ENDIAN; break;
        case ENDIAN_BIG_IDX    : sf.m_Endianess = BIG_ENDIAN;    break;
        default                : sf.m_Endianess = BYTE_ORDER;    break;
    }
}



#include "alsa-sound-configuration.moc"
