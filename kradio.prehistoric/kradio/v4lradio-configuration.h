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
                              public IRadioSoundClient
{
Q_OBJECT
public :
	V4LRadioConfiguration (QWidget *parent, V4LRadio *radio);
	~V4LRadioConfiguration ();

	bool connect (Interface *i);
	bool disconnect (Interface *i);

// IV4LCfgClient

RECEIVERS:
	bool noticeRadioDeviceChanged(const QString &s);
	bool noticeMixerDeviceChanged(const QString &s, int Channel);

// IFrequencyRadioClient

RECEIVERS:
	bool noticeFrequencyChanged(float f, const RadioStation *s);
	bool noticeMinMaxFrequencyChanged(float min, float max);
	bool noticeDeviceMinMaxFrequencyChanged(float min, float max);
	bool noticeScanStepChanged(float s);

RECEIVERS:
	bool noticeVolumeChanged(float v);
	bool noticeSignalQualityChanged(float q);
	bool noticeSignalQualityChanged(bool good);
	bool noticeSignalMinQualityChanged(float q);
	bool noticeStereoChanged(bool  s);
	bool noticeMuted(bool m);

protected slots:

	void selectMixerDevice();
	void selectRadioDevice();

	void slotOk();
	void slotCancel();

protected:

	int m_mixerChannelMask;

};

#endif
