/***************************************************************************
                          radiostation-config.cpp  -  description
                             -------------------
    begin                : Sa Aug 16 2003
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

#include <qlabel.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <klocale.h>

#include <math.h>

#include "radiostation-config.h"

RadioStationConfig::RadioStationConfig(QWidget *parent)
    : QWidget (parent)
{

}

RadioStationConfig::~RadioStationConfig()
{
}


///////////////////////////////////////////////////////////////////////

UndefinedRadioStationConfig::UndefinedRadioStationConfig (QWidget *parent)
    : RadioStationConfig(parent)
{
    new QLabel (i18n("I don't know how to edit this station"), this);
}

UndefinedRadioStationConfig::~UndefinedRadioStationConfig()
{
}

void UndefinedRadioStationConfig::setStationData   (const RadioStation &/*rs*/)
{
}

void UndefinedRadioStationConfig::storeStationData (RadioStation &/*rs*/)
{
}


///////////////////////////////////////////////////////////////////////

#include <kradio/radio-stations/frequencyradiostation.h>

FrequencyRadioStationConfig::FrequencyRadioStationConfig (QWidget *parent)
    : RadioStationConfig(parent)
{
    QHBoxLayout *hl = new QHBoxLayout(this);
    QVBoxLayout *vl = new QVBoxLayout(hl);
    vl->addWidget (new QLabel(i18n("Frequency:"), this));
    m_editFrequency = new QSpinBox(20, 150000, 10, this);
    vl->addWidget (m_editFrequency);
    hl->addItem(new QSpacerItem (10, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));

    connect (m_editFrequency, SIGNAL(valueChanged(int)), this, SLOT(slotValueChanged(int)));
}

FrequencyRadioStationConfig::~FrequencyRadioStationConfig()
{
}

void FrequencyRadioStationConfig::setStationData   (const RadioStation &x)
{
    const FrequencyRadioStation *rs = dynamic_cast<const FrequencyRadioStation*>(&x);
    if (rs) {
        m_editFrequency->setValue((int)rint(rs->frequency() * 1000));
    }
}

void FrequencyRadioStationConfig::storeStationData (RadioStation &x)
{
    FrequencyRadioStation *rs = dynamic_cast<FrequencyRadioStation*>(&x);
    if (rs) {
        rs->setFrequency(0.001 * m_editFrequency->value());
    }
}

void FrequencyRadioStationConfig::slotValueChanged(int /*i*/)
{
    emit changed(this);
}


#include radiostation-config.moc


#include "radiostation-config.moc"
