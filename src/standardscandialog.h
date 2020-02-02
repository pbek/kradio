/***************************************************************************
                          standardscandialog.h  -  description
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

#ifndef KRADIO_STANDARDSCANDIALOG_H
#define KRADIO_STANDARDSCANDIALOG_H

#include "seekradio_interfaces.h"
#include "radio_interfaces.h"
#include "stationlist.h"

#include <QTimer>
#include <QDateTime>
#include <QDialog>

class QPushButton;
class Ui_StandardScanDialogUI;

class KRADIO5_EXPORT StandardScanDialog : public QDialog,
                                          public ISeekRadioClient,
                                          public IRadioClient,
                                          public IErrorLogClient
{
Q_OBJECT
public:
    StandardScanDialog(QWidget *parent);
    ~StandardScanDialog();

    bool connectI    (Interface *i) override;
    bool disconnectI (Interface *i) override;

    void start();
    void stop();

    const StationList &getStations() const { return m_stations; }

// ISeekRadioClient

RECEIVERS:
    bool noticeSeekStarted (bool up) override;
    bool noticeSeekStopped ()        override;
    bool noticeSeekFinished (const RadioStation &s, bool goodQuality) override;
    bool noticeProgress (float f)    override;

// IRadioClient

RECEIVERS:
    bool noticePowerChanged(bool on) override;
    bool noticeStationChanged (const RadioStation &, int /*idx*/) override { return false; }
    bool noticeStationsChanged(const StationList &/*sl*/)         override { return false; }
    bool noticePresetFileChanged(const QUrl &/*f*/)               override { return false; }

    bool noticeRDSStateChanged      (bool  /*enabled*/)           override{ return false; }  // don't care
    bool noticeRDSRadioTextChanged  (const QString &/*s*/)        override{ return false; }  // don't care
    bool noticeRDSStationNameChanged(const QString &/*s*/)        override;

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/) override { return false; }
    bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID /*id*/) override { return false; }

protected slots:

    void slotStartStop();
    void slotOk();
    void slotDiscard();
    void slotWait4RDSTimeout();

protected:

    void addCurrentStation();
    void continueScan();

    int           m_count;
    bool          m_running;
    bool          m_oldPowerOn;
    RadioStation *m_oldStation;
    QDateTime     m_startTime;

    StationList   m_stations;

    bool          m_ignorePower;

    Ui_StandardScanDialogUI *m_ui;
    QPushButton             *m_buttonStartStop;

    QTimer        m_Wait4RDSTimeout;
    bool          m_waiting4RDS;
};


#endif
