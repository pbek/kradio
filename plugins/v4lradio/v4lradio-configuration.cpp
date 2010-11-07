/***************************************************************************
                          v4lradio-configuration.cpp  -  description
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

#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/soundcard.h>
#include <math.h>

#include <QtGui/QSpinBox>
#include <QtGui/QLineEdit>
#include <QtGui/QLabel>
#include <QtCore/QFile>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QCheckBox>

#include <kfiledialog.h>
#include <knuminput.h>
#include <klocale.h>
#include <ktabwidget.h>

#include "utils.h"
#include "gui_list_helper.h"
#include "v4lradio-configuration.h"
#include "v4lradio.h"

V4LRadioConfiguration::V4LRadioConfiguration (QWidget *parent, SoundStreamID ssid)
  : QWidget(parent),
    m_SoundStreamID(ssid),
    m_ignoreGUIChanges(false),
    m_myControlChange(0),
    m_orgTreble(-1),
    m_orgBass(-1),
    m_orgBalance(-2),
    m_orgDeviceVolume(-1),
    m_PlaybackMixerHelper  (NULL, StringListHelper::SORT_BY_DESCR),
    m_CaptureMixerHelper   (NULL, StringListHelper::SORT_BY_DESCR),
    m_PlaybackChannelHelper(NULL, StringListHelper::SORT_NONE),
    m_CaptureChannelHelper (NULL, StringListHelper::SORT_NONE)
{

    setupUi(this);
    QButtonGroup *bgProbe = new QButtonGroup(this);
    bgProbe->addButton(m_rbProbeAtPowerOn);
    bgProbe->addButton(m_rbProbeAtStartup);
    QButtonGroup *bgV4L  = new QButtonGroup(this);
    bgV4L->addButton(rbV4L1Version);
    bgV4L->addButton(rbV4L2Version);


    m_PlaybackMixerHelper  .setList(comboPlaybackMixerDevice);
    m_CaptureMixerHelper   .setList(comboCaptureMixerDevice);
    m_PlaybackChannelHelper.setList(comboPlaybackMixerChannel);
    m_CaptureChannelHelper .setList(comboCaptureMixerChannel);

    buttonSelectRadioDevice->setIcon(KIcon("document-open"));

    QObject::connect(buttonSelectRadioDevice, SIGNAL(clicked()),
                     this, SLOT(selectRadioDevice()));
//     m_cbRadioDevice->installEventFilter(this);
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

    QObject::connect(m_cbRadioDevice, SIGNAL(editTextChanged(QString)),
                     this, SLOT(slotEditRadioDeviceChanged()));

    QObject::connect(m_cbRadioDevice, SIGNAL(currentIndexChanged(int)),
                     this, SLOT(slotRadioDeviceIndexChanged(int)));

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

void V4LRadioConfiguration::noticeConnectedI (IV4LCfg *cfg, bool pointer_valid)
{
    IV4LCfgClient::noticeConnectedI(cfg, pointer_valid);
    populateDeviceComboBox();
}

void V4LRadioConfiguration::noticeDisconnectedI (IV4LCfg *cfg, bool pointer_valid)
{
    IV4LCfgClient::noticeDisconnectedI(cfg, pointer_valid);
    populateDeviceComboBox();
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
        updatePlaybackMixerChannelAlternatives();
    }
    if (i && pointer_valid && i->supportsCapture()) {
        updateCaptureMixerChannelAlternatives();
    }
}


void V4LRadioConfiguration::noticeDisconnectedSoundClient(ISoundStreamClient::thisInterface *i, bool pointer_valid)
{
    if (i && pointer_valid && i->supportsPlayback()) {
        updatePlaybackMixerChannelAlternatives();
    }
    if (i && pointer_valid && i->supportsCapture()) {
        updateCaptureMixerChannelAlternatives();
    }
}

bool V4LRadioConfiguration::noticePlaybackChannelsChanged(const QString & /*client_id*/, const QStringList &/*channels*/)
{
    updatePlaybackMixerChannelAlternatives();
    return true;
}


