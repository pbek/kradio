/***************************************************************************
                          radioview_frequencyseeker.cpp  -  description
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

#include <math.h>
#include <qlayout.h>
#include <qslider.h>
#include <qtoolbutton.h>
#include <qaccel.h>

#include <kiconloader.h>

#include "radioview_frequencyseeker.h"

RadioViewFrequencySeeker::RadioViewFrequencySeeker(QWidget *parent, const QString &name)
  : RadioViewElement(parent, name, clsRadioSeek),
    m_btnSearchLeft(NULL),
    m_btnStepLeft(NULL),
    m_btnStepRight(NULL),
    m_btnSearchRight(NULL),
    m_sldFrequency(NULL),
    m_ignoreChanges(false)
{
	QBoxLayout *l = new QBoxLayout(this, QBoxLayout::LeftToRight, /*spacing=*/ 3);
	l->setMargin(0);

	m_sldFrequency   = new QSlider(Qt::Horizontal, this);
	m_btnSearchLeft  = new QToolButton(this);
	m_btnSearchRight = new QToolButton(this);
	m_btnStepLeft    = new QToolButton(this);
	m_btnStepRight   = new QToolButton(this);

	m_btnSearchLeft ->setToggleButton(true);
	m_btnSearchRight->setToggleButton(true);
	m_sldFrequency->setPageStep(1);

	m_btnSearchLeft ->setIconSet(SmallIconSet("2leftarrow"));
	m_btnSearchRight->setIconSet(SmallIconSet("2rightarrow"));
	m_btnStepLeft   ->setIconSet(SmallIconSet("1leftarrow"));
	m_btnStepRight  ->setIconSet(SmallIconSet("1rightarrow"));

	l->addWidget (m_btnSearchLeft);
	l->addWidget (m_btnStepLeft);
	l->addWidget (m_sldFrequency);
	l->addWidget (m_btnStepRight);
	l->addWidget (m_btnSearchRight);

	QObject::connect(m_sldFrequency,   SIGNAL(valueChanged(int)),
	                 this,               SLOT(slotSliderChanged(int)));
	QObject::connect(m_btnSearchLeft,  SIGNAL(toggled(bool)),
					 this,               SLOT(slotSearchLeft(bool)));
	QObject::connect(m_btnSearchRight, SIGNAL(toggled(bool)),
					 this,               SLOT(slotSearchRight(bool)));
	QObject::connect(m_btnStepLeft,    SIGNAL(clicked()),
					 m_sldFrequency,     SLOT(subtractStep()));
	QObject::connect(m_btnStepRight,   SIGNAL(clicked()),
					 m_sldFrequency,     SLOT(addStep()));

    QAccel *Accel = new QAccel (this);
    Accel->insertItem (Key_Left,  100);
    Accel->insertItem (Key_Right, 101);
    Accel->connectItem (100, m_sldFrequency, SLOT(subtractStep()));
    Accel->connectItem (101, m_sldFrequency, SLOT(addStep()));
}


RadioViewFrequencySeeker::~RadioViewFrequencySeeker()
{
}


float RadioViewFrequencySeeker::getUsability (Interface *i) const
{
	if (dynamic_cast<IFrequencyRadio*>(i))
		return 0.9;
	else
		return 0.0;
}


// Interface

bool RadioViewFrequencySeeker::connect   (Interface *i)
{
	if (IFrequencyRadioClient::connect(i)) {
		ISeekRadioClient::connect(i);
		return true;
	} else {
		return false;
	}	
}


bool RadioViewFrequencySeeker::disconnect(Interface *i)
{
	bool a = IFrequencyRadioClient::disconnect(i);
	bool b = ISeekRadioClient::disconnect(i);
	return a || b;
}



// ISeekRadioClient

bool RadioViewFrequencySeeker::noticeSeekStarted (bool up)
{
	m_ignoreChanges = true;
	m_btnSearchLeft->setOn(!up);
	m_btnSearchRight->setOn(up);
	m_ignoreChanges = false;
	return true;
}


bool RadioViewFrequencySeeker::noticeSeekStopped ()
{
	m_ignoreChanges = true;
	m_btnSearchLeft->setOn(false);
	m_btnSearchRight->setOn(false);
	m_ignoreChanges = false;
	return true;
}


bool RadioViewFrequencySeeker::noticeSeekFinished (const RadioStation &/*s*/)
{
	m_ignoreChanges = true;
	m_btnSearchLeft->setOn(false);
	m_btnSearchRight->setOn(false);
	m_ignoreChanges = false;
	return true;
}



// IFrequencyRadioClient

bool RadioViewFrequencySeeker::noticeFrequencyChanged(float f, const RadioStation */*s*/)
{
	float step = queryScanStep();
	if (step == 0) step = 0.000001;
	
	m_ignoreChanges = true;
	m_sldFrequency->setValue((int)rint(f / step));
	m_ignoreChanges = false;
	return true;
}


bool RadioViewFrequencySeeker::noticeMinMaxFrequencyChanged(float min, float max)
{
	float step = queryScanStep();
	if (step == 0) step = 0.000001;

	m_ignoreChanges = true;
	m_sldFrequency->setMinValue((int)rint(min / step));
	m_sldFrequency->setMaxValue((int)rint(max / step));
	m_sldFrequency->setValue   ((int)rint(queryFrequency() / step));
	m_ignoreChanges = false;
	return true;
}


bool RadioViewFrequencySeeker::noticeDeviceMinMaxFrequencyChanged(float /*min*/, float /*max*/)
{
	return false; // we don't care
}


bool RadioViewFrequencySeeker::noticeScanStepChanged(float s)
{
	if (s == 0) s = 0.000001;
	m_ignoreChanges = true;
	m_sldFrequency->setMinValue((int)rint(queryMinFrequency() / s));
	m_sldFrequency->setMaxValue((int)rint(queryMaxFrequency() / s));
	m_sldFrequency->setValue   ((int)rint(queryFrequency() / s));
	m_ignoreChanges = false;
	return true;
}


void RadioViewFrequencySeeker::slotSearchLeft(bool on)
{
	if (m_ignoreChanges) return;
	if (on) {
		if (queryIsSeekUpRunning())
			sendStopSeek();
		if (!queryIsSeekRunning())
			sendStartSeekDown();
	} else {
		if (queryIsSeekDownRunning())
			sendStopSeek();
	}
	if (!queryIsSeekDownRunning())
		m_btnSearchLeft->setOn(false);
}


void RadioViewFrequencySeeker::slotSearchRight(bool on)
{
	if (m_ignoreChanges) return;
	if (on) {
		if (queryIsSeekDownRunning())
			sendStopSeek();
		if (!queryIsSeekRunning())
			sendStartSeekUp();
	} else {
		if (queryIsSeekUpRunning())
			sendStopSeek();
	}
	if (!queryIsSeekUpRunning())
		m_btnSearchRight->setOn(false);
}


void RadioViewFrequencySeeker::slotSliderChanged(int val)
{
	if (m_ignoreChanges) return;
	sendFrequency(val * queryScanStep());
}


