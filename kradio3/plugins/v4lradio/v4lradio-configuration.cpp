/***************************************************************************
                          v4lradio-configuration.cpp  -  description
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

#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/soundcard.h>

#include <qspinbox.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qfile.h>
#include <qpushbutton.h>
#include <qslider.h>

#include <kfiledialog.h>
#include <knuminput.h>
#include <klocale.h>

#include "../../src/libkradio/utils.h"
#include "../../src/libkradio-gui/gui_list_helper.h"
#include "v4lradio-configuration.h"
#include "v4lradio.h"

V4LRadioConfiguration::V4LRadioConfiguration (QWidget *parent, SoundStreamID ssid)
  : V4LRadioConfigurationUI(parent),
    m_SoundStreamID(ssid),
    m_ignoreGUIChanges(false),
    m_myControlChange(0),
    m_orgTreble(-1),
    m_orgBass(-1),
    m_orgBalance(-2),
    m_orgDeviceVolume(-1),
    m_PlaybackMixerHelper(comboPlaybackMixerDevice, StringListHelper::SORT_BY_DESCR),
    m_CaptureMixerHelper (comboCaptureMixerDevice,  StringListHelper::SORT_BY_DESCR),
    m_PlaybackChannelHelper(comboPlaybackMixerChannel, IntListHelper::SORT_BY_ID),
    m_CaptureChannelHelper (comboCaptureMixerChannel,  IntListHelper::SORT_BY_ID)
{
    QObject::connect(buttonSelectRadioDevice, SIGNAL(clicked()),
                     this, SLOT(selectRadioDevice()));
    editRadioDevice->installEventFilter(this);
    QObject::connect(editMinFrequency, SIGNAL(valueChanged(int)),
                     this, SLOT(guiMinFrequencyChanged(int)));
    QObject::connect(editMaxFrequency, SIGNAL(valueChanged(int)),
                     this, SLOT(guiMaxFrequencyChanged(int)));

    QObject::connect(editDeviceVolume, SIGNAL(valueChanged(double)),
                     this, SLOT(slotDeviceVolumeChanged(double)));
    QObject::connect(editTreble, SIGNAL(valueChanged(double)),
                     this, SLOT(slotTrebleChanged(double)));
    QObject::connect(editBass, SIGNAL(valueChanged(double)),
                     this, SLOT(slotBassChanged(double)));
    QObject::connect(editBalance, SIGNAL(valueChanged(double)),
                     this, SLOT(slotBalanceChanged(double)));

    QObject::connect(sliderDeviceVolume, SIGNAL(valueChanged(int)),
                     this, SLOT(slotDeviceVolumeChanged(int)));
    QObject::connect(sliderTreble, SIGNAL(valueChanged(int)),
                     this, SLOT(slotTrebleChanged(int)));
    QObject::connect(sliderBass, SIGNAL(valueChanged(int)),
                     this, SLOT(slotBassChanged(int)));
    QObject::connect(sliderBalance, SIGNAL(valueChanged(int)),
                     this, SLOT(slotBalanceChanged(int)));

    QObject::connect(comboPlaybackMixerDevice, SIGNAL(activated(int)),
                     this, SLOT(slotComboPlaybackMixerSelected(int)));
    QObject::connect(comboCaptureMixerDevice, SIGNAL(activated(int)),
                     this, SLOT(slotComboCaptureMixerSelected(int)));

    sliderBalance->installEventFilter(this);
}


V4LRadioConfiguration::~V4LRadioConfiguration ()
{
}


bool V4LRadioConfiguration::connectI (Interface *i)
{
    bool a = IV4LCfgClient::connectI(i);
    bool b = IFrequencyRadioClient::connectI(i);
    bool c = IRadioDeviceClient::connectI(i);
    bool d = ISoundStreamClient::connectI(i);
    return a || b || c || d;
}


bool V4LRadioConfiguration::disconnectI (Interface *i)
{
    bool a = IV4LCfgClient::disconnectI(i);
    bool b = IFrequencyRadioClient::disconnectI(i);
    bool c = IRadioDeviceClient::disconnectI(i);
    bool d = ISoundStreamClient::disconnectI(i);
    return a || b || c || d;
}

void V4LRadioConfiguration::noticeConnectedI (ISoundStreamServer *s, bool pointer_valid)
{
    ISoundStreamClient::noticeConnectedI(s, pointer_valid);
    if (s && pointer_valid) {
        s->register4_notifyTrebleChanged(this);
        s->register4_notifyBassChanged(this);
        s->register4_notifyBalanceChanged(this);
        s->register4_notifySignalMinQualityChanged(this);

        s->register4_notifyPlaybackChannelsChanged(this);
        s->register4_notifyCaptureChannelsChanged(this);
        s->register4_notifySoundStreamCreated(this);
    }
}

void V4LRadioConfiguration::noticeConnectedSoundClient(ISoundStreamClient::thisInterface *i, bool pointer_valid)
{
    if (i && pointer_valid && i->supportsPlayback()) {
        noticePlaybackMixerChanged(m_PlaybackMixerHelper.count()   ? m_PlaybackMixerHelper.getCurrentItem()   : QString::null,
                                   m_PlaybackChannelHelper.count() ? m_PlaybackChannelHelper.getCurrentItem() : -1);
    }
    if (i && pointer_valid && i->supportsCapture()) {
        noticeCaptureMixerChanged (m_CaptureMixerHelper.count()    ? m_CaptureMixerHelper.getCurrentItem()    : QString::null,
                                   m_CaptureChannelHelper.count()  ? m_CaptureChannelHelper.getCurrentItem()  : -1);
    }
}


void V4LRadioConfiguration::noticeDisconnectedSoundClient(ISoundStreamClient::thisInterface *i, bool pointer_valid)
{
    if (i && pointer_valid && i->supportsPlayback()) {
        noticePlaybackMixerChanged(m_PlaybackMixerHelper.count()   ? m_PlaybackMixerHelper.getCurrentItem()   : QString::null,
                                   m_PlaybackChannelHelper.count() ? m_PlaybackChannelHelper.getCurrentItem() : -1);
    }
    if (i && pointer_valid && i->supportsCapture()) {
        noticeCaptureMixerChanged (m_CaptureMixerHelper.count()    ? m_CaptureMixerHelper.getCurrentItem()    : QString::null,
                                   m_CaptureChannelHelper.count()  ? m_CaptureChannelHelper.getCurrentItem()  : -1);
    }
}

// IV4LCfgClient

bool V4LRadioConfiguration::noticeRadioDeviceChanged(const QString &s)
{
    bool old = m_ignoreGUIChanges;
    m_ignoreGUIChanges = true;

    editRadioDevice->setText(s);

    m_ignoreGUIChanges = old;
    return true;
}


bool V4LRadioConfiguration::noticePlaybackMixerChanged(const QString &_mixer_id, int Channel)
{
    QString mixer_id = _mixer_id;
    bool old = m_ignoreGUIChanges;
    m_ignoreGUIChanges = true;

    m_PlaybackMixerHelper.setData(getPlaybackClientDescriptions());
    m_PlaybackMixerHelper.setCurrentItem(mixer_id);
    mixer_id = m_PlaybackMixerHelper.count() ? m_PlaybackMixerHelper.getCurrentItem() : QString::null;

    ISoundStreamClient *mixer = getSoundStreamClientWithID(mixer_id);
    if (mixer) {
        m_PlaybackChannelHelper.setData(mixer->getPlaybackChannels());
        m_PlaybackChannelHelper.setCurrentItem(Channel);
    }
    labelPlaybackMixerChannel->setEnabled(mixer != NULL);
    comboPlaybackMixerChannel->setEnabled(mixer != NULL);

    m_ignoreGUIChanges = old;
    return true;
}


bool V4LRadioConfiguration::noticeCaptureMixerChanged(const QString &_mixer_id, int Channel)
{
    QString mixer_id = _mixer_id;
    bool old = m_ignoreGUIChanges;
    m_ignoreGUIChanges = true;

    m_CaptureMixerHelper.setData(getCaptureClientDescriptions());
    m_CaptureMixerHelper.setCurrentItem(mixer_id);
    mixer_id = m_CaptureMixerHelper.count() ? m_CaptureMixerHelper.getCurrentItem() : QString::null;

    ISoundStreamClient *mixer = getSoundStreamClientWithID(mixer_id);
    if (mixer) {
        m_CaptureChannelHelper.setData(mixer->getCaptureChannels());
        m_CaptureChannelHelper.setCurrentItem(Channel);
    }
    labelCaptureMixerChannel->setEnabled(mixer != NULL);
    comboCaptureMixerChannel->setEnabled(mixer != NULL);

    m_ignoreGUIChanges = old;
    return true;
}


bool V4LRadioConfiguration::noticeDeviceVolumeChanged(float v)
{
    bool old = m_ignoreGUIChanges;
    m_ignoreGUIChanges = true;
    v = v > 1 ? 1 : v;
    v = v < 0 ? 0 : v;

    if (!m_myControlChange)
        m_orgDeviceVolume = v;

    editDeviceVolume  ->setValue(v);
    sliderDeviceVolume->setValue(m_caps.maxVolume - m_caps.intGetVolume(v));
    m_ignoreGUIChanges = old;
    return true;
}


bool V4LRadioConfiguration::noticeCapabilitiesChanged(const V4LCaps &c)
{
    bool old = m_ignoreGUIChanges;
    m_ignoreGUIChanges = true;

    labelDeviceVolume ->setEnabled(c.hasVolume);
    editDeviceVolume  ->setEnabled(c.hasVolume);
    editDeviceVolume  ->setRange(0, 1, c.volumeStep(), false);
    sliderDeviceVolume->setMinValue(0);
    sliderDeviceVolume->setMaxValue(c.maxVolume - c.minVolume);
    sliderDeviceVolume->setEnabled(c.hasVolume);

    labelTreble ->setEnabled(c.hasTreble);
    editTreble  ->setEnabled(c.hasTreble);
    editTreble  ->setRange(0, 1, c.trebleStep(), false);
    sliderTreble->setMinValue(0);
    sliderTreble->setMaxValue(c.maxTreble - c.minTreble);
    sliderTreble->setEnabled(c.hasTreble);

    labelBass ->setEnabled(c.hasBass);
    editBass  ->setEnabled(c.hasBass);
    editBass  ->setRange(0, 1, c.bassStep(), false);
    sliderBass->setMinValue(0);
    sliderBass->setMaxValue(c.maxBass - c.minBass);
    sliderBass->setEnabled(c.hasBass);

    labelBalance ->setEnabled(c.hasBalance);
    editBalance  ->setEnabled(c.hasBalance);
    editBalance  ->setRange(-1, 1, c.balanceStep(), false);
    sliderBalance->setMinValue(0);
    sliderBalance->setMaxValue(c.maxBalance - c.minBalance);
    sliderBalance->setEnabled(c.hasBalance);

    m_caps = c;

    float tmp = 0;
    noticeDeviceVolumeChanged(queryDeviceVolume());

    queryTreble(m_SoundStreamID, tmp);
    noticeTrebleChanged(m_SoundStreamID, tmp);

    queryBass(m_SoundStreamID, tmp);
    noticeBassChanged(m_SoundStreamID, tmp);

    queryBalance(m_SoundStreamID, tmp);
    noticeBalanceChanged(m_SoundStreamID, tmp);

    m_ignoreGUIChanges = old;
    return true;
}


// IRadioDeviceClient

bool V4LRadioConfiguration::noticeDescriptionChanged (const QString &s, const IRadioDevice */*sender*/)
{
    labelDescription->setText(s);
    return true;
}