bool V4LRadioConfiguration::noticeCaptureChannelsChanged (const QString & /*client_id*/, const QStringList &/*channels*/)
{
    updateCaptureMixerChannelAlternatives();
    return true;
}



// IV4LCfgClient

bool V4LRadioConfiguration::noticeRadioDeviceChanged(const QString &s)
{
    bool old = m_ignoreGUIChanges;
    m_ignoreGUIChanges = true;
    m_cbRadioDevice->setEditText(s);
    m_ignoreGUIChanges = old;

    slotEditRadioDeviceChanged();
    return true;
}


bool V4LRadioConfiguration::noticePlaybackMixerChanged(const QString &mixer_id, const QString &Channel)
{
//     bool    old        = m_ignoreGUIChanges;
//     m_ignoreGUIChanges = true;

    m_PlaybackMixerHelper  .setOrgItemID(mixer_id);
    m_PlaybackChannelHelper.setOrgItemID(Channel);

//     updatePlaybackMixerChannelAlternatives();

//     m_ignoreGUIChanges = old;
    return true;
}


bool V4LRadioConfiguration::noticeCaptureMixerChanged(const QString &mixer_id, const QString &Channel)
{
//     bool    old        = m_ignoreGUIChanges;
//     m_ignoreGUIChanges = true;

    m_CaptureMixerHelper  .setOrgItemID(mixer_id);
    m_CaptureChannelHelper.setOrgItemID(Channel);

//     updateCaptureMixerChannelAlternatives();

/*    m_ignoreGUIChanges = old;*/
    return true;
}


void V4LRadioConfiguration::updatePlaybackMixerChannelAlternatives()
{
    m_PlaybackMixerHelper.alternativesChanged(getPlaybackClientDescriptions());
    ISoundStreamClient *mixer = getSoundStreamClientWithID(m_PlaybackMixerHelper.getCurrentItemID());
    if (mixer) {
        m_PlaybackChannelHelper.alternativesChanged(mixer->getPlaybackChannels());
    }
    labelPlaybackMixerChannel->setEnabled(mixer != NULL);
    comboPlaybackMixerChannel->setEnabled(mixer != NULL);
}


void V4LRadioConfiguration::updateCaptureMixerChannelAlternatives()
{
    m_CaptureMixerHelper.alternativesChanged(getCaptureClientDescriptions());
    ISoundStreamClient *mixer = getSoundStreamClientWithID(m_CaptureMixerHelper.getCurrentItemID());
    if (mixer) {
        m_CaptureChannelHelper.alternativesChanged(mixer->getCaptureChannels());
    }
    labelCaptureMixerChannel->setEnabled(mixer != NULL);
    comboCaptureMixerChannel->setEnabled(mixer != NULL);
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
    sliderDeviceVolume->setValue(m_caps.intGetVolume(v));
    m_ignoreGUIChanges = old;
    return true;
}


