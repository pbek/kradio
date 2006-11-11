/***************************************************************************
                          standardscandialog.h  -  description
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

#ifndef KRADIO_STANDARDSCANDIALOG_H
#define KRADIO_STANDARDSCANDIALOG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../standardscandialog-ui.h"
#include "radiodevice_interfaces.h"
#include "radio_interfaces.h"
#include "stationlist.h"

#include <qtimer.h>
#include <qdatetime.h>

class StandardScanDialog : public StandardScanDialogUI,
                           public ISeekRadioClient,
//                           public IRadioSoundClient,
                           public IRadioClient
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

// // ISoundRadioClient
//
// RECEIVERS:
//     bool noticeVolumeChanged(float /*v*/)           { return false; }
//     bool noticeTrebleChanged(float /*v*/)           { return false; }
//     bool noticeBassChanged(float /*v*/)             { return false; }
//     bool noticeBalanceChanged(float /*v*/)          { return false; }
//     bool noticeSignalQualityChanged(float /*q*/)    { return false; }
//     bool noticeSignalQualityChanged(bool /*good*/)  { return false; }
//     bool noticeSignalMinQualityChanged(float /*q*/) { return false; }
//     bool noticeStereoChanged(bool  /*s*/)           { return false; }
//     bool noticeMuted(bool /*m*/)                    { return false; }

// IRadioClient

RECEIVERS:
    bool noticePowerChanged(bool on);
    bool noticeStationChanged (const RadioStation &, int /*idx*/){ return false; }
    bool noticeStationsChanged(const StationList &/*sl*/)        { return false; }
    bool noticePresetFileChanged(const QString &/*f*/)           { return false; }

    bool noticeCurrentSoundStreamIDChanged(SoundStreamID /*id*/) { return false; }

protected slots:

    void slotCancelDone();

protected:

    int           m_count;
    bool          m_running;
    bool          m_oldPowerOn;
    RadioStation *m_oldStation;
    QDateTime     m_startTime;

    StationList   m_stations;

    bool          m_ignorePower;
};


#endif
