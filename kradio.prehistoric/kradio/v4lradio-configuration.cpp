/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. 
*****************************************************************************/

#include <qspinbox.h>
#include <qlineedit.h>

#include "v4lradio-configuration.h"
#include "v4lradio.h"

V4LRadioConfiguration::V4LRadioConfiguration (QWidget *parent, V4LRadio *radio)
  : V4LRadioConfigurationUI(parent)
{
	connect (radio);
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
	// FIXME: mixer channel
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


bool V4LRadioConfiguration::noticeDeviceMinMaxFrequencyChanged(float /*min*/, float /*max*/)
{
	return false;  // we don't care
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






/*

void SetupDialogGeneral::slotInit()
{
}

void SetupDialogGeneral::slotDone()
{
}

void SetupDialogGeneral::mixerDeviceChanged( const QString &md )
{
    MixerChannelMask = 0;
    bool ok = false;
    comboMixerChannel->clear();
    QFile f (md);
    if (f.exists()) {
	int mixer_fd = ::open(md.ascii(), O_RDONLY);
	if (mixer_fd > 0) {
	    if ( ioctl(mixer_fd, SOUND_MIXER_READ_DEVMASK, &MixerChannelMask) == 0 ) {
			for (int i = 0; i < SOUND_MIXER_NRDEVICES; ++i) {
				if (MixerChannelMask & (1 << i)) {
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
}


void SetupDialogGeneral::selectRadioDevice()
{
    KFileDialog fd("/dev/", i18n("any") + " ( * )", this, i18n("radio device selection"), TRUE);
    fd.setMode(KFile::File | KFile::ExistingOnly);
    fd.setCaption (i18n("select radio device"));

    if (fd.exec() == QDialog::Accepted) {
		editRadioDevice->setText(fd.selectedFile());
    }
}


void SetupDialogGeneral::selectMixerDevice()
{
    KFileDialog fd("/dev/", i18n("any") + " ( * )", this, i18n("mixer device selection"), TRUE);
    fd.setMode(KFile::File | KFile::ExistingOnly);
    fd.setCaption (i18n("select mixer device"));

    if (fd.exec() == QDialog::Accepted) {
		editMixerDevice->setText(fd.selectedFile());
    }

}


void SetupDialogGeneral::minFrequencyChanged( int i )
{
    editMaxFrequency->setMinValue(i+1);
    emit sigMinMaxFreqChanged (((float)i) / 1000.0, ((float)editMaxFrequency->value()) / 1000.0);
}


void SetupDialogGeneral::maxFrequencyChanged( int i )
{
    editMinFrequency->setMaxValue(i-1);
    emit sigMinMaxFreqChanged (((float)editMinFrequency->value()) / 1000.0, ((float)i) / 1000.0);
}


void SetupDialogGeneral::setData( const SetupDataGeneral &d )
{
    checkboxShortNamesInQuickbar->setChecked(d.displayOnlyShortNames);

    checkboxOverrideRange->setChecked(d.enableRangeOverride);
    editMinFrequency->setValue((int)round(d.minFrequency * 1000));
    editMaxFrequency->setValue((int)round(d.maxFrequency * 1000));

    editSignalMinQuality->setValue((int)round (d.signalMinQuality * 100));
    editSleep->setValue(d.sleep);
    editScanStep->setValue((int)round(d.scanStep * 1000));

    editRadioDevice->setText(d.radioDev);
    editMixerDevice->setText(d.mixerDev);

	int idx = -1;
	for (int i = 0; i <= d.mixerChannel && i < SOUND_MIXER_NRDEVICES; ++i) {
		if (MixerChannelMask & (1 << i))
			++idx;
	}
    comboMixerChannel->setCurrentItem(idx);
}


void SetupDialogGeneral::getData( SetupDataGeneral &d ) const
{
	d.displayOnlyShortNames = checkboxShortNamesInQuickbar->isChecked();
	d.enableRangeOverride   = checkboxOverrideRange->isChecked();
	d.minFrequency          = ((float)editMinFrequency->value()) / 1000.0;
	d.maxFrequency          = ((float)editMaxFrequency->value()) / 1000.0;
	d.signalMinQuality      = editSignalMinQuality->value() * 0.01;
	d.sleep                 = editSleep->value();
	d.radioDev              = editRadioDevice->text();
	d.mixerDev              = editMixerDevice->text();
	d.scanStep              = ((float)editScanStep->value()) / 1000.0;

	d.mixerChannel          = SOUND_MIXER_LINE;
	
	QString s = comboMixerChannel->currentText();
	for (int i = 0; i < SOUND_MIXER_NRDEVICES; ++i) {
		if (s == mixerChannelNames[i])
			d.mixerChannel = i;
	}
}

*/