bool V4LRadioConfiguration::noticeCapabilitiesChanged(const V4LCaps &c)
{
    bool old = m_ignoreGUIChanges;
    m_ignoreGUIChanges = true;

    rbV4L1Version->setEnabled(c.v4l_version_support[V4L_Version1]);
    rbV4L2Version->setEnabled(c.v4l_version_support[V4L_Version2]);
    if (rbV4L1Version->isChecked() && !c.v4l_version_support[V4L_Version1]) {
        rbV4L1Version->setChecked(false);
        rbV4L2Version->setChecked(true);
    }
    if (rbV4L2Version->isChecked() && !c.v4l_version_support[V4L_Version2]) {
        rbV4L1Version->setChecked(true);
        rbV4L2Version->setChecked(false);
    }

    labelDeviceVolume ->setEnabled(c.hasVolume);
    editDeviceVolume  ->setEnabled(c.hasVolume);
    editDeviceVolume  ->setRange(0, 1, max(0.01, c.volumeStep()), false);
    sliderDeviceVolume->setMinimum(0);
    sliderDeviceVolume->setMaximum(c.maxVolume - c.minVolume);
    sliderDeviceVolume->setEnabled(c.hasVolume);

    labelTreble ->setEnabled(c.hasTreble);
    editTreble  ->setEnabled(c.hasTreble);
    editTreble  ->setRange(0, 1, max(0.01, c.trebleStep()), false);
    sliderTreble->setMinimum(c.minTreble);
    sliderTreble->setMaximum(c.maxTreble);
    sliderTreble->setEnabled(c.hasTreble);

    labelBass ->setEnabled(c.hasBass);
    editBass  ->setEnabled(c.hasBass);
    editBass  ->setRange(0, 1, max(0.01, c.bassStep()), false);
    sliderBass->setMinimum(c.minBass);
    sliderBass->setMaximum(c.maxBass);
    sliderBass->setEnabled(c.hasBass);

    labelBalance ->setEnabled(c.hasBalance);
    editBalance  ->setEnabled(c.hasBalance);
    editBalance  ->setRange(-1, 1, max(0.01, c.balanceStep()), false);
    sliderBalance->setMinimum(c.minBalance);
    sliderBalance->setMaximum(c.maxBalance);
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

bool V4LRadioConfiguration::noticeActivePlaybackChanged(bool a, bool muteCaptureChannelPlayback)
{
    bool old = m_ignoreGUIChanges;
    m_ignoreGUIChanges = true;
    m_checkboxActivePlayback->setChecked(a);
    m_checkboxActivePlaybackMuteCaptureChannelPlayback->setChecked(muteCaptureChannelPlayback);
    m_ignoreGUIChanges = old;
    return true;
}

bool V4LRadioConfiguration::noticeMuteOnPowerOffChanged(bool a)
{
    bool old = m_ignoreGUIChanges;
    m_ignoreGUIChanges = true;
    m_checkboxMuteOnPowerOff->setChecked(a);
    m_ignoreGUIChanges = old;
    return true;
}

bool V4LRadioConfiguration::noticeForceRDSEnabledChanged(bool a)
{
    bool old = m_ignoreGUIChanges;
    m_ignoreGUIChanges = true;
    m_checkboxForceRDSEnabled->setChecked(a);
    m_ignoreGUIChanges = old;
    return true;
}

bool V4LRadioConfiguration::noticeDeviceProbeAtStartupChanged(bool e)
{
    bool old = m_ignoreGUIChanges;
    m_ignoreGUIChanges = true;
    m_rbProbeAtStartup->setChecked(e);
    m_rbProbeAtPowerOn->setChecked(!e);
    m_ignoreGUIChanges = old;
    return true;
}

bool V4LRadioConfiguration::noticeVolumeZeroOnPowerOffChanged(bool a)
{
    bool old = m_ignoreGUIChanges;
    m_ignoreGUIChanges = true;
    m_checkboxVolumeZeroOnPowerOff->setChecked(a);
    m_ignoreGUIChanges = old;
    return true;
}


bool V4LRadioConfiguration::noticeV4LVersionOverrideChanged(V4LVersion vo)
{
    bool old = m_ignoreGUIChanges;
    m_ignoreGUIChanges = true;
    rbV4L1Version->setChecked(vo == V4L_Version1);
    rbV4L2Version->setChecked(vo == V4L_Version2);
    m_ignoreGUIChanges = old;
    return true;
}

// IRadioDeviceClient

bool V4LRadioConfiguration::noticeDescriptionChanged (const QString &s, const IRadioDevice */*sender*/)
{
    QString descr = !s.isEmpty() ? s : i18n("Could not get device description.");

    const QString    &devname = m_cbRadioDevice->currentText();
    QFileInfo         dev(devname);
    if (!dev.exists()) {
        descr = i18n("Device file does not exist or top level directories are not accessible.");
    }
    else if (!dev.isReadable() && !dev.isWritable()) {
        descr = i18n("You do not have read+write permissions for the device.");
    }
    else if (!dev.isReadable()) {
        descr = i18n("You do not have read permissions for the device.");
    }
    else if (!dev.isWritable()) {
        descr = i18n("You do not have write permissions for the device.");
    }
    labelDescription->setWordWrap(true);
    labelDescription->setText( descr );
    return true;
}


// IFrequencyRadioClient

bool V4LRadioConfiguration::noticeFrequencyChanged(float /*f*/, const FrequencyRadioStation */*s*/)
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
    int i_min = (int)rint(min*1000);
    int i_max = (int)rint(max*1000);
    editMinFrequency->setMinimum(i_min);
    editMaxFrequency->setMaximum(i_max);
    labelDevMinFrequency->setText(i18n("%1 kHz", i_min));
    labelDevMaxFrequency->setText(i18n("%1 kHz", i_max));
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
    sliderTreble->setValue(m_caps.intGetTreble(t));
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
    sliderBass->setValue(m_caps.intGetBass(b));
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
    KFileDialog fd(KUrl("/dev/"),
                   i18n("any ( * )"),
                   this);
    fd.setModal(TRUE);
    fd.setMode(KFile::File | KFile::ExistingOnly);
    fd.setCaption (i18n("Select Radio Device"));

    if (fd.exec() == QDialog::Accepted) {
        m_cbRadioDevice->setEditText(fd.selectedFile());
        slotEditRadioDeviceChanged();
    }
}


