/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code.
*****************************************************************************/

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

#include "v4lradio-configuration.h"
#include "v4lradio.h"

V4LRadioConfiguration::V4LRadioConfiguration (QWidget *parent)
  : V4LRadioConfigurationUI(parent),
    m_mixerChannelMask (0),
    m_ignoreGUIChanges(false),
    m_myControlChange(0),
    m_orgTreble(-1),
    m_orgBass(-1),
    m_orgBalance(-2),
    m_orgDeviceVolume(-1)
{
	QObject::connect(buttonSelectRadioDevice, SIGNAL(clicked()),
					 this, SLOT(selectRadioDevice()));
	QObject::connect(buttonSelectMixerDevice, SIGNAL(clicked()),
					 this, SLOT(selectMixerDevice()));
	QObject::connect(editRadioDevice, SIGNAL(textChanged(const QString &)),
					 this, SLOT(slotEditRadioDeviceChanged(const QString &)));
	QObject::connect(editMixerDevice, SIGNAL(textChanged(const QString &)),
					 this, SLOT(slotEditMixerDeviceChanged(const QString &)));
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
}


V4LRadioConfiguration::~V4LRadioConfiguration ()
{
}


bool V4LRadioConfiguration::connect (Interface *i)
{
	bool a = IV4LCfgClient::connect(i);
	bool b = IFrequencyRadioClient::connect(i);
	bool c = IRadioSoundClient::connect(i);
	bool d = IRadioDeviceClient::connect(i);
	return a || b || c || d;
}


