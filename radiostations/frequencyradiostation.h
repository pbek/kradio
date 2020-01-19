/***************************************************************************
                          frequencyradiostation.h  -  description
                             -------------------
    begin                : Sat March 29 2003
    copyright            : (C) 2003 by Klas Kalass, Ernst Martin Witte
    email                : klas@kde.org, emw-kradio@nocabal.de
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

#include "radiostation.h"

/**
 * @author Klas Kalass, Ernst Martin Witte
 */


// Kopenhagener Wellenplan: 300kHz
#define STATION_FREQ_INTERVAL_FM   0.3

// Kopenhagener Wellenplan:   9kHz
#define STATION_FREQ_INTERVAL_AM   0.009

class KRADIO5_EXPORT FrequencyRadioStation : public RadioStation  {
public:
    FrequencyRadioStation ();
    FrequencyRadioStation (float frequency);
    FrequencyRadioStation (const QString &name, const QString &shortName, float frequency);
    FrequencyRadioStation (const FrequencyRadioStation &);
    FrequencyRadioStation (RegisterStationClass, const QString &classname = QString());
    ~FrequencyRadioStation();

    FrequencyRadioStation & operator = (const FrequencyRadioStation &) = default;

            float      frequency    () const          { return m_frequency; }
            void       setFrequency (float frequency) { m_frequency = frequency; }

    virtual QString    longName()    const override;
    virtual QString    description() const override;
    virtual bool       isValid ()    const override;

    /*  = 0 : "this" is same as "s", i.e. approximately same frequency
        > 0 : this.frequency > s.frequency
        < 0 : this.frequency < s.frequency
        other class than FrequencyRadioStation: compare typeid(.).name()
    */
    virtual int    compare (const RadioStation &s) const override;

    /** returns an exact copy of this station */
    virtual RadioStation *copy()      const override;
    virtual RadioStation *copyNewID() const override;

    virtual RadioStationConfig *createEditor() const override;

    // for XML-Parsing/Export
    virtual bool        setProperty(const QString &property_name, const QString &val) override;
    virtual QString     getProperty(const QString &property_name) const               override;
    virtual QStringList getPropertyNames() const                                      override;
    virtual QString     getClassName()        const override { return QString::fromLatin1("FrequencyRadioStation"); }
    virtual QString     getClassDescription() const override { return i18n("AM/FM Radio Station"); }
    virtual bool        isClassUserVisible()  const override { return true; }

    virtual bool        frequencyMatches(const FrequencyRadioStation &x) const;
    virtual bool        operator == (const RadioStation &x) const override;

protected:

    float m_frequency;
};



#endif
