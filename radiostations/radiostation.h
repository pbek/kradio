/***************************************************************************
                          radiostation.h  -  description
                             -------------------
    begin                : Sat Feb 2 2002
    copyright            : (C) 2003 by Martin Witte, Klas Kalass
    email                : emw-kradio@nocabal.de / klas@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRADIO_RADIOSTATION_H
#define KRADIO_RADIOSTATION_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QMap>

#include <klocalizedstring.h>

#include "kradio-def.h"

/**
  *@author Martin Witte, Klas Kalass
  */

/*

   RadioStation

   RadioStation is an abstract base class for any type of radio station,
   e.g. AM/FM stations or internet radio stations. Thus no specific knowledge
   about the frequency or URL is included in this class. A radio station
   should not contain information on a matching device as well. The device has
   to decide on its own to use or not to use a station.

   There are some important abstract functions, that have to be overwritten by
   a derived radio station:

      copy         create an exact copy of a station (in case we only have a RadioStation*
      longName     return a verbous station description
      isValid      is this station setup correctly ?
      compare      is this station equivalent to another station, e.g. approximately same frequency
      getclassname classname string for station registry

   Other methods "should" be overwritten, but still call inherited methods for completeness!

      get/setProperty
      getPropertyNames

*/

/////////////////////////////////////////////////////////////////////////////

extern struct RegisterStationClass {} registerStationClass;

/////////////////////////////////////////////////////////////////////////////

class RadioStationConfig;

enum StationStereoMode { STATION_STEREO_ON, STATION_STEREO_OFF, STATION_STEREO_DONTCARE };

class KRADIO5_EXPORT RadioStation
{
protected:
    RadioStation (RegisterStationClass, const QString &classname);
    
    RadioStation & operator = (const RadioStation &) = default;
    
public:
    RadioStation ();
    RadioStation (const QString &name, const QString &shortName);
    RadioStation (const RadioStation &);
    virtual ~RadioStation();

    const QString     &stationID()      const { return m_stationID; }

    virtual QString    longName()       const = 0;
    virtual QString    description()    const = 0;

    const QString     &name()           const { return m_name;          }
    const QString     &shortName()      const { return m_shortName;     }
    const QString     &iconName()       const { return m_iconName;      }
    float              initialVolume()  const { return m_initialVolume; }
    StationStereoMode  stereoMode()     const { return m_stereoMode;    }

    void  setName         (const QString &name)       { m_name          = name;          }
    void  setShortName    (const QString &shortName)  { m_shortName     = shortName;     }
    void  setIconName     (const QString &iconName)   { m_iconName      = iconName;      }
    void  setInitialVolume(float initialVolume)       { m_initialVolume = initialVolume; }
    void  setStereoMode   (StationStereoMode mode)    { m_stereoMode    = mode;          }

    void  copyDescriptionFrom(const RadioStation &rs);

    // for XML-Parsing/Export
    virtual bool        setProperty(const QString &property_name, const QString &val);
    virtual QString     getProperty(const QString &property_name) const;
    virtual QStringList getPropertyNames() const;
    virtual QString     getClassName()        const = 0;
    virtual QString     getClassDescription() const = 0;
    virtual bool        isClassUserVisible()  const = 0;

    // get empty derived stations by classname from registry
    static RadioStation const  *getStationClass(const QString &classname);
    static QList<RadioStation*> getStationClasses();
           RadioStation const  *getStationClass() const { return getStationClass(getClassName()); }

    // = 0 : "this" is same as "s", e.g. approximately same frequency, same url, ...
    // > 0 : "this" is numerically (frequencies) or alphanumerically (urls) or ... greater than "s"
    // < 0 : "this" is numerically (frequencies) or alphanumerically (urls) or ... smaller than "s"
    virtual int compare(const RadioStation &s) const = 0;

    // is this station setup correctly ?
    virtual bool isValid() const = 0;

    /** returns an exact copy of this station */
    virtual RadioStation *copy() const = 0;
    /** returns an exact copy of this station, BUT with a new station ID */
    virtual RadioStation *copyNewID() const = 0;

    void generateNewStationID();

    virtual RadioStationConfig *createEditor() const = 0;

    virtual bool operator == (const RadioStation &x) const;
    virtual bool operator != (const RadioStation &x) const { return !operator==(x); }

protected :
    QString  m_stationID;

    QString           m_name;
    QString           m_shortName;
    float             m_initialVolume;        // <0: => Don't use
    QString           m_iconName;

    StationStereoMode m_stereoMode;

private:
    static QMap<QString, RadioStation*>  &getStationClassRegistry();
    static QMap<QString, RadioStation*>  *m_stationClassRegistry;

};







class KRADIO5_EXPORT UndefinedRadioStation : public RadioStation
{
public:
    UndefinedRadioStation (RegisterStationClass) : RadioStation (registerStationClass, getClassName()) {}

    virtual QString       longName()    const override { return i18nc("Unknown station long name", "unknown"); }
    virtual QString       description() const override { return i18nc("Unknown station description", "unknown"); }
    virtual bool          isValid()     const override { return false; }
    virtual RadioStation *copy()        const override { return new UndefinedRadioStation(*this); }
    virtual RadioStation *copyNewID()   const override { RadioStation *x = new UndefinedRadioStation(*this); x->generateNewStationID(); return x;  }
    virtual int           compare(const RadioStation &s) const override;

    virtual QString       getClassName()        const override { return QString::fromLatin1("UndefinedRadioStation"); }
    virtual QString       getClassDescription() const override { return QString::fromLatin1("UndefinedRadioStation"); }
    virtual bool          isClassUserVisible()  const override { return false; }
    virtual RadioStationConfig *createEditor() const override;
};


extern KRADIO5_EXPORT const UndefinedRadioStation undefinedRadioStation;

#endif
