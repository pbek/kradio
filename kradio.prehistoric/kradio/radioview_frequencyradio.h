/***************************************************************************
                          kradiodisplay.h  -  description
                             -------------------
    begin                : Mit Jan 29 2003
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

#ifndef KRADIO_RADIOVIEW_FREQUENCYRADIO_H
#define KRADIO_RADIOVIEW_FREQUENCYRADIO_H

#include "radioview_element.h"
#include "radiodevice_interfaces.h"

/**
  *@author Martin Witte
  */

class RadioViewFrequencyRadio : public RadioViewElement,  // is a QObject, must be first
					            public IRadioDeviceClient,
					            public IFrequencyRadioClient,
					            public IRadioSoundClient
{
Q_OBJECT
public:
	RadioViewFrequencyRadio(QWidget *parent, const QString &name);
	~RadioViewFrequencyRadio();

	float getUsability (Interface *) const;

// Interface

	bool connect   (Interface *);
	bool disconnect(Interface *);

// IRadioDeviceClient	
RECEIVERS:
	bool noticePowerChanged   (bool on, const IRadioDevice *sender = NULL);
	bool noticeStationChanged (const RadioStation &, const IRadioDevice *sender = NULL);

// IRadioSoundClient	
RECEIVERS:
	bool noticeVolumeChanged(float v);
	bool noticeSignalQualityChanged(float q);
	bool noticeSignalQualityChanged(bool good);
	bool noticeSignalMinQualityChanged(float q);
	bool noticeStereoChanged(bool  s);
	bool noticeMuted(bool m);

// IFrequencyRadioClient
RECEIVERS:
	bool noticeFrequencyChanged(float f, const RadioStation *s);
	bool noticeMinMaxFrequencyChanged(float min, float max);
	bool noticeDeviceMinMaxFrequencyChanged(float min, float max);
	bool noticeScanStepChanged(float s);

// own stuff ;)	

protected:

	void drawContents(QPainter *p);

protected:

	bool  m_power;
	bool  m_valid;
	float m_frequency;
	float m_quality;
	bool  m_stereo;
};

#endif