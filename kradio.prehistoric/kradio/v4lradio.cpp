/***************************************************************************
                          v4lradio.cpp  -  description
                             -------------------
    begin                : Don Mär  8 21:57:17 CET 2001
    copyright            : (C) 2002, 2003 by Ernst Martin Witte
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

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <linux/videodev.h>
#include <linux/soundcard.h>

#include <qlayout.h>

#include <kconfig.h>
#include <kiconloader.h>
#include <kdialogbase.h>

#include "v4lradio.h"
#include "v4lradio-configuration.h"
#include "pluginmanager.h"

struct _lrvol { unsigned char l, r; short dummy; };

V4LRadio::V4LRadio(const QString &name)
  : PluginBase(name),
    m_volume(0),
    m_muted(true),
    m_signalQuality(0),
    m_stereo(false),
    
	m_minQuality(0.75),
	m_minFrequency(0),
	m_maxFrequency(0),

	m_seekHelper(*this),	
	m_scanStep(0.05),
	
	m_mixerChannel(0),	
	m_radio_fd(0),
	m_mixer_fd(0),
	m_pollTimer(this),

	m_blockReadTuner(false),
	m_blockReadAudio(false)
{
	QObject::connect (&m_pollTimer, SIGNAL(timeout()), this, SLOT(poll()));
	m_pollTimer.start(333);

	m_tuner = new video_tuner;
	m_tuner->tuner = 0;
	m_audio = new video_audio;

	m_seekHelper.connect(this);
}


V4LRadio::~V4LRadio()
{
	setPower(false);

	delete m_audio;
	delete m_tuner;
}


bool V4LRadio::connect (Interface *i)
{
	bool a = IRadioDevice::connect(i);
	bool b = IRadioSound::connect(i);
	bool c = ISeekRadio::connect(i);
	bool d = IFrequencyRadio::connect(i);
	bool e = IV4LCfg::connect(i);
/*
    if (a) kdDebug() << "V4LRadio: IRadioDevice connected\n";
    if (b) kdDebug() << "V4LRadio: IRadioSound connected\n";
    if (c) kdDebug() << "V4LRadio: ISeekRadio connected\n";
    if (d) kdDebug() << "V4LRadio: IFrequency connected\n";
*/
	return a || b || c || d || e;
}


bool V4LRadio::disconnect (Interface *i)
{
	bool a = IRadioDevice::disconnect(i);
	bool b = IRadioSound::disconnect(i);
	bool c = ISeekRadio::disconnect(i);
	bool d = IFrequencyRadio::disconnect(i);
	bool e = IV4LCfg::disconnect(i);
/*
    if (a) kdDebug() << "V4LRadio: IRadioDevice disconnected\n";
    if (b) kdDebug() << "V4LRadio: IRadioSound disconnected\n";
    if (c) kdDebug() << "V4LRadio: ISeekRadio disconnected\n";
    if (d) kdDebug() << "V4LRadio: IFrequency disconnected\n";
*/
	return a || b || c || d || e;
}


// IRadioDevice methods

bool V4LRadio::setPower (bool on)
{
	return on ? powerOn() : powerOff();
}

bool V4LRadio::powerOn ()
{
	if (isPowerOn())
		return true;

    radio_init();
    if (isPowerOn()) {
		unmute();
		notifyPowerChanged(true);
	}

    return true;
}


bool V4LRadio::powerOff ()
{
	if (! isPowerOn())
		return true;

    mute();
    radio_done();
    if (isPowerOff()) {
		notifyPowerChanged(false);
	}

    return true;
}


bool V4LRadio::activateStation(const RadioStation &rs)
{
	const FrequencyRadioStation *frs = dynamic_cast<const FrequencyRadioStation*>(&rs);
	if (frs == NULL)
		return false;

	if (setFrequency(frs->frequency())) {
		m_currentStation = *frs;

		if (frs->initialVolume() > 0)
			setVolume(frs->initialVolume());
			
		return true;
	}

	return false;
}



bool V4LRadio::isPowerOn() const
{
	return m_radio_fd != 0;
}


bool V4LRadio::isPowerOff() const
{
	return m_radio_fd <= 0;
}