// IFrequencyRadioClient

bool V4LRadioConfiguration::noticeFrequencyChanged(float /*f*/, const RadioStation */*s*/)
{
    return false;  // we don't care
}


bool V4LRadioConfiguration::noticeMinMaxFrequencyChanged(float min, float max)
{
    editMinFrequency->setValue((int)rint(min*1000));
    editMaxFrequency->setValue((int)rint(max*1000));
    return true;
}


bool V4LRadioConfiguration::noticeDeviceMinMaxFrequencyChanged(float min, float max)
{
    editMinFrequency->setMinValue((int)rint(min*1000));
    editMaxFrequency->setMaxValue((int)rint(max*1000));
    return true;
}


bool V4LRadioConfiguration::noticeScanStepChanged(float s)
{
    editScanStep->setValue((int)rint(s * 1000));
    return true;
}


// IRadioSoundClient

bool V4LRadioConfiguration::noticeTrebleChanged(SoundStreamID id, float t)
{
    if (id != m_SoundStreamID)
        return false;

    bool old = m_ignoreGUIChanges;
    m_ignoreGUIChanges = true;
    t = t > 1 ? 1 : t;
    t = t < 0 ? 0 : t;

    if (!m_myControlChange)
        m_orgTreble = t;

    editTreble  ->setValue  (t);
    sliderTreble->setValue(m_caps.maxTreble - m_caps.intGetTreble(t));
    m_ignoreGUIChanges = old;
    return true;
}


