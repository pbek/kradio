/***************************************************************************
                          radio.h  -  description
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

#ifndef RADIO_H
#define RADIO_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qobject.h>
#include "stationlist.h"

// forward declarations
class RadioDevice;
class FrequencyRadio;
class InternetRadio;
class RadioStation;

// Qt forwards
class QString;

// KDE forwards
class KURL;

/**
 * The main Radio class, which is used as the interface of the radio functionality
 * to the GUI parts of the application
 * @author Klas Kalass
 */

class Radio : public QObject {
    Q_OBJECT
public:
    Radio();
    ~Radio();

    /** currently active radio station */
    RadioStation *currentStation();
protected slots:
    /**
     * Set the currently active radio station, this is really only for internal use.
     * If you wish to activate a station, call station->activate()!
     */
    void slotCurrentStationChanged(RadioStation *station);

public:
    // create stations, be aware that you need to take care of memory Management
    // yourself because those stations are not managed by this class!
    /**
     * Create a station for a frequency based Radio station.
     */
    RadioStation *createFrequencyRadioStation(QString const &name, float frequency);
    RadioStation *createInternetRadioStation(QString const &name, KURL const &url);

    StationList &stations(){return m_stations;};

    bool muted();
    bool power();

public slots:
    void slotPowerOn();
    void slotPowerOff();
    void slotPowerToggle();
    void slotMute();
    void slotUnmute();

signals:
    /** inform the outer world that a new radio station was activated,
     this actually just passes the signal from the station on.*/
    void signalStationChanged(RadioStation *);
    void signalPowerChanged(bool on);

protected:
    /**
     * register the radio station with this radio, connect
     * signals
     */
    void registerStation(RadioStation *station);
    /**
     * register the radio device with this radio, connect
     * signals
     */
    void registerDevice(RadioDevice *radiodevice);

    StationList m_stations;
    RadioStation *m_currentStation;
    FrequencyRadio *m_frequencyRadio;
    InternetRadio *m_artsStreamRadio;
};

#endif