const RadioStation &V4LRadio::getCurrentStation() const
{
	return m_currentStation;
}



// IRadioSound methods

bool V4LRadio::setVolume (float vol)
{
	if (vol > 1.0) vol = 1.0;
	if (vol < 0) vol = 0.0;

	const int divs = 100;
	vol = round(vol * divs) / float(divs);

  	if (m_volume != vol) {
	    if(m_radio_fd != 0) {
    		_lrvol tmpvol;
			tmpvol.r = tmpvol.l = (unsigned int)(round(vol * divs));
			if (ioctl(m_mixer_fd, MIXER_WRITE(m_mixerChannel), &tmpvol) != 0) {
				kdDebug() << "V4LRadio::setVolume: " << i18n("error setting volume to") << " " << vol;
				return false;
			}
	    }
		m_volume = vol;
		notifyVolumeChanged(m_volume);
	}

	return true;
}


bool V4LRadio::mute (bool mute)
{
	// mute
  	if (!readAudioInfo())
	   return false;

    if (m_muted != mute) {
		if (mute)		m_audio->flags |= VIDEO_AUDIO_MUTE;
		else			m_audio->flags &= ~VIDEO_AUDIO_MUTE;

		return writeAudioInfo();
	}
	return false;
}


bool V4LRadio::unmute (bool unmute)
{
	return mute(!unmute);
}


bool V4LRadio::setSignalMinQuality (float mq)
{
	m_minQuality = mq;
	notifySignalMinQualityChanged(m_minQuality);
	return true;
}


bool    V4LRadio::setStereo(bool /*b*/)
{
	// FIXME if possible
	return false;  // we can't do that currently, not even switch stereo to mono
}


float   V4LRadio::getVolume() const
{
	if (m_radio_fd != 0) {
		_lrvol tmpvol;
		if(ioctl(m_mixer_fd, MIXER_READ(m_mixerChannel), &tmpvol)) {
			kdDebug() << "V4LRadio::getVolume: "
			          << i18n("error reading volume")
			          << endl;
			tmpvol.l = tmpvol.r = 0;
		}
		float v = float(tmpvol.l) / 100.0;

		if (m_volume != v) {
			m_volume = v;
			notifyVolumeChanged(m_volume);
		}
	}
	return m_volume;
}


float   V4LRadio::getSignalMinQuality() const
{
	return m_minQuality;
}


float   V4LRadio::getSignalQuality() const
{
  	readTunerInfo();
    return m_signalQuality;
}


bool   V4LRadio::hasGoodQuality() const
{
	return getSignalQuality() >= m_minQuality;
}


bool    V4LRadio::isStereo() const
{
	readAudioInfo();
	return m_stereo;
}


bool    V4LRadio::isMuted() const
{
	readAudioInfo();
    return m_muted;
}


// ISeekRadio

bool V4LRadio::startSeekUp()
{
	return startSeek(true);
}

bool V4LRadio::startSeekDown()
{
	return startSeek(false);
}

bool V4LRadio::startSeek(bool up)
{
	if (isPowerOn()) {
		m_seekHelper.start(up ? SeekHelper::up : SeekHelper::down);
		return true;
	} else {
		return false;
	}
}

bool V4LRadio::stopSeek()
{
	m_seekHelper.stop();
	return true;
}

bool V4LRadio::isSeekRunning() const
{
	return m_seekHelper.isRunning();
}


bool V4LRadio::isSeekUpRunning() const
{
	return m_seekHelper.isRunningUp();
}


bool V4LRadio::isSeekDownRunning() const
{
	return m_seekHelper.isRunningDown();
}



// IFrequencyRadio

