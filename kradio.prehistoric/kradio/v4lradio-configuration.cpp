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

#include <kfiledialog.h>

#include "v4lradio-configuration.h"
#include "v4lradio.h"

V4LRadioConfiguration::V4LRadioConfiguration (QWidget *parent)
  : V4LRadioConfigurationUI(parent),
    m_mixerChannelMask (0)
{
	QObject::connect(buttonSelectRadioDevice, SIGNAL(clicked()),
					 this, SLOT(selectRadioDevice()));
	QObject::connect(buttonSelectMixerDevice, SIGNAL(clicked()),
					 this, SLOT(selectMixerDevice()));
	QObject::connect(editMinFrequency, SIGNAL(valueChanged(int)),
					 this, SLOT(guiMinFrequencyChanged(int)));
	QObject::connect(editMaxFrequency, SIGNAL(valueChanged(int)),
					 this, SLOT(guiMaxFrequencyChanged(int)));
}


V4LRadioConfiguration::~V4LRadioConfiguration ()
{
}


bool V4LRadioConfiguration::connect (Interface *i)
{
	bool a = IV4LCfgClient::connect(i);
	bool b = IFrequencyRadioClient::connect(i);
	bool c = IRadioSoundClient::connect(i);
	return a || b || c;
}


bool V4LRadioConfiguration::disconnect (Interface *i)
{
	bool a = IV4LCfgClient::disconnect(i);
	bool b = IFrequencyRadioClient::disconnect(i);
	bool c = IRadioSoundClient::disconnect(i);
	return a || b || c;
}


// IV4LCfgClient

bool V4LRadioConfiguration::noticeRadioDeviceChanged(const QString &s)
{
	editRadioDevice->setText(s);
	return true;
}


bool V4LRadioConfiguration::noticeMixerDeviceChanged(const QString &s, int Channel)
{
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

	return true;
}


// IFrequencyRadioClient

bool V4LRadioConfiguration::noticeFrequencyChanged(float /*f*/, const RadioStation */*s*/)
{
	return false;  // we don't care
}


bool V4LRadioConfiguration::noticeMinMaxFrequencyChanged(float min, float max)
{
	editMinFrequency->setValue((int)round(min*1000));
	editMaxFrequency->setValue((int)round(max*1000));
	return true;
}


bool V4LRadioConfiguration::noticeDeviceMinMaxFrequencyChanged(float min, float max)
{
	editMinFrequency->setMinValue((int)round(min*1000));
	editMaxFrequency->setMaxValue((int)round(max*1000));
	return true;
}


bool V4LRadioConfiguration::noticeScanStepChanged(float s)
{
	editScanStep->setValue((int)round(s * 1000));
	return true;
}


// IRadioSoundClient

bool V4LRadioConfiguration::noticeVolumeChanged(float /*v*/)
{
	return false; // we don't care
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
	editSignalMinQuality->setValue((int)round(q * 100));
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
    KFileDialog fd("/dev/", i18n("any") + " ( * )", this, i18n("radio device selection"), TRUE);
    fd.setMode(KFile::File | KFile::ExistingOnly);
    fd.setCaption (i18n("select radio device"));

    if (fd.exec() == QDialog::Accepted) {
		editRadioDevice->setText(fd.selectedFile());
    }
}


void V4LRadioConfiguration::selectMixerDevice()
{
    KFileDialog fd("/dev/", i18n("any") + " ( * )", this, i18n("mixer device selection"), TRUE);
    fd.setMode(KFile::File | KFile::ExistingOnly);
    fd.setCaption (i18n("select mixer device"));

    if (fd.exec() == QDialog::Accepted) {
		editMixerDevice->setText(fd.selectedFile());
    }

}


void V4LRadioConfiguration::slotOk()
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
}


void V4LRadioConfiguration::slotCancel()
{
	noticeRadioDeviceChanged(queryRadioDevice());
	noticeMixerDeviceChanged(queryMixerDevice(), queryMixerChannel());
	noticeMinMaxFrequencyChanged(queryMinFrequency(), queryMaxFrequency());
	noticeSignalMinQualityChanged(querySignalMinQuality());
	noticeScanStepChanged(queryScanStep());
}


void V4LRadioConfiguration::guiMinFrequencyChanged(int v)
{
	editMaxFrequency->setMinValue(v);
}


void V4LRadioConfiguration::guiMaxFrequencyChanged(int v)
{
	editMinFrequency->setMaxValue(v);
}