bool V4LRadioConfiguration::noticeBassChanged(SoundStreamID id, float b)
{
    if (id != m_SoundStreamID)
        return false;

    bool old = m_ignoreGUIChanges;
    m_ignoreGUIChanges = true;
    b = b > 1 ? 1 : b;
    b = b < 0 ? 0 : b;

    if (!m_myControlChange)
        m_orgBass = b;

    editBass  ->setValue(b);
    sliderBass->setValue(m_caps.maxBass - m_caps.intGetBass(b));
    m_ignoreGUIChanges = old;
    return true;
}


bool V4LRadioConfiguration::noticeBalanceChanged(SoundStreamID id, float b)
{
    if (id != m_SoundStreamID)
        return false;

    bool old = m_ignoreGUIChanges;
    m_ignoreGUIChanges = true;
    b = b >  1 ?  1 : b;
    b = b < -1 ? -1 : b;

    if (!m_myControlChange)
        m_orgBalance = b;

    editBalance  ->setValue(b);
    //sliderBalance->setValue(m_caps.maxBalance - m_caps.intGetBalance(b));
    sliderBalance->setValue(m_caps.intGetBalance(b));
    m_ignoreGUIChanges = old;
    return true;
}


bool V4LRadioConfiguration::noticeSignalMinQualityChanged(SoundStreamID id, float q)
{
    if (id != m_SoundStreamID)
        return false;

    editSignalMinQuality->setValue((int)rint(q * 100));
    return true;
}


