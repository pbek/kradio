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
#include <qscrollview.h>

#include <kurlrequester.h>
#include <knuminput.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <ktabwidget.h>

#include "alsa-mixer-element.h"
#include "alsa-sound-configuration.h"
#include "alsa-sound.h"


AlsaSoundConfiguration::AlsaSoundConfiguration (QWidget *parent, AlsaSoundDevice *dev)
 : AlsaSoundConfigurationUI(parent),
   m_SoundDevice (dev),
   m_groupMixerLayout(NULL),
   m_groupMixerScrollView(NULL),
   m_groupMixerSubFrame(NULL),
   m_dirty(true),
   m_ignore_updates(false)
{
    QObject::connect(m_comboPlaybackCard,   SIGNAL(activated(int)),    this, SLOT(slotSetDirty()));
    QObject::connect(m_comboCaptureCard,    SIGNAL(activated(int)),    this, SLOT(slotSetDirty()));
    QObject::connect(m_comboPlaybackDevice, SIGNAL(activated(int)),    this, SLOT(slotSetDirty()));
    QObject::connect(m_comboCaptureDevice,  SIGNAL(activated(int)),    this, SLOT(slotSetDirty()));
    QObject::connect(editHWBufferSize,      SIGNAL(valueChanged(int)), this, SLOT(slotSetDirty()));
    QObject::connect(editBufferSize,        SIGNAL(valueChanged(int)), this, SLOT(slotSetDirty()));
    QObject::connect(chkDisablePlayback,    SIGNAL(toggled(bool)),     this, SLOT(slotSetDirty()));
    QObject::connect(chkDisableCapture,     SIGNAL(toggled(bool)),     this, SLOT(slotSetDirty()));

    QObject::connect(m_comboPlaybackCard, SIGNAL(activated(const QString &)),
                     this, SLOT(slotPlaybackCardSelected(const QString &)));
    QObject::connect(m_comboCaptureCard, SIGNAL(activated(const QString &)),
                     this, SLOT(slotCaptureCardSelected(const QString &)));

    m_groupMixer->setColumnLayout(0, Qt::Horizontal );

    QHBoxLayout *tmp_layout = new QHBoxLayout( m_groupMixer->layout() );

    m_groupMixerScrollView = new QScrollView (m_groupMixer);
    m_groupMixerScrollView->setFrameShape(QFrame::NoFrame);
    m_groupMixerScrollView->setFrameShadow(QFrame::Plain);
    m_groupMixerScrollView->enableClipper(true);
    m_groupMixerScrollView->setResizePolicy(QScrollView::AutoOneFit);
    //m_groupMixerScrollView->setHScrollBarMode(QScrollView::AlwaysOn);

    tmp_layout->addWidget(m_groupMixerScrollView);


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

    if (m_groupMixerSubFrame)
        delete m_groupMixerSubFrame;

    m_groupMixerSubFrame = new QFrame(m_groupMixerScrollView->viewport());
    m_groupMixerSubFrame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_groupMixerScrollView->addChild(m_groupMixerSubFrame);

    int rows = 1;
    int cols = (all_list.count()+rows-1)/rows;
    m_groupMixerLayout = new QGridLayout( m_groupMixerSubFrame, rows, cols, 0, 0 );
    m_groupMixerLayout->setAlignment( Qt::AlignBottom );

    int idx = 0;
    for (QValueListConstIterator<QString> it = all_list.begin(); it != all_list.end(); ++it, ++idx) {
        QAlsaMixerElement *e = new QAlsaMixerElement(m_groupMixerSubFrame, *it,
                                                     sw_list.contains(*it), vol_list.contains(*it));
        QObject::connect(e, SIGNAL(sigDirty()), this, SLOT(slotSetDirty()));
        m_groupMixerLayout->addWidget(e, idx > cols, idx % cols);
        e->show();
        m_MixerElements.insert(*it, e);
    }
    restoreCaptureMixerSettings();
    m_groupMixerSubFrame->show();
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
        e->slotResetDirty();
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
    if (!m_dirty)
        return;

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
    m_dirty = false;
}


void AlsaSoundConfiguration::slotCancel()
{
    if (!m_dirty)
        return;
    m_ignore_updates = true;

    int card = m_SoundDevice ?  m_SoundDevice->getPlaybackCard()   : 0;
    int dev  = m_SoundDevice ?  m_SoundDevice->getPlaybackDevice() : 0;
    m_comboPlaybackCard  ->setCurrentItem(m_playbackCard2idx[card]);
    slotPlaybackCardSelected(m_comboPlaybackCard->currentText());
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

#include "alsa-sound-configuration.moc"
