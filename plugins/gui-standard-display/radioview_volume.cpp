/***************************************************************************
                          radioview_volume.cpp  -  description
                             -------------------
    begin                : Don Jun 19 2003
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
#include <QtGui/QSlider>
#include <QtGui/QLayout>
#include <QtGui/QToolTip>
// #include <QtGui/QAccel>

#include <klocale.h>

#include "radioview_volume.h"
#include "pluginbase.h"

#define SLIDER_MINVAL   0
#define SLIDER_MAXVAL   32768
#define SLIDER_RANGE    (SLIDER_MAXVAL - SLIDER_MINVAL)

RadioViewVolume::RadioViewVolume(QWidget *parent, const QString &name)
  : RadioViewElement (parent, name, clsRadioSound),
    m_slider(NULL),
    m_handlingSlot(false),
    m_volUpAction(parent),
    m_volDownAction(parent)
{
    float v = 0;
    SoundStreamID ssid = queryCurrentSoundStreamSinkID();
    sendLogDebug (QString ("RadioViewVolume: ssid=%1").arg(ssid.getID()));
    queryPlaybackVolume(ssid, v);
    m_slider = new QSlider(Qt::Vertical, this);
    m_slider->setMinimum(SLIDER_MINVAL);
    m_slider->setMaximum(SLIDER_MAXVAL);
    m_slider->setPageStep(SLIDER_RANGE/10);
    m_slider->setValue(getSlider4Volume(v));

    QObject::connect(m_slider, SIGNAL(valueChanged(int)),
                     this,       SLOT(slotVolumeChanged(int)));

    QHBoxLayout *l = new QHBoxLayout(this);
    l->setSpacing(0);
    l->addWidget(m_slider);

    // Tooltips

    m_slider->setToolTip(i18n("Change volume"));

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
}


RadioViewVolume::~RadioViewVolume()
{
}


float RadioViewVolume::getUsability (Interface */*i*/) const
{
    return 0.5;  // there could be more features like mute control, capture settings, ...
}


bool RadioViewVolume::connectI   (Interface *i)
{
    bool a = IRadioDeviceClient::connectI(i);
    bool b = ISoundStreamClient::connectI(i);
    return a || b;
}


bool RadioViewVolume::disconnectI(Interface *i)
{
    bool a = IRadioDeviceClient::disconnectI(i);
    bool b = ISoundStreamClient::disconnectI(i);
    return a || b;
}

void RadioViewVolume::noticeConnectedI (ISoundStreamServer *s, bool pointer_valid)
{
    ISoundStreamClient::noticeConnectedI(s, pointer_valid);
    if (s && pointer_valid) {
        s->register4_notifyPlaybackVolumeChanged(this);
    }
}

// ISoundStreamClient

bool RadioViewVolume::noticePlaybackVolumeChanged(SoundStreamID id, float v)
{
    if (queryCurrentSoundStreamSinkID() != id)
        return false;
    m_slider->setValue(getSlider4Volume(v));
    return true;
}



void RadioViewVolume::slotVolumeChanged(int val)
{
    if (m_handlingSlot) return;
    m_handlingSlot = true;
    SoundStreamID ssid = queryCurrentSoundStreamSinkID();
    sendPlaybackVolume(ssid, getVolume4Slider(val));
    m_handlingSlot = false;
}


int RadioViewVolume::getSlider4Volume(float volume)
{
    if (volume >= 1) volume = 1;
    if (volume < 0)  volume = 0;
    return SLIDER_MINVAL + (int)rint(SLIDER_RANGE * volume);
}


float RadioViewVolume::getVolume4Slider(int sl)
{
    if (sl > SLIDER_MAXVAL) sl = SLIDER_MAXVAL;
    if (sl < SLIDER_MINVAL) sl = SLIDER_MINVAL;
    return (float)(sl - SLIDER_MINVAL) / (float)SLIDER_RANGE;
}


void RadioViewVolume::slotStepUp()
{
    m_slider->triggerAction(QAbstractSlider::SliderSingleStepAdd);
}

void RadioViewVolume::slotStepDown()
{
    m_slider->triggerAction(QAbstractSlider::SliderSingleStepSub);
}


#include "radioview_volume.moc"
