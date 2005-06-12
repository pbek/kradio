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

#include <kurlrequester.h>
#include <knuminput.h>
#include <klineedit.h>
#include <kcombobox.h>

#include "alsa-sound-configuration.h"
#include "alsa-sound.h"

AlsaSoundConfiguration::AlsaSoundConfiguration (QWidget *parent, AlsaSoundDevice *dev)
 : AlsaSoundConfigurationUI(parent),
   m_SoundDevice (dev)
{
    slotCancel();
    QObject::connect(m_comboPlaybackCard, SIGNAL(activated(const QString &)),
                     this, SLOT(slotPlaybackCardSelected(const QString &)));
    QObject::connect(m_comboCaptureCard, SIGNAL(activated(const QString &)),
                     this, SLOT(slotCaptureCardSelected(const QString &)));

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
                    m_captureDevice2idx[card] = idx_capture++;
                }
            }
        } else {
            break;
        }
    }
    slotPlaybackCardSelected(m_comboPlaybackCard->currentText());
    slotCaptureCardSelected(m_comboCaptureCard->currentText());
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

    listSoundDevices(m_comboCaptureDevice, &m_captureDeviceName2dev, &m_dev2captureDeviceName, &m_captureDevice2idx, m_name2card[cardname], SND_PCM_STREAM_CAPTURE);
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
        m_SoundDevice->setBufferSize        ( editBufferSize    ->value() * 1024);
        m_SoundDevice->enablePlayback       (!chkDisablePlayback->isChecked());
        m_SoundDevice->enableCapture        (!chkDisableCapture ->isChecked());

        int card   = m_name2card[m_comboPlaybackCard->currentText()];
        int device = m_playbackDeviceName2dev[m_comboPlaybackDevice->currentText()];
        m_SoundDevice->setPlaybackDevice( card, device);
        card   = m_name2card[m_comboCaptureCard->currentText()];
        device = m_captureDeviceName2dev[m_comboCaptureDevice->currentText()];
        m_SoundDevice->setCaptureDevice     ( card, device);
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
    m_comboCaptureDevice->setCurrentItem(m_captureDevice2idx[dev]);

    editBufferSize    ->setValue  (m_SoundDevice ?  m_SoundDevice->getBufferSize()/1024 : 4);
    chkDisablePlayback->setChecked(m_SoundDevice ? !m_SoundDevice->isPlaybackEnabled()  : false);
    chkDisableCapture ->setChecked(m_SoundDevice ? !m_SoundDevice->isCaptureEnabled()   : false);
}


void AlsaSoundConfiguration::slotUpdateConfig()
{
    slotCancel();
}



#include "alsa-sound-configuration.moc"
