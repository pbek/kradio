/***************************************************************************
                          radio-configuration.h  -  description
                             -------------------
    begin                : Son Aug 3 2003
    copyright            : (C) 2003 by Martin Witte
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

#ifndef KRADIO_RADIO_CONFIGURATION_H
#define KRADIO_RADIO_CONFIGURATION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qdict.h>

#include "../../src/interfaces/radio_interfaces.h"
#include "../../src/libkradio/stationlist.h"
#include "../../src/interfaces/radiodevicepool_interfaces.h"
#include "radio-configuration-ui.h"

class QPopupMenu;
class RadioStationConfig;

class RadioConfiguration : public RadioConfigurationUI,
                           public IRadioClient,
                           public IRadioDevicePoolClient
{
Q_OBJECT
public :
    RadioConfiguration (QWidget *parent, const IErrorLogClient &m_logger);
    ~RadioConfiguration ();

    bool connectI (Interface *i);
    bool disconnectI (Interface *i);

    // IRadioDevicePoolClient

RECEIVERS:
    bool noticeActiveDeviceChanged(IRadioDevice *)  { return false; }
    bool noticeDevicesChanged(const QPtrList<IRadioDevice> &);
    bool noticeDeviceDescriptionChanged(const QString &);

    // IRadioClient

RECEIVERS:
    bool noticePowerChanged(bool /*on*/)                          { return false; }  // don't care
    bool noticeStationChanged (const RadioStation &, int /*idx*/) { return false; }  // don't care
    bool noticeStationsChanged(const StationList &sl);
    bool noticePresetFileChanged(const QString &f);

    bool noticeCurrentSoundStreamIDChanged(SoundStreamID /*id*/)  { return false; }

protected slots:

    void slotStationSelectionChanged(int idx);
    void slotNewStation();
    void slotDeleteStation();
    void slotStationEditorChanged(RadioStationConfig *c);
    void slotStationNameChanged( const QString & s);
    void slotStationShortNameChanged( const QString & sn);
    void slotPixmapChanged( const QString &s );
    void slotSelectPixmap();
    void slotVolumePresetChanged(int v);
    void slotStationUp();
    void slotStationDown();
    void slotActivateStation( int );
    void slotLoadPresets();
    void slotStorePresets();
    void slotLastChangeNow();
    void slotSendPresetsByMail( const QString &url );

    void slotSearchStations(int i);
    void slotSearchStations0() { slotSearchStations(0); }

    void slotOK();
    void slotCancel();


protected:

    StationList                 m_stations;
    bool                        ignoreChanges;

    QPopupMenu                 *devicePopup;
    QPtrList<IRadioDevice>      devices;

    QDict<RadioStationConfig>   stationEditors;

    const IErrorLogClient      &m_logger;
};

#endif