bool V4LRadio::setFrequency(float freq)
{
//	if (isSeekRunning())
//		stopSeek();
	
	if (m_currentStation.frequency() == freq) {
		return true;
	}
	
	if (m_radio_fd > 0) {
		
  		bool oldMute = isMuted();
  		if (!oldMute) mute();


		if (!m_tunercache.valid) readTunerInfo();
		float         df = m_tunercache.deltaF;
		
  		unsigned long lfreq = (unsigned long) round(freq / df);

	  	if (freq > getMaxFrequency() || freq < getMinFrequency()) {
	  		kdDebug() << "V4LRadio::setFrequency: "
	                  << i18n("invalid frequency") << " "
	                  << freq << endl;
            if (!oldMute) unmute();
	    	return false;
	    }

  		if (ioctl(m_radio_fd, VIDIOCSFREQ, &lfreq)) {
  			kdDebug() << "V4LRadio::setFrequency: "
                      << i18n("error setting frequency to")
                      << freq << endl;
            // unmute the old radio with the old radio station
            if (!oldMute) unmute();
            return false;
        }

        // unmute this radio device, because we now have the current
        // radio station
  		if (!oldMute) unmute();
	}
	
    m_currentStation.setFrequency(freq);
    notifyFrequencyChanged(freq, &m_currentStation);
    notifyStationChanged(m_currentStation);
    return true;
}


bool V4LRadio::setMinFrequency (float minF)
{
	float oldm = getMinFrequency();
	m_minFrequency = minF;

	float newm = getMinFrequency();
	if (oldm != newm)
	    notifyMinMaxFrequencyChanged(newm, getMaxFrequency());
	    
	return true;
}


bool V4LRadio::setMaxFrequency (float maxF)
{
	float oldm = getMaxFrequency();
	m_maxFrequency = maxF;

	float newm = getMaxFrequency();
	if (oldm != newm)
	    notifyMinMaxFrequencyChanged(getMinFrequency(), newm);

	return true;
}


bool V4LRadio::setScanStep(float s)
{
	float old = m_scanStep;
	m_scanStep = s;

	if (old != s) notifyScanStepChanged(m_scanStep);
	return true;
}


float V4LRadio::getFrequency()     const
{
	return m_currentStation.frequency();
}


float V4LRadio::getMinFrequency()  const
{
	return m_minFrequency ? m_minFrequency : getMinDeviceFrequency();
}


float V4LRadio::getMaxFrequency()  const
{
	return m_maxFrequency ? m_maxFrequency : getMaxDeviceFrequency();
}


float V4LRadio::getMinDeviceFrequency() const
{
	if (!m_tunercache.valid)
		readTunerInfo();

	return m_tunercache.minF;
}


float V4LRadio::getMaxDeviceFrequency() const
{
	if (!m_tunercache.valid)
		readTunerInfo();

	return m_tunercache.maxF;
}


float V4LRadio::getScanStep()      const
{
	return m_scanStep;
}



// IV4LCfg methods

bool  V4LRadio::setRadioDevice(const QString &s)
{
	if (m_radioDev != s) {
		bool p = isPowerOn();
		powerOff();
		m_radioDev = s;
		setPower(p);
	}
	return true;
}


bool  V4LRadio::setMixerDevice(const QString &s, int ch)
{
	if (m_mixerDev != s || m_mixerChannel != ch) {
		bool p = isPowerOn();
		powerOff();
		m_mixerDev = s;
		m_mixerChannel = ch;
		setPower(p);
	}
	return true;
}


bool  V4LRadio::setDevices(const QString &r, const QString &m, int ch)
{
	if (m_radioDev != r || m_mixerDev != m || m_mixerChannel != ch) {
		bool p = isPowerOn();
		powerOff();
		m_radioDev = r;
		m_mixerDev = m;
		m_mixerChannel = ch;
		setPower(p);
	}
	return true;
}




// PluginBase methods

void   V4LRadio::saveState (KConfig *config) const
{
    config->setGroup(QString("v4lradio-") + name());
    
	config->writeEntry("MixerDev",         m_mixerDev);
	config->writeEntry("RadioDev",         m_radioDev);
	int ch = m_mixerChannel;
	if(ch < 0 || ch >= SOUND_MIXER_NRDEVICES)
		ch = SOUND_MIXER_LINE;
	config->writeEntry("MixerChannel",     mixerChannelNames[ch]);
	config->writeEntry("fMinOverride",     m_minFrequency);
	config->writeEntry("fMaxOverride",     m_maxFrequency);

	config->writeEntry("signalMinQuality", m_minQuality);

	config->writeEntry("scanStep",         m_scanStep);

    config->writeEntry("Frequency",        m_currentStation.frequency());
    config->writeEntry("Volume",           m_volume);

    config->writeEntry("PowerOn",          isPowerOn());
}


