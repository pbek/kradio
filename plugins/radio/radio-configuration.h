/***************************************************************************
                          radio-configuration.h  -  description
                             -------------------
    begin                : Son Aug 3 2003
    copyright            : (C) 2003 by Martin Witte
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

#ifndef KRADIO_RADIO_CONFIGURATION_H
#define KRADIO_RADIO_CONFIGURATION_H

#include "radio_interfaces.h"
#include "stationlist.h"
#include "radiodevicepool_interfaces.h"
#include "ui_radio-configuration-ui.h"

class RadioStationConfig;


class RadioConfiguration : public QWidget,
                           public Ui_RadioConfigurationUI,
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
    bool noticeDevicesChanged(const QList<IRadioDevice*> &);
    bool noticeDeviceDescriptionChanged(const QString &);

    // IRadioClient

RECEIVERS:
    bool noticePowerChanged(bool /*on*/)                          { return false; }  // don't care
    bool noticeStationChanged (const RadioStation &, int /*idx*/) { return false; }  // don't care
    bool noticeStationsChanged(const StationList &sl);
    bool noticePresetFileChanged(const QString &f);

    bool noticeRDSStateChanged      (bool  /*enabled*/)           { return false; }  // don't care
    bool noticeRDSRadioTextChanged  (const QString &/*s*/)        { return false; }  // don't care
    bool noticeRDSStationNameChanged(const QString &/*s*/)        { return false; }  // don't care

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/)  { return false; }
    bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID /*id*/)  { return false; }

protected:

    void createNewStation(const RadioStation *rs_template);

protected slots:

    void slotStationSelectionChanged(int idx);
    void slotNewStation();
    void slotNewStation(QAction *a);
    void slotDeleteStation();
    void slotStationEditorChanged(RadioStationConfig *c);
    void slotStationNameChanged( const QString & s);
    void slotStationShortNameChanged( const QString & sn);
    void slotPixmapChanged( const QString &s );
    void slotSelectPixmap();
    void slotVolumePresetChanged(int v);
    void slotStereoModeChanged(int m);
    void slotStationUp();
    void slotStationDown();
    void slotActivateStation( int );
    void slotLoadPresets();
    void slotAddPresets();
    void slotStorePresets();
    void slotLastChangeNow();
    void slotSendPresetsByMail( const QString &url );

    void slotSearchStations(QAction *a);

    void slotOK();
    void slotCancel();
    void slotSetDirty();

    void loadPresets(bool add);

protected:

    StationList                          m_stations;
    bool                                 m_ignoreChanges;

    QMenu                               *m_loadPopup;
    QMenu                               *m_devicePopup;
    QList<IRadioDevice*>                 m_devices;

    QMap<QString, RadioStationConfig*>   m_stationEditors;

    const IErrorLogClient               &m_logger;
    bool                                 m_dirty;

    QMenu                               *m_stationTypeMenu;
};

#endif

