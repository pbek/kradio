/***************************************************************************
                          radioview_frequencyseeker.cpp  -  description
                             -------------------
    begin                : Fre Jun 20 2003
    copyright            : (C) 2003 by Martin Witte
    email                : emw-kradio@nocabal.de
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
#include <QLayout>
#include <QSlider>
#include <QToolButton>
// #include <QAccel>

#include <klocalizedstring.h>
#include <kicon.h>

#include "radioview_frequencyseeker.h"

RadioViewFrequencySeeker::RadioViewFrequencySeeker(QWidget *parent, const QString &name)
  : RadioViewElement(parent, name, clsRadioSeek),
    m_btnSearchLeft(NULL),
    m_btnStepLeft(NULL),
    m_btnStepRight(NULL),
    m_btnSearchRight(NULL),
    m_sldFrequency(NULL),
    m_ignoreChanges(false),
    m_accelLeft(parent),
    m_accelRight(parent)
{
    QHBoxLayout *l = new QHBoxLayout(this);
    l->setSpacing(1);
    l->setMargin(0);

    m_sldFrequency   = new QSlider(Qt::Horizontal, this);
    m_btnSearchLeft  = new QToolButton(this);
    m_btnSearchRight = new QToolButton(this);
    m_btnStepLeft    = new QToolButton(this);
    m_btnStepRight   = new QToolButton(this);

    m_btnSearchLeft ->setCheckable(true);
    m_btnSearchRight->setCheckable(true);
    m_sldFrequency  ->setPageStep(1);

    m_btnSearchLeft ->setIcon(KIcon("media-skip-backward"));
    m_btnSearchRight->setIcon(KIcon("media-skip-forward"));
    m_btnStepLeft   ->setIcon(KIcon("media-seek-backward"));
    m_btnStepRight  ->setIcon(KIcon("media-seek-forward"));

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
                     this,               SLOT(slotStepDown()));
    QObject::connect(m_btnStepRight,   SIGNAL(clicked()),
                     this,               SLOT(slotStepUp()));

    // Tooltips

    m_btnSearchLeft ->setToolTip(i18n("Search for previous radio station"));
    m_btnSearchRight->setToolTip(i18n("Search for next radio station"));
    m_btnStepLeft   ->setToolTip(i18n("Decrement frequency"));
    m_btnStepRight  ->setToolTip(i18n("Increment frequency"));
    m_sldFrequency  ->setToolTip(i18n("Change frequency"));

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
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

bool RadioViewFrequencySeeker::connectI   (Interface *i)
{
    if (IFrequencyRadioClient::connectI(i)) {
        ISeekRadioClient::connectI(i);
        return true;
    } else {
        return false;
    }
}


bool RadioViewFrequencySeeker::disconnectI(Interface *i)
{
    bool a = IFrequencyRadioClient::disconnectI(i);
    bool b = ISeekRadioClient::disconnectI(i);
    return a || b;
}



// ISeekRadioClient

bool RadioViewFrequencySeeker::noticeSeekStarted (bool up)
{
    m_ignoreChanges = true;
    m_btnSearchLeft->setChecked(!up);
    m_btnSearchRight->setChecked(up);
    m_ignoreChanges = false;
    return true;
}


bool RadioViewFrequencySeeker::noticeSeekStopped ()
{
    m_ignoreChanges = true;
    m_btnSearchLeft->setChecked(false);
    m_btnSearchRight->setChecked(false);
    m_ignoreChanges = false;
    return true;
}


bool RadioViewFrequencySeeker::noticeSeekFinished (const RadioStation &/*s*/, bool /*goodQuality*/)
{
    m_ignoreChanges = true;
    m_btnSearchLeft->setChecked(false);
    m_btnSearchRight->setChecked(false);
    m_ignoreChanges = false;
    return true;
}



// IFrequencyRadioClient

bool RadioViewFrequencySeeker::noticeFrequencyChanged(float f, const FrequencyRadioStation */*s*/)
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
    m_sldFrequency->setMinimum((int)rint(min / step));
    m_sldFrequency->setMaximum((int)rint(max / step));
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
    m_sldFrequency->setMinimum((int)rint(queryMinFrequency() / s));
    m_sldFrequency->setMaximum((int)rint(queryMaxFrequency() / s));
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
        m_btnSearchLeft->setChecked(false);
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
        m_btnSearchRight->setChecked(false);
}


void RadioViewFrequencySeeker::slotSliderChanged(int val)
{
    if (m_ignoreChanges) return;
    sendFrequency(val * queryScanStep());
}


void RadioViewFrequencySeeker::slotStepUp()
{
    m_sldFrequency->triggerAction(QAbstractSlider::SliderSingleStepAdd);
}

void RadioViewFrequencySeeker::slotStepDown()
{
    m_sldFrequency->triggerAction(QAbstractSlider::SliderSingleStepSub);
}


#include "radioview_frequencyseeker.moc"