void   V4LRadio::restoreState (KConfig *config)
{
    config->setGroup(QString("v4lradio-") + name());

	m_radioDev              = config->readEntry ("RadioDev", "/dev/radio");
	m_mixerDev              = config->readEntry ("MixerDev", "/dev/mixer");

	QString s               = config->readEntry ("MixerChannel", "line");	
	m_mixerChannel = 0;
	for (m_mixerChannel = 0; m_mixerChannel < SOUND_MIXER_NRDEVICES; ++m_mixerChannel) {
		if (s == mixerChannelLabels[m_mixerChannel] ||
			s == mixerChannelNames[m_mixerChannel])
			break;
	}
	if (m_mixerChannel == SOUND_MIXER_NRDEVICES)
		m_mixerChannel = SOUND_MIXER_LINE;

	m_minFrequency          = config->readDoubleNumEntry ("fMinOverride", 87.0);
	m_maxFrequency          = config->readDoubleNumEntry ("fMaxOverride", 108.0);
    notifyMinMaxFrequencyChanged(getMinFrequency(),getMaxFrequency());

	m_minQuality            = config->readDoubleNumEntry ("signalMinQuality", 0.75);
	notifySignalMinQualityChanged(m_minQuality);
	m_scanStep              = config->readDoubleNumEntry ("scanStep", 0.05);
	notifyScanStepChanged(getScanStep());

    setFrequency(config->readDoubleNumEntry("Frequency", 88));
    setVolume(config->readNumEntry         ("Volume",    65535));
    setPower(config->readBoolEntry         ("PowerOn",   false));

}


void V4LRadio::createConfigurationPage()
{
	QFrame *f = m_manager->addConfigurationPage(
	  this,
      i18n("V4L Radio Options"),
	  i18n("V4L Radio Options"),
	  KGlobal::instance()->iconLoader()->loadIcon( "package_utilities", KIcon::NoGroup, KIcon::SizeMedium )
    );

    QGridLayout *l = new QGridLayout(f);
    l->setSpacing( 0 );
    l->setMargin( 0 );

    V4LRadioConfiguration *v4lconf = new V4LRadioConfiguration(f, this);
    connect(v4lconf);
    l->addWidget( v4lconf, 0, 0 );
}


void V4LRadio::createAboutPage()
{
	// FIXME
}
	
////////////////////////////////////////
// anything else

void V4LRadio::radio_init()
{
	if (isSeekRunning())
		stopSeek();
	
	m_mixer_fd = open(m_mixerDev, O_RDONLY);
	if (m_mixer_fd <= 0) {
		radio_done();
		
		kdDebug() << "V4LRadio::radio_init: "
		          << i18n("cannot open mixer device")
		          << " " << m_mixerDev << endl;
		return;
	}
	
	m_radio_fd = open(m_radioDev, O_RDONLY);
  	if (m_radio_fd <= 0) {
		radio_done();

		kdDebug() << "V4LRadio::radio_init: "
		          << i18n("cannot open radio device") << " "
		          << m_radioDev << endl;
    	return;
	}

	readTunerInfo();
	readAudioInfo();

  	// restore frequency
    float old = getFrequency();
    m_currentStation.setFrequency(0);
  	setFrequency(old);

  	// read volume level from mixer
  	setVolume (getVolume());
}


void V4LRadio::radio_done()
{
	if (isSeekRunning())
		stopSeek();
	
	if (m_radio_fd > 0) close (m_radio_fd);
	if (m_mixer_fd > 0) close (m_mixer_fd);

	m_radio_fd = m_mixer_fd = 0;
}










