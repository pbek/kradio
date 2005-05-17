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


// Kopenhagener Wellenplan: 300kHz
#define STATION_FREQ_INTERVAL_FM   0.3

// Kopenhagener Wellenplan:   9kHz
#define STATION_FREQ_INTERVAL_AM   0.009

class FrequencyRadioStation : public RadioStation  {
public:
    FrequencyRadioStation ();
    FrequencyRadioStation (float frequency);
    FrequencyRadioStation (const QString &name, const QString &shortName, float frequency);
    FrequencyRadioStation (const FrequencyRadioStation &);
    FrequencyRadioStation (RegisterStationClass, const QString &classname = QString::null);
    ~FrequencyRadioStation();

    float  frequency()  const             { return m_frequency; }
    void   setFrequency (float frequency) { m_frequency = frequency; }

    virtual QString    longName() const;
    virtual QString    description() const;
    virtual bool    isValid () const;

    /*  = 0 : "this" is same as "s", i.e. approximately same frequency
        > 0 : this.frequency > s.frequency
        < 0 : this.frequency < s.frequency
        other class than FrequencyRadioStation: compare typeid(.).name()
    */
    virtual int    compare (const RadioStation &s) const;

    /** returns an exact copy of this station */
    virtual RadioStation *copy() const;

    virtual RadioStationConfig *createEditor() const;

    // for XML-Parsing/Export
    virtual bool setProperty(const QString &property_name, const QString &val);
    virtual QString getProperty(const QString &property_name) const;
    virtual QStringList getPropertyNames() const;
    virtual QString getClassName() const { return "FrequencyRadioStation"; }

protected:

    float m_frequency;
};



#endif
