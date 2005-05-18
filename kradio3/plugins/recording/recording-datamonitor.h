/***************************************************************************
                          recording-monitor-widget.h  -  description
                             -------------------
    begin                : So Sep 7 2003
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

#ifndef KRADIO_RECORDING_DATA_MONITOR
#define KRADIO_RECORDING_DATA_MONITOR

#include <qframe.h>
#include <qcolor.h>

//#include <kradio/interfaces/recording-interfaces.h>
#include <kradio/interfaces/soundstreamclient_interfaces.h>

class RecordingDataMonitor : public QFrame//,
                             //public ISoundStreamClient
                             //public IRecordingClient
{
Q_OBJECT
public:
    RecordingDataMonitor(QWidget *parent, const char *name);
    ~RecordingDataMonitor();

//    bool connectI(Interface *i);
//    bool disconnectI(Interface *i);

// IRecordingClient

/*RECEIVERS:
    bool noticeRecordingStarted();
    bool noticeMonitoringStarted();
    bool noticeRecordingStopped();
    bool noticeMonitoringStopped();
    bool noticeRecordingConfigChanged(const RecordingConfig &);
    bool noticeRecordingContextChanged(const RecordingContext &);
*/
    bool noticeSoundStreamData(SoundStreamID id,
                               const SoundFormat &sf, const char *data, unsigned size,
                               const SoundMetaData &md);

// QT/KDE ...

protected:

    void drawContents(QPainter *p);
    void internalDrawContents(QPainter &painter, bool repaintAll);
// own stuff ...

protected:

    void setChannels(int n);
    bool setColors(const QColor &activeColor, const QColor &bkgnd);

// data
protected:

    int     *m_channelsMax;  // maximum absolute value recorded on each channel
    double  *m_channelsAvg;  // average value recorded on each channel
    int      m_maxValue;     // maximum absolute value possible for samples
    int      m_channels;

    QColor   m_colorActiveText, m_colorButton;

    int     *m_pActiveBlocks;
};

#endif