/*
int V4LRadio::set_bass(__u16 lbass)
{
  bass = lbass;
  if(radio_fd == 0)
    return RADIO_DEV_NOT_OPEN;
//  audio->bass = bass;
//  audio->volume = 65535;
//  audio->treble = treble;
//	audio->balance = balance;
//  ioctl(radio_fd, VIDIOCSAUDIO, audio);
  return RADIO_OK;
}

int V4LRadio::set_treble(__u16 ltreble)
{
  treble = ltreble;
  if(radio_fd == 0)
    return RADIO_DEV_NOT_OPEN;
//  audio->treble = treble;
//  audio->volume = 65535;
//  audio->bass = bass;
//	audio->balance = balance;
//  ioctl(radio_fd, VIDIOCSAUDIO, audio);

  return RADIO_OK;
}


*/



bool V4LRadio::readTunerInfo() const
{
	if (m_blockReadTuner) return true;

	float oldminf = m_tunercache.minF;
	float oldmaxf = m_tunercache.maxF;

	if (!m_tunercache.valid) {
		m_tunercache.minF   = 87;
		m_tunercache.maxF   = 109;
		m_tunercache.deltaF = 1.0/16.0;
		m_tunercache.valid  = true;
	}

    if (m_radio_fd > 0) {
		int r = ioctl(m_radio_fd, VIDIOCGTUNER, m_tuner);
		
		if (r == 0) {
			if ((m_tuner->flags & VIDEO_TUNER_LOW) != 0)
				m_tunercache.deltaF = 1.0 / 16000.0;

			m_tunercache.minF = float(m_tuner->rangelow)  * m_tunercache.deltaF;
			m_tunercache.maxF = float(m_tuner->rangehigh) * m_tunercache.deltaF;

			m_tunercache.valid = true;

		} else {
			kdDebug() << "V4LRadio::readTunerInfo: "
			          << i18n("cannot get tuner info")
			          << " (" << i18n("error") << " " << r << ")"
			          << endl;
			return false;
		}
	} else {
		m_tuner->signal = 0;
	}

	// prevent loops, if noticeXYZ-method is reading my state
	m_blockReadTuner = true;

	if (oldminf != m_tunercache.minF || oldmaxf != m_tunercache.maxF)
		notifyDeviceMinMaxFrequencyChanged(m_tunercache.minF, m_tunercache.maxF);

	if (  ! m_minFrequency && (oldminf != m_tunercache.minF)
	   || ! m_maxFrequency && (oldmaxf != m_tunercache.maxF))
		notifyMinMaxFrequencyChanged(getMinFrequency(), getMaxFrequency());
		
	// extract information on current state
	float oldq = m_signalQuality;
	m_signalQuality = float(m_tuner->signal) / 32767.0;

	if (m_signalQuality != oldq)
		notifySignalQualityChanged(m_signalQuality);
	if ( (m_signalQuality >= m_minQuality) != (oldq >= m_minQuality))
		notifySignalQualityChanged(m_signalQuality > m_minQuality);		

	m_blockReadTuner = false;
	
	return true;
}


bool V4LRadio::updateAudioInfo(bool write) const
{
	if (m_blockReadAudio && !write)
		return true;

	if (m_radio_fd > 0) {
		if (ioctl(m_radio_fd, write ? VIDIOCSAUDIO : VIDIOCGAUDIO, m_audio) != 0) {
			kdDebug() << "V4LRadio::updateAudioInfo: "
			          << i18n("error updating radio audio info") << " ("
			          << i18n(write ? "write" : "read") << ")"
			          << endl;

			return false;
		}
	} else {
		m_audio->mode  &= ~VIDEO_SOUND_STEREO;
		m_audio->flags &= ~VIDEO_AUDIO_MUTE;
	}

	bool oldStereo = m_stereo;
	bool oldMute   = m_muted;

	m_stereo = (m_audio->mode  & VIDEO_SOUND_STEREO) != 0;
	m_muted  = (m_audio->flags & VIDEO_AUDIO_MUTE) != 0;

	// prevent loops, if noticeXYZ-method is reading my state
	bool oldBlock = m_blockReadAudio;
	m_blockReadAudio = true;

	if (oldStereo != m_stereo)
		notifyStereoChanged(m_stereo);
	if (oldMute != m_muted)
		notifyMuted(m_muted);
	
	m_blockReadAudio = oldBlock;

	return (m_radio_fd > 0);
}




void V4LRadio::poll()
{
	readTunerInfo();
	readAudioInfo();
	getVolume();
}