bool V4LRadioConfiguration::noticeSoundStreamCreated(SoundStreamID id)
{
    if (id.HasSamePhysicalID(m_SoundStreamID)) {
        m_SoundStreamID = id;
    }
    return true;
}


// GUI Slots


void V4LRadioConfiguration::selectRadioDevice()
{
    KFileDialog fd("/dev/",
                   i18n("any ( * )").ascii(),
                   this,
                   i18n("Radio Device Selection").ascii(),
                   TRUE);
    fd.setMode(KFile::File | KFile::ExistingOnly);
    fd.setCaption (i18n("Select Radio Device"));

    if (fd.exec() == QDialog::Accepted) {
        editRadioDevice->setText(fd.selectedFile());
    }
}


bool V4LRadioConfiguration::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::FocusOut && o == editRadioDevice) {
        slotEditRadioDeviceChanged();
/*    } else if (e->type() == QEvent::FocusOut && o == editMixerDevice) {
        slotEditMixerDeviceChanged();*/
    }
    if (e->type() == QEvent::MouseButtonDblClick && o == sliderBalance) {
        slotBalanceCenter();
    }
    return false;
}


void V4LRadioConfiguration::slotEditRadioDeviceChanged()
{
    if (m_ignoreGUIChanges) return;
    const QString &s = editRadioDevice->text();
    if (s != queryRadioDevice() || !queryIsPowerOn()) {
        V4LCaps c = queryCapabilities(s);
        noticeDescriptionChanged(c.description);
    } else {
        noticeDescriptionChanged(queryDescription());
    }
}


void V4LRadioConfiguration::slotComboPlaybackMixerSelected(int /*idx*/)
{
    if (m_ignoreGUIChanges) return;
    QString id = m_PlaybackMixerHelper.getCurrentItem();
    noticePlaybackMixerChanged(id, queryPlaybackMixerChannel());
}


void V4LRadioConfiguration::slotComboCaptureMixerSelected(int /*idx*/)
{
    if (m_ignoreGUIChanges) return;
    QString id = m_CaptureMixerHelper.getCurrentItem();
    noticeCaptureMixerChanged(id, queryCaptureMixerChannel());
}


void V4LRadioConfiguration::slotOK()
{
    sendMinFrequency(((float)editMinFrequency->value()) / 1000.0);
    sendMaxFrequency(((float)editMaxFrequency->value()) / 1000.0);
    sendSignalMinQuality(m_SoundStreamID, editSignalMinQuality->value() * 0.01);
    sendRadioDevice(editRadioDevice->text());
    sendScanStep(((float)editScanStep->value()) / 1000.0);

    sendPlaybackMixer(m_PlaybackMixerHelper.getCurrentItem(),
                      m_PlaybackChannelHelper.getCurrentItem());
    sendCaptureMixer (m_CaptureMixerHelper.getCurrentItem(),
                      m_CaptureChannelHelper.getCurrentItem());

    queryTreble (m_SoundStreamID, m_orgTreble);
    queryBass   (m_SoundStreamID, m_orgBass);
    queryBalance(m_SoundStreamID, m_orgBalance);
    m_orgDeviceVolume = queryDeviceVolume();
}


