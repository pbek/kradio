/***************************************************************************
                          frequencyseekhelper.cpp  -  description
                             -------------------
    begin                : Fre Mai 9 2003
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

#include "frequencyseekhelper.h"
#include <qtimer.h>

#include <kdebug.h>
  
FrequencySeekHelper::FrequencySeekHelper(ISeekRadio &parent)
  : SeekHelper(parent)
{
	m_timer = new QTimer(this);
	QObject::connect (m_timer, SIGNAL(timeout()), this, SLOT(step()));
}


FrequencySeekHelper::~FrequencySeekHelper()
{
	delete m_timer;
}


bool FrequencySeekHelper::connect   (Interface *i)
{
	bool a = SeekHelper::connect(i);
	bool b = IFrequencyRadioClient::connect(i);

    if (a) kdDebug() << "FrequencySeekHelper: SeekHelper connected\n";
    if (b) kdDebug() << "FrequencySeekHelper: IFrequencyRadioClient connected\n";

	return a || b;
}


bool FrequencySeekHelper::disconnect(Interface *i)
{
	bool a = SeekHelper::disconnect(i);
	bool b = IFrequencyRadioClient::disconnect(i);

	return a || b;
}


void FrequencySeekHelper::abort()
{
	m_timer->stop();
	m_bestFrequency = 0;
}


bool FrequencySeekHelper::nextSeekStep()
{
	float f = queryFrequency();
	f += (m_direction == up ? 1 : -1) * queryScanStep();
	if (f > queryMaxFrequency() || f < queryMinFrequency())
		return false;

	if (sendFrequency(f) > 0) {
		m_timer->start (50, true);
		return true;
	} else {
		return false;
	}
}


bool FrequencySeekHelper::bestFound() const
{
	return m_bestFrequency > 0;
}


void FrequencySeekHelper::rememberBest()
{
	m_bestFrequency = m_currentFrequency;
}


void FrequencySeekHelper::getData()
{
	m_oldSignal        = m_currentSignal;
	m_oldFrequency     = m_currentFrequency;

	m_currentSignal    = querySignalQuality();
	m_goodSignal       = queryHasGoodQuality();
	m_currentFrequency = queryFrequency();
}


bool FrequencySeekHelper::isBetter() const
{
	return m_currentSignal > m_oldSignal;
}


bool FrequencySeekHelper::isWorse() const
{
	return m_currentSignal < m_oldSignal;
}


bool FrequencySeekHelper::isGood() const
{
	return m_goodSignal;
}


void FrequencySeekHelper::applyBest()
{
	sendFrequency( (m_bestFrequency + m_currentFrequency) / 2);
}


