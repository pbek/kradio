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

#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtGui/QDialog>

class Ui_StandardScanDialogUI;

class KDE_EXPORT StandardScanDialog : public QDialog,
                                      public ISeekRadioClient,
                                      public IRadioClient,
                                      public IErrorLogClient
{
Q_OBJECT
public:
    StandardScanDialog(QWidget *parent);
    ~StandardScanDialog();

    bool connectI (Interface *i);
    bool disconnectI (Interface *i);

    void start();
    void stop();

    const StationList &getStations() const { return m_stations; }

// ISeekRadioClient

RECEIVERS:
    bool noticeSeekStarted (bool up);
    bool noticeSeekStopped ();
    bool noticeSeekFinished (const RadioStation &s, bool goodQuality);
    bool noticeProgress (float f);

// IRadioClient

RECEIVERS:
    bool noticePowerChanged(bool on);
    bool noticeStationChanged (const RadioStation &, int /*idx*/){ return false; }
    bool noticeStationsChanged(const StationList &/*sl*/)        { return false; }
    bool noticePresetFileChanged(const QString &/*f*/)           { return false; }

    bool noticeRDSStateChanged      (bool  /*enabled*/)          { return false; }  // don't care
    bool noticeRDSRadioTextChanged  (const QString &/*s*/)       { return false; }  // don't care
    bool noticeRDSStationNameChanged(const QString &/*s*/);

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/) { return false; }
    bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID /*id*/) { return false; }

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

    QTimer        m_Wait4RDSTimeout;
    bool          m_waiting4RDS;
};


#endif
