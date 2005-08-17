/***************************************************************************
                       alsa-sound-configuration.cpp  -  description
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

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlayout.h>

#include <kurlrequester.h>
#include <knuminput.h>
#include <klineedit.h>
#include <kcombobox.h>

#include "alsa-mixer-element.h"
#include "alsa-sound-configuration.h"
#include "alsa-sound.h"


AlsaSoundConfiguration::AlsaSoundConfiguration (QWidget *parent, AlsaSoundDevice *dev)
 : AlsaSoundConfigurationUI(parent),
   m_SoundDevice (dev),
   m_groupMixerLayout(NULL)
{
    QObject::connect(m_comboPlaybackCard, SIGNAL(activated(const QString &)),
                     this, SLOT(slotPlaybackCardSelected(const QString &)));
    QObject::connect(m_comboCaptureCard, SIGNAL(activated(const QString &)),
                     this, SLOT(slotCaptureCardSelected(const QString &)));

    m_groupMixer->setColumnLayout(0, Qt::Horizontal );

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
                    m_comboPlaybackCard->insertItem(name);
                    m_playbackCard2idx[card] = idx_playback++;
                }
                if (listSoundDevices(NULL, NULL, NULL, NULL, card, SND_PCM_STREAM_CAPTURE)) {
                    m_comboCaptureCard->insertItem(name);
                    m_captureCard2idx[card]  = idx_capture++;
                }
            }
        } else {
            break;
        }
    }

    slotCancel();
    slotPlaybackCardSelected (m_comboPlaybackCard->currentText());
    slotCaptureCardSelected  (m_comboCaptureCard->currentText());
}


AlsaSoundConfiguration::~AlsaSoundConfiguration ()
{
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

    for (QMapIterator<QString, QAlsaMixerElement*> it = m_MixerElements.begin(); it != m_MixerElements.end(); ++it) {
        delete *it;
    }
    m_MixerElements.clear();

    if (m_groupMixerLayout)
        delete m_groupMixerLayout;

    m_groupMixerLayout = new QHBoxLayout( m_groupMixer->layout() );
    m_groupMixerLayout->setAlignment( Qt::AlignBottom );

    for (QValueListConstIterator<QString> it = all_list.begin(); it != all_list.end(); ++it) {
        QAlsaMixerElement *e = new QAlsaMixerElement(m_groupMixer, *it,
                                                     sw_list.contains(*it), vol_list.contains(*it));
        m_groupMixerLayout->addWidget(e);
        e->show();
        m_MixerElements.insert(*it, e);
    }
    restoreCaptureMixerSettings();
}

void AlsaSoundConfiguration::saveCaptureMixerSettings()
{
    for (QMapIterator<QString, QAlsaMixerElement*> it = m_MixerElements.begin(); it != m_MixerElements.end(); ++it) {
        const QString     &name = it.key();
        int                card = m_currentCaptureCard;
        QString            id   = AlsaConfigMixerSetting::getIDString(card, name);
        QAlsaMixerElement *e    = *it;
        float              vol    = e->getVolume();
        bool               use    = e->getOverride();
        bool               active = e->getActive();
        m_MixerSettings[id] = AlsaConfigMixerSetting(card,name,use,active,vol);
    }
}

void AlsaSoundConfiguration::restoreCaptureMixerSettings()
{
    for (QMapIterator<QString, QAlsaMixerElement*> it = m_MixerElements.begin(); it != m_MixerElements.end(); ++it) {
        const QString     &name = it.key();
        int                card = m_currentCaptureCard;
        QString            id   = AlsaConfigMixerSetting::getIDString(card, name);
        QAlsaMixerElement *e    = *it;

        if (m_MixerSettings.contains(id)) {
            const AlsaConfigMixerSetting &s = m_MixerSettings[id];
            e->setVolume(s.m_volume);
            e->setOverride(s.m_use);
            e->setActive(s.m_active);
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

    if (snd_ctl_open (&handle, ctlname.ascii(), 0) == 0) {
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
                        //logError(QString("control digital audio info (%1): %2").arg(card).arg(snd_strerror(err)));
                    }
                    continue;
                }
                const char *dev_name = snd_pcm_info_get_name(pcminfo);
                QString devname = QString(dev_name) + " device " + QString::number(dev);
                if (combobox)
                    combobox->insertItem(devname);
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
    if (m_SoundDevice) {
        m_SoundDevice->setHWBufferSize      ( editHWBufferSize  ->value() * 1024);
        m_SoundDevice->setBufferSize        ( editBufferSize    ->value() * 1024);
        m_SoundDevice->enablePlayback       (!chkDisablePlayback->isChecked());
        m_SoundDevice->enableCapture        (!chkDisableCapture ->isChecked());

        int card   = m_name2card[m_comboPlaybackCard->currentText()];
        int device = m_playbackDeviceName2dev[m_comboPlaybackDevice->currentText()];
        m_SoundDevice->setPlaybackDevice( card, device);
        card   = m_name2card[m_comboCaptureCard->currentText()];
        device = m_captureDeviceName2dev[m_comboCaptureDevice->currentText()];
        m_SoundDevice->setCaptureDevice     ( card, device);

        saveCaptureMixerSettings();
        m_SoundDevice->setCaptureMixerSettings(m_MixerSettings);
    }
}


void AlsaSoundConfiguration::slotCancel()
{
    int card = m_SoundDevice ?  m_SoundDevice->getPlaybackCard()   : 0;
    int dev  = m_SoundDevice ?  m_SoundDevice->getPlaybackDevice() : 0;
    m_comboPlaybackCard  ->setCurrentItem(m_playbackCard2idx[card]);
    m_comboPlaybackDevice->setCurrentItem(m_playbackDevice2idx[dev]);
    card = m_SoundDevice ?  m_SoundDevice->getCaptureCard()   : 0;
    dev  = m_SoundDevice ?  m_SoundDevice->getCaptureDevice() : 0;

    m_comboCaptureCard  ->setCurrentItem(m_captureCard2idx[card]);
    slotCaptureCardSelected(m_comboCaptureCard->currentText());

    m_comboCaptureDevice->setCurrentItem(m_captureDevice2idx[dev]);

    //IErrorLogClient::staticLogDebug(QString("capture: card = %1(%2), dev = %3").arg(card).arg(m_captureCard2idx[card]).arg(dev));

    editHWBufferSize  ->setValue  (m_SoundDevice ?  m_SoundDevice->getHWBufferSize()/1024 : 4);
    editBufferSize    ->setValue  (m_SoundDevice ?  m_SoundDevice->getBufferSize()/1024 : 4);
    chkDisablePlayback->setChecked(m_SoundDevice ? !m_SoundDevice->isPlaybackEnabled()  : false);
    chkDisableCapture ->setChecked(m_SoundDevice ? !m_SoundDevice->isCaptureEnabled()   : false);

    //IErrorLogClient::staticLogDebug(QString("capture: card = %1").arg(m_comboCaptureCard->currentText()));


    if (m_SoundDevice)
        m_MixerSettings = m_SoundDevice->getCaptureMixerSettings();
    else
        m_MixerSettings.clear();
    restoreCaptureMixerSettings();
}


void AlsaSoundConfiguration::slotUpdateConfig()
{
    slotCancel();
}



#include "alsa-sound-configuration.moc"
