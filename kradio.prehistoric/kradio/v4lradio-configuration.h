/***************************************************************************
                          v4lradio-configuration.h  -  description
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

#ifndef KRADIO_V4LRADIO_CONFIGURATION_H
#define KRADIO_V4LRADIO_CONFIGURATION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "v4lradio-configuration-ui.h"
#include "v4lcfg_interfaces.h"
#include "radiodevice_interfaces.h"

class V4LRadio;
class QWidget;

class V4LRadioConfiguration : public V4LRadioConfigurationUI,
                              public IV4LCfgClient,
                              public IFrequencyRadioClient,
                              public IRadioSoundClient,
                              public IRadioDeviceClient
{
Q_OBJECT
public :
	V4LRadioConfiguration (QWidget *parent);
	~V4LRadioConfiguration ();

	bool connect (Interface *i);
	bool disconnect (Interface *i);

// IV4LCfgClient

RECEIVERS:
	bool noticeRadioDeviceChanged(const QString &s);
	bool noticeMixerDeviceChanged(const QString &s, int Channel);
	bool noticeDeviceVolumeChanged(float v);
	bool noticeCapabilitiesChanged(const V4LCaps &c);

// IRadiODeviceClient

RECEIVERS:
	bool noticePowerChanged   (bool /*on*/, const IRadioDevice */*sender = NULL*/)          { return false; }
	bool noticeStationChanged (const RadioStation &, const IRadioDevice */*sender = NULL*/) { return false; }
	bool noticeDescriptionChanged (const QString &, const IRadioDevice *sender = NULL);

// IFrequencyRadioClient

RECEIVERS:
	bool noticeFrequencyChanged(float f, const RadioStation *s);
	bool noticeMinMaxFrequencyChanged(float min, float max);
	bool noticeDeviceMinMaxFrequencyChanged(float min, float max);
	bool noticeScanStepChanged(float s);

RECEIVERS:
	bool noticeVolumeChanged(float v);
	bool noticeTrebleChanged(float t);
	bool noticeBassChanged(float b);
	bool noticeBalanceChanged(float b);
	bool noticeSignalQualityChanged(float q);
	bool noticeSignalQualityChanged(bool good);
	bool noticeSignalMinQualityChanged(float q);
	bool noticeStereoChanged(bool  s);
	bool noticeMuted(bool m);

protected:

	bool eventFilter(QObject *o, QEvent *e);

protected slots:

	void selectMixerDevice();
	void selectRadioDevice();
	void slotEditRadioDeviceChanged();
	void slotEditMixerDeviceChanged();

	void slotOK();
	void slotCancel();

	void guiMinFrequencyChanged(int v);
	void guiMaxFrequencyChanged(int v);

	void slotDeviceVolumeChanged (double v); // for KDoubleNumInput,  0.0..1.0
	void slotTrebleChanged (double t);       // for KDoubleNumInput,  0.0..1.0
	void slotBassChanged   (double b);       // for KDoubleNumInput,  0.0..1.0
	void slotBalanceChanged(double b);       // for KDoubleNumInput, -1.0..1.0

	void slotDeviceVolumeChanged (int v);    // for slider, 0..65535
	void slotTrebleChanged (int t);          // for slider, 0..65535
	void slotBassChanged   (int b);          // for slider, 0..65535
	void slotBalanceChanged(int b);          // for slider, 0..65535

protected:

	int     m_mixerChannelMask;
	bool    m_ignoreGUIChanges;
	
	int     m_myControlChange;
	float   m_orgTreble,
	        m_orgBass,
	        m_orgBalance,
	        m_orgDeviceVolume;

	V4LCaps m_caps;
};

#endif
