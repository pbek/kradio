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
#include "pluginbase_config_page.h"

class RadioStationConfig;


class RadioConfiguration : public PluginConfigPageBase,
                           public Ui_RadioConfigurationUI,
                           public IRadioClient,
                           public IRadioDevicePoolClient
{
Q_OBJECT
public :
    RadioConfiguration (QWidget *parent, const IErrorLogClient &m_logger);
    ~RadioConfiguration ();

    bool connectI    (Interface *i) override;
    bool disconnectI (Interface *i) override;

    // IRadioDevicePoolClient

RECEIVERS:
    bool noticeActiveDeviceChanged(IRadioDevice *) override  { return false; }
    bool noticeDevicesChanged(const QList<IRadioDevice*> &) override;
    bool noticeDeviceDescriptionChanged(const QString &) override;

    // IRadioClient

RECEIVERS:
    bool noticePowerChanged(bool /*on*/)                            override { return false; }  // don't care
    bool noticeStationChanged   (const RadioStation &, int /*idx*/) override { return false; }  // don't care
    bool noticeStationsChanged  (const StationList  & sl)           override;
    bool noticePresetFileChanged(const QUrl         & f)            override;

    bool noticeRDSStateChanged      (bool  /*enabled*/)             override { return false; }  // don't care
    bool noticeRDSRadioTextChanged  (const QString &/*s*/)          override { return false; }  // don't care
    bool noticeRDSStationNameChanged(const QString &/*s*/)          override { return false; }  // don't care

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/)  override { return false; }
    bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID /*id*/)  override { return false; }

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

    virtual void slotOK    () override;
    virtual void slotCancel() override;
    void         slotSetDirty();

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