bool V4LRadioConfiguration::eventFilter(QObject *o, QEvent *e)
{
/*    if (e->type() == QEvent::FocusOut && o == editRadioDevice) {
        slotEditRadioDeviceChanged();
    }*/
    if (e->type() == QEvent::MouseButtonDblClick && o == sliderBalance) {
        slotBalanceCenter();
    }
    return false;
}

void V4LRadioConfiguration::slotRadioDeviceIndexChanged(int idx)
{
    m_cbRadioDevice->setEditText(m_cbRadioDevice->itemData(idx).toString());
    slotEditRadioDeviceChanged();
}

void V4LRadioConfiguration::slotEditRadioDeviceChanged()
{
    if (m_ignoreGUIChanges) return;
    const QString &s = m_cbRadioDevice->currentText();
    if (s != queryRadioDevice()) { // || !queryIsPowerOn()) {
        V4LCaps c = queryCapabilities(s);
        noticeDescriptionChanged(c.description);
        noticeCapabilitiesChanged(c);
    } else {
        noticeDescriptionChanged(queryDescription());
        noticeCapabilitiesChanged(queryCapabilities());
    }
}


void V4LRadioConfiguration::slotComboPlaybackMixerSelected(int /*idx*/)
{
    if (m_ignoreGUIChanges) return;
    // since we do not know in which order signals are delivered, ensure that the helper already nows about the selection change
    m_PlaybackMixerHelper.slotUserSelection();
    updatePlaybackMixerChannelAlternatives();
}


void V4LRadioConfiguration::slotComboCaptureMixerSelected(int /*idx*/)
{
    if (m_ignoreGUIChanges) return;
    // since we do not know in which order signals are delivered, ensure that the helper already nows about the selection change
    m_CaptureMixerHelper.slotUserSelection();
    updateCaptureMixerChannelAlternatives();
}


