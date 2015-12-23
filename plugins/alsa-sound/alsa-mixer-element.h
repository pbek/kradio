/***************************************************************************
                          alsa-mixer-element.h  -  description
                             -------------------
    begin                : Mon Aug 15 2005
    copyright            : (C) 2005 by Martin Witte
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

#ifndef __KRADIO_ALSA_MIXER_ELEMENT_H
#define __KRADIO_ALSA_MIXER_ELEMENT_H

#include "ui_alsa-mixer-element-ui.h"

#include <QWidget>

class QAlsaMixerElement : public QWidget,
                          public Ui_AlsaMixerElementUI
{
Q_OBJECT
public:
    QAlsaMixerElement(QWidget *parent, const QString &label, bool has_switch, bool has_volume);
    ~QAlsaMixerElement();


    float getVolume() const;
    bool  getActive() const;
    bool  getOverride() const;
    
    bool  isDirty() const { return m_dirty; }

public slots:

    void setLabel(const QString &label);
    void setOverride(bool ov);
    void setActive(bool active);
    void setVolume(float vol);
    void slotResetDirty();
    void slotSetDirty();

protected slots:
    void slotSpinboxValueChanged(int v);
    void slotSliderValueChanged(int v);

signals:

    void sigDirty();

protected:

    bool m_HasVolume;
    bool m_HasSwitch;
    bool m_dirty;
    bool m_ignore_updates;
};

#endif
