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

#ifdef HAVE_V4L2
#include "linux/videodev2.h"
#endif
#include "linux/videodev.h"
#include <linux/soundcard.h>
#include <string.h> // memcpy needed

#include <qlayout.h>

#include <kconfig.h>
#include <kiconloader.h>
#include <kdialogbase.h>

#include "v4lradio.h"
#include "v4lradio-configuration.h"
#include "pluginmanager.h"

struct _lrvol { unsigned char l, r; short dummy; };


///////////////////////////////////////////////////////////////////////

V4LRadio::V4LRadio(const QString &name)
  : PluginBase(name, "Video For Linux Plugin"),
    m_volume(0),
    m_treble(0.5),
    m_bass(0.5),
    m_balance(0),
    m_deviceVolume(0.9),
    m_muted(true),
    m_signalQuality(0),
    m_stereo(false),
    
	m_minQuality(0.75),
	m_minFrequency(0),
	m_maxFrequency(0),

	m_seekHelper(*this),	
	m_scanStep(0.05),

	m_radioDev("/dev/radio0"),
	m_mixerDev("/dev/mixer0"),
	m_mixerChannel(0),	
	m_radio_fd(-1),
	m_mixer_fd(-1),
	m_pollTimer(this),

	m_blockReadTuner(false),
	m_blockReadAudio(false)
{
	QObject::connect (&m_pollTimer, SIGNAL(timeout()), this, SLOT(poll()));
	m_pollTimer.start(333);

	m_audio = new video_audio;
	m_tuner = new video_tuner;
	m_tuner->tuner = 0;
#ifdef HAVE_V4L2
	m_tuner2 = new v4l2_tuner;
	m_tuner2->index = 0;
#endif
	m_caps.version = 0;

	m_seekHelper.connect(this);
}


V4LRadio::~V4LRadio()
{
	setPower(false);

	delete m_audio;
	delete m_tuner;
#ifdef HAVE_V4L2
	delete m_tuner2;
#endif
}


bool V4LRadio::connect (Interface *i)
{
	bool a = IRadioDevice::connect(i);
	bool b = IRadioSound::connect(i);
	bool c = ISeekRadio::connect(i);
	bool d = IFrequencyRadio::connect(i);
	bool e = IV4LCfg::connect(i);

	return a || b || c || d || e;
}


bool V4LRadio::disconnect (Interface *i)
{
	bool a = IRadioDevice::disconnect(i);
	bool b = IRadioSound::disconnect(i);
	bool c = ISeekRadio::disconnect(i);
	bool d = IFrequencyRadio::disconnect(i);
	bool e = IV4LCfg::disconnect(i);

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
	return m_radio_fd >= 0;
}


bool V4LRadio::isPowerOff() const
{
	return m_radio_fd < 0;
}


const RadioStation &V4LRadio::getCurrentStation() const
{
	return m_currentStation;
}


const QString &V4LRadio::getDescription() const
{
	return m_caps.description;
}


// IRadioSound methods

