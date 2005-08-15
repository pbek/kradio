/***************************************************************************
                          alsa-mixer-element.h  -  description
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

#ifndef __KRADIO_ALSA_MIXER_ELEMENT_H
#define __KRADIO_ALSA_MIXER_ELEMENT_H

#include "alsa-mixer-element-ui.h"

class QAlsaMixerElement : public AlsaMixerElementUI
{
Q_OBJECT
public:
    QAlsaMixerElement(QWidget *parent, const QString &label, bool has_switch, bool has_volume);
    ~QAlsaMixerElement();


    float getVolume() const;
    bool  getActive() const;
    bool  getOverride() const;

public slots:

    void setLabel(const QString &label);
    void setOverride(bool ov);
    void setActive(bool active);
    void setVolume(float vol);

protected slots:
    void slotSpinboxValueChanged(int v);
    void slotSliderValueChanged(int v);

protected:

    bool m_HasVolume;
    bool m_HasSwitch;
};

#endif
