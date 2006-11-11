/***************************************************************************
                          alsa-mixer-element.cpp  -  description
                             -------------------
    begin                : Mon Aug 15 2005
    copyright            : (C) 2005 by Martin Witte
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

#include "alsa-mixer-element.h"

#include <knuminput.h>
#include <qslider.h>
#include <qlabel.h>
#include <qcheckbox.h>

#include <math.h>

QAlsaMixerElement::QAlsaMixerElement(QWidget *parent, const QString &label, bool has_switch, bool has_volume)
    : AlsaMixerElementUI(parent),
      m_HasVolume(has_volume),
      m_HasSwitch(has_switch),
      m_dirty(false),
      m_ignore_updates(false)
{
    setLabel(label);
    setVolume(0);

    QObject::connect(m_spinboxVolume, SIGNAL(valueChanged(int)),
                     this,            SLOT  (slotSpinboxValueChanged(int)));
    QObject::connect(m_sliderVolume,  SIGNAL(valueChanged(int)),
                     this,            SLOT  (slotSliderValueChanged(int)));

    if (m_HasVolume) {
        QObject::connect(m_checkboxOverride, SIGNAL(toggled(bool)),
                         m_spinboxVolume,    SLOT  (setEnabled(bool)));
        QObject::connect(m_checkboxOverride, SIGNAL(toggled(bool)),
                         m_sliderVolume,     SLOT  (setEnabled(bool)));
    } else {
        m_spinboxVolume->hide();
        m_sliderVolume->hide();
    }
    if (m_HasSwitch) {
        QObject::connect(m_checkboxOverride, SIGNAL(toggled(bool)),
                         m_checkboxActive,   SLOT  (setEnabled(bool)));
    } else {
        //m_checkboxActive->hide();
        m_checkboxActive->setEnabled(false);
        m_checkboxActive->setChecked(true);
    }

    connect(m_checkboxOverride, SIGNAL(toggled(bool)),     this, SLOT(slotSetDirty()));
    connect(m_checkboxActive,   SIGNAL(toggled(bool)),     this, SLOT(slotSetDirty()));
    connect(m_spinboxVolume,    SIGNAL(valueChanged(int)), this, SLOT(slotSetDirty()));
    connect(m_sliderVolume,     SIGNAL(valueChanged(int)), this, SLOT(slotSetDirty()));
}


QAlsaMixerElement::~QAlsaMixerElement()
{
}

float QAlsaMixerElement::getVolume() const
{
    return ((float)m_spinboxVolume->value())/100.0;
}

bool  QAlsaMixerElement::getActive() const
{
    return m_checkboxActive->isChecked();
}

bool  QAlsaMixerElement::getOverride() const
{
    return m_checkboxOverride->isChecked();
}

void QAlsaMixerElement::setLabel(const QString &label)
{
    m_labelMixerElementName->setText(label);
}

void QAlsaMixerElement::setOverride(bool ov)
{
    m_ignore_updates = true;
    m_checkboxOverride->setChecked(ov);
    m_ignore_updates = false;
}

void QAlsaMixerElement::setActive(bool active)
{
    m_ignore_updates = true;
    m_checkboxActive->setChecked(active);
    m_ignore_updates = false;
}

void QAlsaMixerElement::setVolume(float vol)
{
    m_ignore_updates = true;
    int v = (int)rint(vol*100 + 0.5);
    m_sliderVolume->setValue(100 - v);
    m_spinboxVolume->setValue(v);
    m_ignore_updates = false;
}

void QAlsaMixerElement::slotSpinboxValueChanged(int v)
{
    m_sliderVolume->setValue(100-v);
}

void QAlsaMixerElement::slotSliderValueChanged(int v)
{
    m_spinboxVolume->setValue(100-v);
}


void QAlsaMixerElement::slotSetDirty()
{
    if (!m_dirty && !m_ignore_updates) {
        m_dirty = true;
        emit sigDirty();
    }
}


void QAlsaMixerElement::slotResetDirty()
{
    m_dirty = false;
}

#include "alsa-mixer-element.moc"
