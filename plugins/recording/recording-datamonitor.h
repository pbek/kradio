/***************************************************************************
                          recording-monitor-widget.h  -  description
                             -------------------
    begin                : So Sep 7 2003
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

#ifndef KRADIO_RECORDING_DATA_MONITOR
#define KRADIO_RECORDING_DATA_MONITOR

#include <QFrame>
#include <QColor>

#include "soundstreamclient_interfaces.h"

class RecordingDataMonitor : public QFrame
{
Q_OBJECT

    enum PaintEventUpdateOptions { updateDefault, updateAllForced, updatePartially };

public:
    RecordingDataMonitor(QWidget *parent);
    ~RecordingDataMonitor();

    bool noticeSoundStreamData(SoundStreamID id,
                               const SoundFormat &sf, const char *data, size_t size, size_t &consumed_size,
                               const SoundMetaData &md);

    void reset();

// QT/KDE ...

protected:

    void paintEvent(QPaintEvent *e);
    void internalDrawContents(QPainter &painter, bool repaintAll);
    void setUpdateMode(PaintEventUpdateOptions opt);
    void update(PaintEventUpdateOptions opt);

// own stuff ...

protected:

    void setChannels(int n, bool force = false);
    bool setColors(const QColor &activeColor, const QColor &bkgnd);

// data
protected:

    int     *m_channelsMax;  // maximum absolute value recorded on each channel
    double  *m_channelsAvg;  // average value recorded on each channel
    int      m_maxValue;     // maximum absolute value possible for samples
    int      m_channels;

    QColor   m_colorActiveText, m_colorActiveButton;
    QColor   m_redActive,   m_orangeActive,   m_yellowActive;
    QColor   m_redInactive, m_orangeInactive, m_yellowInactive;

    int     *m_pActiveBlocks;


    PaintEventUpdateOptions  m_paintEventUpdateMode;
};

#endif
