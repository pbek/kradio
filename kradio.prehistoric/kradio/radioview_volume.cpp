/***************************************************************************
                          radioview_volume.cpp  -  description
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

#include <math.h>
#include <qslider.h>
#include <qlayout.h>
#include <qaccel.h>
#include <qtooltip.h>
   
#include <klocale.h>

#include "radioview_volume.h"

#define SLIDER_MINVAL   0
#define SLIDER_MAXVAL   32768
#define SLIDER_RANGE    (SLIDER_MAXVAL - SLIDER_MINVAL)

RadioViewVolume::RadioViewVolume(QWidget *parent, const QString &name)
  : RadioViewElement (parent, name, clsRadioSound),
	m_slider(NULL),
	m_handlingSlot(false)
{
	m_slider = new QSlider(SLIDER_MINVAL,
					       SLIDER_MAXVAL,
					       SLIDER_RANGE/10,
					       getSlider4Volume(queryVolume()),
					       Qt::Vertical, this);

	QObject::connect(m_slider, SIGNAL(valueChanged(int)),
					 this,     SLOT(slotVolumeChanged(int)));

	QBoxLayout *l = new QBoxLayout(this, QBoxLayout::LeftToRight);
	l->addWidget(m_slider);

	// Tooltips
	
    QToolTip::add(m_slider, i18n("Change Volume"));

	// Accelerators
    QAccel *Accel = new QAccel (this);
    Accel->insertItem (Key_Up,  100);
    Accel->insertItem (Key_Down, 101);
    Accel->connectItem (100, m_slider, SLOT(subtractStep()));
    Accel->connectItem (101, m_slider, SLOT(addStep()));
}


RadioViewVolume::~RadioViewVolume()
{
}


float RadioViewVolume::getUsability (Interface *i) const
{
	if (dynamic_cast<IRadioSound*>(i))
		return 0.5;  // there could be more features like mute control, capture settings, ...
	else
		return 0.0;
}


bool RadioViewVolume::connect   (Interface *i)
{
	return IRadioSoundClient::connect(i);
}


bool RadioViewVolume::disconnect(Interface *i)
{
	return IRadioSoundClient::disconnect(i);
}



// IRadioSoundClient
bool RadioViewVolume::noticeVolumeChanged(float v)
{
	m_slider->setValue(getSlider4Volume(v));
	return true;
}


bool RadioViewVolume::noticeSignalQualityChanged(float /*q*/)
{
	return false;  // we don't care
}


bool RadioViewVolume::noticeSignalQualityChanged(bool /*good*/)
{
	return false;  // we don't care
}


bool RadioViewVolume::noticeSignalMinQualityChanged(float /*q*/)
{
	return false;  // we don't care
}


bool RadioViewVolume::noticeStereoChanged(bool  /*s*/)
{
	return false;  // we don't care
}


bool RadioViewVolume::noticeMuted(bool /*m*/)
{
	return false;  // we don't care
}


void RadioViewVolume::slotVolumeChanged(int val)
{
	if (m_handlingSlot) return;
	m_handlingSlot = true;
	sendVolume(getVolume4Slider(val));
	m_handlingSlot = false;
}


int RadioViewVolume::getSlider4Volume(float volume)
{
	if (volume >= 1) volume = 1;
	if (volume < 0) volume = 0;
    return SLIDER_MAXVAL - (int)rint(SLIDER_RANGE * volume);
}


float RadioViewVolume::getVolume4Slider(int sl)
{
	if (sl > SLIDER_MAXVAL) sl = SLIDER_MAXVAL;
	if (sl < SLIDER_MINVAL) sl = SLIDER_MINVAL;
    return (float)(SLIDER_MAXVAL - sl) / (float)SLIDER_RANGE;
}

