/***************************************************************************
                          frequencyradiostation.h  -  description
                             -------------------
    begin                : Sat March 29 2003
    copyright            : (C) 2003 by Klas Kalass
    email                : klas@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FREQUENCYRADIOSTATION_H
#define FREQUENCYRADIOSTATION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "radiostation.h"

// forward declarations
class FrequencyRadio;
class RadioDevice;

/**
 * @author Klas Kalass
 */

class FrequencyRadioStation : public RadioStation  {
public:
    FrequencyRadioStation(QObject *parent, QString name, FrequencyRadio* radio, float frequency);
    FrequencyRadioStation(QObject *parent, FrequencyRadioStation const &);
    FrequencyRadioStation(FrequencyRadioStation const &);
    ~FrequencyRadioStation();

    float frequency() const {return m_frequency;};
    void setFrequency(float frequency) {m_frequency = frequency;};

    // implementation of the RadioStation pure virtuals
    RadioDevice *radio();

    QString	longName() const;
    bool frequencyMatch(float frequency);
    bool isValid() const;

    /** returns an exact copy of this station, but the parent is the one given */
    virtual RadioStation *copy(QObject *parent);

public slots:
    void slotActivate();

protected:
    FrequencyRadio *m_frequencyRadio;
    float m_frequency;
};

#endif