void V4LRadioConfiguration::slotCancel()
{
    noticeRadioDeviceChanged(queryRadioDevice());
    noticePlaybackMixerChanged(queryPlaybackMixerID(), queryPlaybackMixerChannel());
    noticeCaptureMixerChanged (queryCaptureMixerID(),  queryCaptureMixerChannel());
    noticeMinMaxFrequencyChanged(queryMinFrequency(), queryMaxFrequency());
    float q = 0;
    querySignalMinQuality(m_SoundStreamID, q);
    noticeSignalMinQualityChanged(m_SoundStreamID, q);
    noticeScanStepChanged(queryScanStep());

    sendTreble      (m_SoundStreamID, m_orgTreble);
    sendBass        (m_SoundStreamID, m_orgBass);
    sendBalance     (m_SoundStreamID, m_orgBalance);
    sendDeviceVolume(m_orgDeviceVolume);
}


void V4LRadioConfiguration::guiMinFrequencyChanged(int v)
{
    editMaxFrequency->setMinValue(v);
}


void V4LRadioConfiguration::guiMaxFrequencyChanged(int v)
{
    editMinFrequency->setMaxValue(v);
}

void V4LRadioConfiguration::slotDeviceVolumeChanged (double v) // for KDoubleNumInput, 0.0..1.0
{
    if (m_ignoreGUIChanges) return;
    ++m_myControlChange;
    sendDeviceVolume(v);
    --m_myControlChange;
}

void V4LRadioConfiguration::slotTrebleChanged (double t) // for KDoubleNumInput, 0.0..1.0
{
    if (m_ignoreGUIChanges) return;
    ++m_myControlChange;
    sendTreble(m_SoundStreamID, t);
    --m_myControlChange;
}

void V4LRadioConfiguration::slotBassChanged   (double b) // for KDoubleNumInput, 0.0..1.0
{
    if (m_ignoreGUIChanges) return;
    ++m_myControlChange;
    sendBass(m_SoundStreamID, b);
    --m_myControlChange;
}

void V4LRadioConfiguration::slotBalanceChanged(double b) // for KDoubleNumInput, -1.0..1.0
{
    if (m_ignoreGUIChanges) return;
    ++m_myControlChange;
    sendBalance(m_SoundStreamID, b);
    --m_myControlChange;
}


void V4LRadioConfiguration::slotDeviceVolumeChanged (int v)
{
    if (m_ignoreGUIChanges) return;
    ++m_myControlChange;
    sendDeviceVolume(m_caps.floatGetVolume(m_caps.maxVolume - v));
    --m_myControlChange;
}

void V4LRadioConfiguration::slotTrebleChanged (int t)
{
    if (m_ignoreGUIChanges) return;
    ++m_myControlChange;
    sendTreble(m_SoundStreamID, m_caps.floatGetTreble(m_caps.maxTreble - t));
    --m_myControlChange;
}

void V4LRadioConfiguration::slotBassChanged   (int b)
{
    if (m_ignoreGUIChanges) return;
    ++m_myControlChange;
    sendBass(m_SoundStreamID, m_caps.floatGetBass(m_caps.maxBass - b));
    --m_myControlChange;
}

void V4LRadioConfiguration::slotBalanceChanged(int b)
{
    if (m_ignoreGUIChanges) return;
    ++m_myControlChange;
//    sendBalance(m_caps.floatGetBalance(m_caps.maxBalance - b));
    sendBalance(m_SoundStreamID, m_caps.floatGetBalance(b));
    --m_myControlChange;
}


void V4LRadioConfiguration::slotBalanceCenter()
{
    if (m_ignoreGUIChanges) return;
    ++m_myControlChange;
//    sendBalance(m_caps.floatGetBalance(m_caps.maxBalance - b));
    sendBalance(m_SoundStreamID, 0);
    --m_myControlChange;
}


bool V4LRadioConfiguration::noticePlaybackChannelsChanged(const QString & client_id, const QMap<int, QString> &/*channels*/)
{
    if (m_PlaybackMixerHelper.getCurrentItem() == client_id) {
        noticePlaybackMixerChanged(client_id, m_PlaybackChannelHelper.count() ? m_PlaybackChannelHelper.getCurrentItem() : -1);
    }
    return true;
}


bool V4LRadioConfiguration::noticeCaptureChannelsChanged (const QString & client_id, const QMap<int, QString> &/*channels*/)
{
    QString tmp = m_CaptureMixerHelper.getCurrentItem();
    if (tmp == client_id) {
        noticeCaptureMixerChanged(client_id, m_CaptureChannelHelper.count() ? m_CaptureChannelHelper.getCurrentItem() : -1);
    }
    return true;
}



#include "v4lradio-configuration.moc"