bool V4LRadio::setVolume (float vol)
{
	if (vol > 1.0) vol = 1.0;
	if (vol < 0) vol = 0.0;

	const int divs = 100;
	vol = rint(vol * divs) / float(divs);

  	if (m_volume != vol) {
	    if(m_radio_fd >= 0) {
    		_lrvol tmpvol;
			tmpvol.r = tmpvol.l = (unsigned int)(rint(vol * divs));
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





bool V4LRadio::setTreble (float t)
{
	if (t > 1.0) t = 1.0;
	if (t < 0)   t = 0.0;
	if ((int)rint(m_treble*65535) != (int)rint(t*65535)) {
		m_treble = t;
		writeAudioInfo();
		notifyTrebleChanged(t);
	}
	return true;
}


bool V4LRadio::setBass (float b)
{
	if (b > 1.0) b = 1.0;
	if (b < 0)   b = 0.0;
	if ((int)rint(m_bass*65535) != (int)rint(b*65535)) {
		m_bass = b;
		writeAudioInfo();
		notifyBassChanged(b);
	}

	return true;
}


bool V4LRadio::setBalance (float b)
{
	if (b > +1.0) b = +1.0;
	if (b < -1.0) b = -1.0;
	if ((int)rint(m_balance*32767) != (int)rint(b*32767)) {
		m_balance = b;
		writeAudioInfo();
		notifyBalanceChanged(b);
	}
	return true;
}


bool V4LRadio::setDeviceVolume (float v)
{
	if (v > 1.0) v = 1.0;
	if (v < 0)   v = 0;
	if ((int)rint(m_deviceVolume*65535) != (int)rint(v*65535)) {
		m_deviceVolume = v;
		writeAudioInfo();
		notifyDeviceVolumeChanged(v);
	}
	return true;
}


bool V4LRadio::mute (bool mute)
{
    if (m_muted != mute) {
		m_muted = mute;
		bool r = writeAudioInfo();
		notifyMuted(m_muted);
		return r;
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
	if (m_radio_fd >= 0) {
		_lrvol tmpvol;
		if(ioctl(m_mixer_fd, MIXER_READ(m_mixerChannel), &tmpvol)) {
			kdDebug() << "V4LRadio::getVolume: "
			          << i18n("error reading volume")
			          << endl;
			tmpvol.l = tmpvol.r = 0;
		}
		float v = float(tmpvol.l) / 100.0;

		if (rint(100*m_volume) != rint(100*v)) {
			m_volume = v;
			notifyVolumeChanged(m_volume);
		}
	}
	return m_volume;
}


float V4LRadio::getTreble () const
{
	readAudioInfo();
	return m_treble;
}


float V4LRadio::getBass () const
{
	readAudioInfo();
	return m_bass;
}


float V4LRadio::getBalance () const
{
	readAudioInfo();
	return m_balance;
}


float V4LRadio::getDeviceVolume () const
{
	readAudioInfo();
	return m_deviceVolume;
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

bool V4LRadio::toBeginning()
{
	setFrequency(getMinFrequency());
	return true;
}

bool V4LRadio::toEnd()
{
	setFrequency(getMaxFrequency());
	return true;
}

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

float V4LRadio::getProgress () const
{
	float min = getMinFrequency();
	float max = getMaxFrequency();

	return (getFrequency() - min) / (max - min);
}


// IFrequencyRadio

bool V4LRadio::setFrequency(float freq)
{
//	if (isSeekRunning())
//		stopSeek();

	if (m_currentStation.frequency() == freq) {
		return true;
	}

	float minf = getMinFrequency();
	float maxf = getMaxFrequency();

	if (m_radio_fd >= 0) {

  		bool oldMute = isMuted();
  		if (!oldMute) mute();


		if (!m_tunercache.valid) readTunerInfo();
		float         df = m_tunercache.deltaF;

  		unsigned long lfreq = (unsigned long) rint(freq / df);

	  	if (freq > maxf || freq < minf) {
	  		kdDebug() << "V4LRadio::setFrequency: "
	                  << i18n("invalid frequency") << " "
	                  << freq << endl;
            if (!oldMute) unmute();
	    	return false;
	    }

		int r = -1;
		if (m_caps.version == 1) {
			r = ioctl(m_radio_fd, VIDIOCSFREQ, &lfreq);
		}
#ifdef HAVE_V4L2
		else if (m_caps.version == 2) {
			v4l2_frequency   tmp;
			tmp.tuner = 0;
			tmp.type = V4L2_TUNER_RADIO;
			tmp.frequency = lfreq;
			r = ioctl(m_radio_fd, VIDIOC_S_FREQUENCY, &tmp);
		}
#endif
	    else {
			kdDebug() << "V4LRadio::setFrequency: don't known how to handle V4L-version " << m_caps.version << endl;
	    }

  		if (r) {
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
    notifyProgress((freq - minf) / (maxf - minf));
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

		m_caps = readV4LCaps(m_radioDev);
		notifyRadioDeviceChanged(m_radioDev);
        notifyDescriptionChanged(m_caps.description);
        notifyCapabilitiesChanged(m_caps);
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
		notifyMixerDeviceChanged(m_mixerDev, m_mixerChannel);
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

		m_caps = readV4LCaps(m_radioDev);
		notifyRadioDeviceChanged(m_radioDev);
        notifyDescriptionChanged(m_caps.description);
        notifyCapabilitiesChanged(m_caps);        
		notifyMixerDeviceChanged(m_mixerDev, m_mixerChannel);
		
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
    config->writeEntry("Treble",           m_treble);
    config->writeEntry("Bass",             m_bass);
    config->writeEntry("Balance",          m_balance);
    config->writeEntry("DeviceVolume",     m_deviceVolume);

    config->writeEntry("PowerOn",          isPowerOn());
}


void   V4LRadio::restoreState (KConfig *config)
{
    config->setGroup(QString("v4lradio-") + name());

	QString rDev            = config->readEntry ("RadioDev", "/dev/radio");
	QString mDev            = config->readEntry ("MixerDev", "/dev/mixer");
	QString s               = config->readEntry ("MixerChannel", "line");
	int c = 0;
	for (c = 0; c < SOUND_MIXER_NRDEVICES; ++c) {
		if (s == mixerChannelLabels[c] ||
			s == mixerChannelNames[c])
			break;
	}
	if (c == SOUND_MIXER_NRDEVICES)
		c = SOUND_MIXER_LINE;
	setDevices(rDev, mDev, c);

	m_minFrequency          = config->readDoubleNumEntry ("fMinOverride", 87.0);
	m_maxFrequency          = config->readDoubleNumEntry ("fMaxOverride", 108.0);
    notifyMinMaxFrequencyChanged(getMinFrequency(),getMaxFrequency());

	m_minQuality            = config->readDoubleNumEntry ("signalMinQuality", 0.75);
	notifySignalMinQualityChanged(m_minQuality);
	m_scanStep              = config->readDoubleNumEntry ("scanStep", 0.05);
	notifyScanStepChanged(getScanStep());

    setFrequency(config->readDoubleNumEntry("Frequency", 88));
    setVolume(config->readDoubleNumEntry   ("Volume",    0.5));
    setPower(config->readBoolEntry         ("PowerOn",   false));
    setTreble(config->readDoubleNumEntry   ("Treble",    0.5));
    setBass(config->readDoubleNumEntry     ("Bass",      0.5));
    setBalance(config->readDoubleNumEntry  ("Balance",   0.0));
    setDeviceVolume(config->readDoubleNumEntry("DeviceVolume", 0.9));
}


ConfigPageInfo V4LRadio::createConfigurationPage()
{
    V4LRadioConfiguration *v4lconf = new V4LRadioConfiguration(NULL);
    connect(v4lconf);
    return ConfigPageInfo (v4lconf,
						   i18n("V4L Radio Options"),
						   i18n("V4L Radio Options"),
						   "package_utilities");
}


QWidget *V4LRadio::createAboutPage()
{
	// FIXME
	return NULL;
}

////////////////////////////////////////
// anything else

void V4LRadio::radio_init()
{
	if (isSeekRunning())
		stopSeek();

	m_caps = readV4LCaps(m_radioDev);
	notifyCapabilitiesChanged(m_caps);
    notifyDescriptionChanged(m_caps.description);
        
	m_mixer_fd = open(m_mixerDev, O_RDONLY);
	if (m_mixer_fd < 0) {
		radio_done();

		kdDebug() << "V4LRadio::radio_init: "
		          << i18n("cannot open mixer device")
		          << " " << m_mixerDev << endl;
		return;
	}

	m_radio_fd = open(m_radioDev, O_RDONLY);
  	if (m_radio_fd < 0) {
		radio_done();

		kdDebug() << "V4LRadio::radio_init: "
		          << i18n("cannot open radio device") << " "
		          << m_radioDev << endl;
    	return;
	}

	readTunerInfo();
	writeAudioInfo(); // set tuner-audio config as used last time
	readAudioInfo();  // reread tuner-audio and read-only flags (e.g. stereo)

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

	if (m_radio_fd >= 0) close (m_radio_fd);
	if (m_mixer_fd >= 0) close (m_mixer_fd);

	m_radio_fd = m_mixer_fd = -1;
}





#define CAPS_NAME_LEN 127
V4LCaps V4LRadio::readV4LCaps(const QString &device)
{
	char buffer[CAPS_NAME_LEN+1];
	int r;
	int fd;

	V4LCaps c;

	fd = open(device, O_RDONLY);

	if (fd < 0) {
		kdDebug() << "V4LRadio::readV4LCaps: cannot open " << device << endl;
		return c;
	}

	video_capability caps;
	r = ioctl(fd, VIDIOCGCAP, &caps);
	if (r == 0) {
		c.version = 1;
		
		size_t l = sizeof(caps.name);
		l = l < CAPS_NAME_LEN ? l : CAPS_NAME_LEN;
		memcpy(buffer, caps.name, l);
		buffer[l] = 0;
		c.description = buffer;

		video_audio audiocaps;
		if (0 == ioctl(fd, VIDIOCGAUDIO, &audiocaps)) {
			kdDebug() << "V4LRadio::readV4LCaps: audio caps = " << audiocaps.flags << endl;
			c.hasVolume  = (audiocaps.flags & VIDEO_AUDIO_VOLUME)  != 0;
			c.hasTreble  = (audiocaps.flags & VIDEO_AUDIO_TREBLE)  != 0;
			c.hasBass    = (audiocaps.flags & VIDEO_AUDIO_BASS)    != 0;

			// at least my driver has support for balance, but the bit is not set ...
			c.hasBalance = true; //(audiocaps.flags & VIDEO_AUDIO_BALANCE) != 0;
		}
	} else {
		kdDebug() << "V4LRadio::readV4LCaps: error reading V4L1 caps\n";
	}
#ifdef HAVE_V4L2
	v4l2_capability caps2;
	r = ioctl(fd, VIDIOC_QUERYCAP, &caps2);
	if (r == 0) {
		c.version  = 2;
		
		size_t l = sizeof(caps.name);
		l = l < CAPS_NAME_LEN ? l : CAPS_NAME_LEN;
		memcpy(buffer, caps.name, l);
		buffer[l] = 0;
		c.description = buffer;

		v4l2_queryctrl  ctrl;
		ctrl.id = V4L2_CID_AUDIO_VOLUME;
		c.hasVolume = (0 == ioctl(fd, VIDIOC_QUERYCTRL, &ctrl)) && !(ctrl.flags & V4L2_CTRL_FLAG_DISABLED);
		ctrl.id = V4L2_CID_AUDIO_TREBLE;
		c.hasTreble = (0 == ioctl(fd, VIDIOC_QUERYCTRL, &ctrl)) && !(ctrl.flags & V4L2_CTRL_FLAG_DISABLED);
		ctrl.id = V4L2_CID_AUDIO_BASS;
		c.hasBass   = (0 == ioctl(fd, VIDIOC_QUERYCTRL, &ctrl)) && !(ctrl.flags & V4L2_CTRL_FLAG_DISABLED);
		ctrl.id = V4L2_CID_AUDIO_BALANCE;
		c.hasBalance = (0 == ioctl(fd, VIDIOC_QUERYCTRL, &ctrl)) && !(ctrl.flags & V4L2_CTRL_FLAG_DISABLED);
		
	} else {
		kdDebug() << "V4LRadio::readV4LCaps: error reading V4L2 caps\n";
	}
#endif
	if (c.version > 0) {
		kdDebug() << "V4L" << c.version << " detected\n";
	} else {
		kdDebug() << "V4L not detected\n";
	}

	close(fd);
	return c;
}


bool V4LRadio::readTunerInfo() const
{
	if (m_blockReadTuner) return true;

	float oldq    = m_signalQuality;
	float oldminf = m_tunercache.minF;
	float oldmaxf = m_tunercache.maxF;

	if (!m_tunercache.valid) {
		m_tunercache.minF   = 87;
		m_tunercache.maxF   = 109;
		m_tunercache.deltaF = 1.0/16.0;
		m_tunercache.valid  = true;
	}

	int r = 0;
    if (m_radio_fd >= 0) {

		// v4l1
		if (m_caps.version == 1) {
		
			r = ioctl(m_radio_fd, VIDIOCGTUNER, m_tuner);

			if (r == 0) {
				if ((m_tuner->flags & VIDEO_TUNER_LOW) != 0)
					m_tunercache.deltaF = 1.0 / 16000.0;
				m_tunercache.minF = float(m_tuner->rangelow)  * m_tunercache.deltaF;
				m_tunercache.maxF = float(m_tuner->rangehigh) * m_tunercache.deltaF;
				m_tunercache.valid = true;				
				m_signalQuality = float(m_tuner->signal) / 32767.0;
			}
		}
#ifdef HAVE_V4L2
	    // v4l2
		else if (m_caps.version == 2) {
		
			r = ioctl(m_radio_fd, VIDIOC_G_TUNER, m_tuner2);

			if (r == 0) {
				if ((m_tuner2->capability & V4L2_TUNER_CAP_LOW) != 0)
					m_tunercache.deltaF = 1.0 / 16000.0;
				m_tunercache.minF = float(m_tuner2->rangelow)  * m_tunercache.deltaF;
				m_tunercache.maxF = float(m_tuner2->rangehigh) * m_tunercache.deltaF;
				m_tunercache.valid = true;
				m_signalQuality = float(m_tuner2->signal) / 32767.0;
			}
		}
#endif
		else {
			kdDebug() << "V4LRadio::readTunerInfo: don't known how to handle V4L-version " << m_caps.version << endl;
		}
		
		if (r != 0) {
			m_signalQuality = 0;
			kdDebug() << "V4LRadio::readTunerInfo: "
					  << i18n("cannot get tuner info")
					  << " (" << i18n("error") << " " << r << ")"
					  << endl;
		}
	} else {
		m_signalQuality = 0;
	}

	// prevent loops, if noticeXYZ-method is reading my state
	m_blockReadTuner = true;

	if (oldminf != m_tunercache.minF || oldmaxf != m_tunercache.maxF)
		notifyDeviceMinMaxFrequencyChanged(m_tunercache.minF, m_tunercache.maxF);

	if (  ! m_minFrequency && (oldminf != m_tunercache.minF)
	   || ! m_maxFrequency && (oldmaxf != m_tunercache.maxF))
		notifyMinMaxFrequencyChanged(getMinFrequency(), getMaxFrequency());


	if (m_signalQuality != oldq)
		notifySignalQualityChanged(m_signalQuality);
	if ( (m_signalQuality >= m_minQuality) != (oldq >= m_minQuality))
		notifySignalQualityChanged(m_signalQuality > m_minQuality);

	m_blockReadTuner = false;

	return true;
}



#define V4L2_S_CTRL(what,val) \
	ctl.value = (val); \
	ctl.id    = (what); \
	r = ioctl (m_radio_fd, VIDIOC_S_CTRL, &ctl);

#define V4L2_G_CTRL(what) \
	ctl.id    = (what); \
	r = ioctl (m_radio_fd, VIDIOC_G_CTRL, &ctl); \
	x = x ? x : r;


bool V4LRadio::updateAudioInfo(bool write) const
{
	if (m_blockReadAudio && !write)
		return true;

	bool  oldStereo  = m_stereo;
	bool  oldMute    = m_muted;
	float oldTreble  = m_treble;
	float oldBass    = m_bass;
	float oldBalance = m_balance;
	float oldDeviceVolume = m_deviceVolume;

	if (m_radio_fd >= 0) {
		int r = -1;
		if (m_caps.version == 1) {
		    if (m_muted) m_audio->flags |=  VIDEO_AUDIO_MUTE;
		    else         m_audio->flags &= ~VIDEO_AUDIO_MUTE;

		    m_audio->volume  = (int)rint(65535 * m_deviceVolume);
		    m_audio->treble  = (int)rint(65535 * m_treble);
		    m_audio->bass    = (int)rint(65535 * m_bass);
		    m_audio->balance = (int)rint(32767 * m_balance + 32768);

			r = ioctl(m_radio_fd, write ? VIDIOCSAUDIO : VIDIOCGAUDIO, m_audio);

			m_stereo = (r == 0) && ((m_audio->mode  & VIDEO_SOUND_STEREO) != 0);
			m_muted  = (r != 0) || ((m_audio->flags & VIDEO_AUDIO_MUTE) != 0);

			/* Some drivers seem to set volumes to zero if they are muted.
			   Thus we do not reload them if radio is muted */
			if (!m_muted && !write) {
				m_treble  = r ? 0 : (1 / 65535.0 * (float)m_audio->treble);
				m_bass    = r ? 0 : (1 / 65535.0 * (float)m_audio->bass);
				m_balance = r ? 0 : (1 / 32767.0 * (float)(m_audio->balance - 32768));
				m_deviceVolume = r ? 0 : (1 / 65535.0 * (float)m_audio->volume);
			}
		}
#ifdef HAVE_V4L2
		else if (m_caps.version == 2) {
			v4l2_control   ctl;
			if (write) {
				V4L2_S_CTRL(V4L2_CID_AUDIO_MUTE,    m_muted);
				V4L2_S_CTRL(V4L2_CID_AUDIO_TREBLE,  (int)rint(65535 * m_treble));
				V4L2_S_CTRL(V4L2_CID_AUDIO_BASS,    (int)rint(65535 * m_bass));
				V4L2_S_CTRL(V4L2_CID_AUDIO_BALANCE, (int)rint(32767 * m_balance + 32768));
				V4L2_S_CTRL(V4L2_CID_AUDIO_VOLUME,  (int)rint(65535 * m_deviceVolume));
			} else {
				int x = 0;    // x stores first ioctl error
				V4L2_G_CTRL(V4L2_CID_AUDIO_MUTE);
                m_muted   = (r != 0) || ctl.value;

				/* Some drivers seem to set volumes to zero if they are muted.
				   Thus we do not reload them if radio is muted */
				if (!m_muted) {
                
					V4L2_G_CTRL(V4L2_CID_AUDIO_TREBLE);
					m_treble  = r ? 0 : (1 / 65535.0 * (float)ctl.value); 
					V4L2_G_CTRL(V4L2_CID_AUDIO_BASS);
					m_bass    = r ? 0 : (1 / 65535.0 * (float)ctl.value); 
					V4L2_G_CTRL(V4L2_CID_AUDIO_BALANCE);
					m_balance = r ? 0 : (1 / 32767.0 * (float)(ctl.value - 32768));
					V4L2_G_CTRL(V4L2_CID_AUDIO_VOLUME);
					m_deviceVolume = r ? 0 : (1 / 65535.0 * (float)ctl.value);
				}
                
                r = ioctl (m_radio_fd, VIDIOC_G_TUNER, m_tuner2);
                m_stereo = (r == 0) && ((m_tuner2->rxsubchans & V4L2_TUNER_SUB_STEREO) != 0);
                x = x ? x : r;
				r = x;  // store first error back to r, used below for error message
			}
		}
#endif
		else  {
			kdDebug() << "V4LRadio::updateAudioInfo: don't known how to handle V4L-version " << m_caps.version << endl;
		}
		
		if (r) {
			kdDebug() << "V4LRadio::updateAudioInfo: "
					  << i18n("error updating radio audio info") << " ("
					  << i18n(write ? "write" : "read") << ")"
					  << endl;
			return false;
		}
	}

	// prevent loops, if noticeXYZ-method is reading my state
	bool oldBlock = m_blockReadAudio;
	m_blockReadAudio = true;

	// ignore values read if a feature is not supported
	if (!m_caps.hasVolume)   m_deviceVolume = 1.0;
	if (!m_caps.hasTreble)   m_treble       = 1.0;
	if (!m_caps.hasBass  )   m_bass         = 1.0;
	if (!m_caps.hasBalance)  m_balance      = 0.0;

    // send notifications
	
	if (oldStereo != m_stereo)
		notifyStereoChanged(m_stereo);
	if (oldMute != m_muted)
		notifyMuted(m_muted);
	if ((int)rint(65535*oldTreble)       != (int)rint(65535*m_treble))
		notifyTrebleChanged(m_treble);
	if ((int)rint(65535*oldBass)         != (int)rint(65535*m_bass))
		notifyBassChanged(m_bass);
	if ((int)rint(32767*oldBalance)      != (int)rint(32767*m_balance))
		notifyBalanceChanged(m_balance);
	if ((int)rint(65535*oldDeviceVolume) != (int)rint(65535*m_deviceVolume))
		notifyDeviceVolumeChanged(m_deviceVolume);
	
	m_blockReadAudio = oldBlock;

	return (m_radio_fd >= 0);
}




void V4LRadio::poll()
{
	readTunerInfo();
	readAudioInfo();
	getVolume();
}


