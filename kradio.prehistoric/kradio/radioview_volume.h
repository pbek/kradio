/***************************************************************************
                          radioview_volume.h  -  description
                             -------------------
    begin                : Don Jun 19 2003
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

#ifndef KRADIO_RADIOVIEW_VOLUME_H
#define KRADIO_RADIOVIEW_VOLUME_H

#include "radioview_element.h"
#include "radiodevice_interfaces.h"

/**
  *@author Martin Witte
  */

class QSlider;
  
class RadioViewVolume : public RadioViewElement,  // is a QObject, must be first
					    public IRadioSoundClient
{
Q_OBJECT
public:
	RadioViewVolume(QWidget *parent, const QString &name);
	~RadioViewVolume();

	float getUsability(Interface *) const;

// Interface

	bool connectI   (Interface *);
	bool disconnectI(Interface *);

// IRadioSoundClient
RECEIVERS:
	bool noticeVolumeChanged(float v);
	bool noticeTrebleChanged(float /*v*/)        { return false; }
	bool noticeBassChanged(float /*v*/)          { return false; }
	bool noticeBalanceChanged(float /*v*/)       { return false; }
	bool noticeSignalQualityChanged(float q);
	bool noticeSignalQualityChanged(bool good);
	bool noticeSignalMinQualityChanged(float q);
	bool noticeStereoChanged(bool  s);
	bool noticeMuted(bool m);

// own stuff
protected slots:

	void slotVolumeChanged(int val);	

protected:

	int   getSlider4Volume(float volume);
	float getVolume4Slider(int sl);

	QSlider  *m_slider;
	bool      m_handlingSlot;

};

#endif
