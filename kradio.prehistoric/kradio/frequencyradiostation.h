/***************************************************************************
                          frequencyradiostation.h  -  description
                             -------------------
    begin                : Sat March 29 2003
    copyright            : (C) 2003 by Klas Kalass, Ernst Martin Witte
    email                : klas@kde.org, witte@kawo1.rwth-aachen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRADIO_FREQUENCYRADIOSTATION_H
#define KRADIO_FREQUENCYRADIOSTATION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "radiostation.h"

/**
 * @author Klas Kalass, Ernst Martin Witte
 */

class FrequencyRadioStation : public RadioStation  {
public:
    FrequencyRadioStation (const QString &name, const QString &shortName, float frequency);
    FrequencyRadioStation (const FrequencyRadioStation &);
    ~FrequencyRadioStation();

    float  frequency()  const;
    void   setFrequency (float frequency);

    virtual QString	longName() const;

    virtual bool equals(const RadioStation &s) const;
    virtual bool isValid() const;

    /** returns an exact copy of this station */
    virtual RadioStation *copy() const;

protected:

    float m_frequency;
};

#endif