bool V4LRadioConfiguration::disconnect (Interface *i)
{
	bool a = IV4LCfgClient::disconnect(i);
	bool b = IFrequencyRadioClient::disconnect(i);
	bool c = IRadioSoundClient::disconnect(i);
	bool d = IRadioDeviceClient::disconnect(i);
	return a || b || c || d;
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


bool V4LRadioConfiguration::noticeMixerDeviceChanged(const QString &s, int Channel)
{
	bool old = m_ignoreGUIChanges;
	m_ignoreGUIChanges = true;

	editMixerDevice->setText(s);

    m_mixerChannelMask = 0;
    bool ok = false;
    comboMixerChannel->clear();
    QFile f (s);
    if (f.exists()) {
		int mixer_fd = ::open((const char*)s, O_RDONLY);
		if (mixer_fd > 0) {
			if ( ioctl(mixer_fd, SOUND_MIXER_READ_DEVMASK, &m_mixerChannelMask) == 0 ) {
				for (int i = 0; i < SOUND_MIXER_NRDEVICES; ++i) {
					if (m_mixerChannelMask & (1 << i)) {
						comboMixerChannel->insertItem(mixerChannelNames[i]);
					}
				}
				ok = true;
			}
			::close(mixer_fd);
		}
    }
    labelMixerChannel->setEnabled(ok);
    comboMixerChannel->setEnabled(ok);

	int idx = -1;
	for (int i = 0; i <= Channel && i < SOUND_MIXER_NRDEVICES; ++i) {
		if (m_mixerChannelMask & (1 << i))
			++idx;
	}
    comboMixerChannel->setCurrentItem(idx);

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

bool V4LRadioConfiguration::noticeVolumeChanged(float /*v*/)
{
	return false; // we don't care
}


bool V4LRadioConfiguration::noticeTrebleChanged(float t)
{
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


bool V4LRadioConfiguration::noticeBassChanged(float b)
{
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


bool V4LRadioConfiguration::noticeBalanceChanged(float b)
{
	bool old = m_ignoreGUIChanges;
	m_ignoreGUIChanges = true;
	b = b >  1 ?  1 : b;
	b = b < -1 ? -1 : b;

	if (!m_myControlChange)
		m_orgBalance = b;

	editBalance  ->setValue(b);
	sliderBalance->setValue(m_caps.maxBalance - m_caps.intGetBalance(b));
	m_ignoreGUIChanges = old;
	return true;
}


bool V4LRadioConfiguration::noticeSignalQualityChanged(float /*q*/)
{
	return false; // we don't care
}


bool V4LRadioConfiguration::noticeSignalQualityChanged(bool /*good*/)
{
	return false; // we don't care
}


bool V4LRadioConfiguration::noticeSignalMinQualityChanged(float q)
{
	editSignalMinQuality->setValue((int)rint(q * 100));
	return true;
}


bool V4LRadioConfiguration::noticeStereoChanged(bool /*s*/)
{
	return false; // we don't care
}


bool V4LRadioConfiguration::noticeMuted(bool /*m*/)
{
	return false; // we don't care
}


// GUI Slots


void V4LRadioConfiguration::selectRadioDevice()
{
    KFileDialog fd("/dev/", i18n("any ( * )"), this, i18n("Radio Device Selection"), TRUE);
    fd.setMode(KFile::File | KFile::ExistingOnly);
    fd.setCaption (i18n("Select Radio Device"));

    if (fd.exec() == QDialog::Accepted) {
		editRadioDevice->setText(fd.selectedFile());
    }
}


void V4LRadioConfiguration::selectMixerDevice()
{
    KFileDialog fd("/dev/", i18n("any ( * )"), this, i18n("Mixer Device Selection"), TRUE);
    fd.setMode(KFile::File | KFile::ExistingOnly);
    fd.setCaption (i18n("Select Mixer Device"));

    if (fd.exec() == QDialog::Accepted) {
		editMixerDevice->setText(fd.selectedFile());
    }
}


void V4LRadioConfiguration::slotEditRadioDeviceChanged(const QString &s)
{
	if (m_ignoreGUIChanges) return;
	if (s != queryRadioDevice()) {
		V4LCaps c = V4LRadio::readV4LCaps(s);
		noticeDescriptionChanged(c.description);
	} else {
		noticeDescriptionChanged(queryDescription());
	}
}


void V4LRadioConfiguration::slotEditMixerDeviceChanged(const QString &s)
{
	if (m_ignoreGUIChanges) return;
	noticeMixerDeviceChanged(s, queryMixerChannel());
}


void V4LRadioConfiguration::slotOK()
{
	sendMinFrequency(((float)editMinFrequency->value()) / 1000.0);
	sendMaxFrequency(((float)editMaxFrequency->value()) / 1000.0);
	sendSignalMinQuality(editSignalMinQuality->value() * 0.01);
	sendRadioDevice(editRadioDevice->text());
	sendScanStep(((float)editScanStep->value()) / 1000.0);

	int mixerChannel          = SOUND_MIXER_LINE;

	QString s = comboMixerChannel->currentText();
	for (int i = 0; i < SOUND_MIXER_NRDEVICES; ++i) {
		if (s == mixerChannelNames[i])
			mixerChannel = i;
	}
		
	sendMixerDevice(editMixerDevice->text(), mixerChannel);

	m_orgTreble       = queryTreble();
	m_orgBass         = queryBass();
	m_orgBalance      = queryBalance();
	m_orgDeviceVolume = queryDeviceVolume();
}


void V4LRadioConfiguration::slotCancel()
{	
	noticeRadioDeviceChanged(queryRadioDevice());
	noticeMixerDeviceChanged(queryMixerDevice(), queryMixerChannel());
	noticeMinMaxFrequencyChanged(queryMinFrequency(), queryMaxFrequency());
	noticeSignalMinQualityChanged(querySignalMinQuality());
	noticeScanStepChanged(queryScanStep());
	
	sendTreble      (m_orgTreble);
	sendBass        (m_orgBass);
	sendBalance     (m_orgBalance);
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
	sendTreble(t);
	--m_myControlChange;
}

void V4LRadioConfiguration::slotBassChanged   (double b) // for KDoubleNumInput, 0.0..1.0
{
	if (m_ignoreGUIChanges) return;
	++m_myControlChange;
	sendBass(b);
	--m_myControlChange;
}

void V4LRadioConfiguration::slotBalanceChanged(double b) // for KDoubleNumInput, -1.0..1.0
{
	if (m_ignoreGUIChanges) return;
	++m_myControlChange;
	sendBalance(b);
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
	sendTreble(m_caps.floatGetTreble(m_caps.maxTreble - t));
	--m_myControlChange;
}

void V4LRadioConfiguration::slotBassChanged   (int b)
{
	if (m_ignoreGUIChanges) return;
	++m_myControlChange;
	sendBass(m_caps.floatGetBass(m_caps.maxBass - b));
	--m_myControlChange;
}

void V4LRadioConfiguration::slotBalanceChanged(int b)
{
	if (m_ignoreGUIChanges) return;
	++m_myControlChange;
	sendBalance(m_caps.floatGetBalance(m_caps.maxBalance - b));
	--m_myControlChange;
}