void V4LRadioConfiguration::slotOK()
{
    m_PlaybackMixerHelper  .slotOK();
    m_CaptureMixerHelper   .slotOK();
    m_PlaybackChannelHelper.slotOK();
    m_CaptureChannelHelper .slotOK();

    sendMinFrequency(((float)editMinFrequency->value()) / 1000.0);
    sendMaxFrequency(((float)editMaxFrequency->value()) / 1000.0);
    sendSignalMinQuality(m_SoundStreamID, editSignalMinQuality->value() * 0.01);

    V4LVersion tmp_vo = V4L_VersionUnkown;
    if (rbV4L1Version->isChecked()) {
        tmp_vo = V4L_Version1;
    }
    else if (rbV4L2Version->isChecked()) {
        tmp_vo = V4L_Version2;
    }
    sendV4LVersionOverride(tmp_vo);
    sendRadioDevice(m_cbRadioDevice->currentText());

    sendScanStep(((float)editScanStep->value()) / 1000.0);

    QString cap_mixer_id   = m_CaptureMixerHelper  .getCurrentItemID();
    QString cap_channel_id = m_CaptureChannelHelper.getCurrentItemID();
    sendCaptureMixer (cap_mixer_id, cap_channel_id, false);


    QString plb_mixer_id   = m_PlaybackMixerHelper  .getCurrentItemID();
    QString plb_channel_id = m_PlaybackChannelHelper.getCurrentItemID();
    sendPlaybackMixer (plb_mixer_id, plb_channel_id, false);

    sendActivePlayback      (m_checkboxActivePlayback ->isChecked(), m_checkboxActivePlaybackMuteCaptureChannelPlayback->isChecked());
    sendMuteOnPowerOff      (m_checkboxMuteOnPowerOff ->isChecked());
    sendForceRDSEnabled     (m_checkboxForceRDSEnabled->isChecked());
    sendDeviceProbeAtStartup(m_rbProbeAtStartup       ->isChecked());
    sendVolumeZeroOnPowerOff(m_checkboxVolumeZeroOnPowerOff->isChecked());

    queryTreble (m_SoundStreamID, m_orgTreble);
    queryBass   (m_SoundStreamID, m_orgBass);
    queryBalance(m_SoundStreamID, m_orgBalance);
    m_orgDeviceVolume = queryDeviceVolume();
}


void V4LRadioConfiguration::slotCancel()
{
    m_PlaybackMixerHelper  .slotCancel();
    m_CaptureMixerHelper   .slotCancel();
    m_PlaybackChannelHelper.slotCancel();
    m_CaptureChannelHelper .slotCancel();

    noticeRadioDeviceChanged(queryRadioDevice());
    noticeMinMaxFrequencyChanged(queryMinFrequency(), queryMaxFrequency());
    bool mutecaptureplayback = false;
    bool activepb            = queryActivePlayback(mutecaptureplayback);
    noticeActivePlaybackChanged      (activepb, mutecaptureplayback);
    noticeMuteOnPowerOffChanged      (queryMuteOnPowerOff());
    noticeForceRDSEnabledChanged     (queryForceRDSEnabled());
    noticeDeviceProbeAtStartupChanged(queryDeviceProbeAtStartup());
    noticeVolumeZeroOnPowerOffChanged(queryVolumeZeroOnPowerOff());
    noticeV4LVersionOverrideChanged  (queryV4LVersionOverride());

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
    editMaxFrequency->setMinimum(v);
}


void V4LRadioConfiguration::guiMaxFrequencyChanged(int v)
{
    editMinFrequency->setMaximum(v);
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
    sendDeviceVolume(m_caps.floatGetVolume(m_caps.minVolume + v));
    --m_myControlChange;
}

void V4LRadioConfiguration::slotTrebleChanged (int t)
{
    if (m_ignoreGUIChanges) return;
    ++m_myControlChange;
    sendTreble(m_SoundStreamID, m_caps.floatGetTreble(t));
    --m_myControlChange;
}

void V4LRadioConfiguration::slotBassChanged   (int b)
{
    if (m_ignoreGUIChanges) return;
    ++m_myControlChange;
    sendBass(m_SoundStreamID, m_caps.floatGetBass(b));
    --m_myControlChange;
}

void V4LRadioConfiguration::slotBalanceChanged(int b)
{
    if (m_ignoreGUIChanges) return;
    ++m_myControlChange;
    sendBalance(m_SoundStreamID, m_caps.floatGetBalance(b));
    --m_myControlChange;
}


void V4LRadioConfiguration::slotBalanceCenter()
{
    if (m_ignoreGUIChanges) return;
    ++m_myControlChange;
    sendBalance(m_SoundStreamID, 0);
    --m_myControlChange;
}


void V4LRadioConfiguration::populateDeviceComboBox()
{
    m_cbRadioDevice->clear();
    foreach (const DeviceInfo &dev, queryDeviceProposals()) {
        m_cbRadioDevice->addItem(dev.description, dev.path);
    }
}


#include "v4lradio-configuration.moc"
