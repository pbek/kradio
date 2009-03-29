/***************************************************************************
                          radioview_frequencyseeker.h  -  description
                             -------------------
    begin                : Fre Jun 20 2003
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

#ifndef KRADIO_RADIOVIEW_FREQUENCYSEEKER_H
#define KRADIO_RADIOVIEW_FREQUENCYSEEKER_H

#include <QtGui/QAction>

#include "seekradio_interfaces.h"
#include "frequencyradio_interfaces.h"
#include "radioview_element.h"

class QToolButton;
class QSlider;

class RadioViewFrequencySeeker : public RadioViewElement,  // is a QObject, must be first
                                 public ISeekRadioClient,
                                 public IFrequencyRadioClient
{
Q_OBJECT
public:
    RadioViewFrequencySeeker(QWidget *parent, const QString &name);
    ~RadioViewFrequencySeeker();

    float getUsability(Interface *) const;

// Interface

    bool connectI   (Interface *);
    bool disconnectI(Interface *);

// ISeekRadioClient
RECEIVERS:
    bool noticeSeekStarted (bool up);
    bool noticeSeekStopped ();
    bool noticeSeekFinished (const RadioStation &s, bool goodQuality);
    bool noticeProgress (float ) { return false; }

// IFrequencyRadioClient
RECEIVERS:
    bool noticeFrequencyChanged(float f, const FrequencyRadioStation *s);
    bool noticeMinMaxFrequencyChanged(float min, float max);
    bool noticeDeviceMinMaxFrequencyChanged(float min, float max);
    bool noticeScanStepChanged(float s);

// own stuff ;)

protected slots:

    void slotSearchLeft(bool on);
    void slotSearchRight(bool on);
    void slotSliderChanged(int val);
    void slotStepUp();
    void slotStepDown();

protected:

    QToolButton *m_btnSearchLeft,
                *m_btnStepLeft,
                *m_btnStepRight,
                *m_btnSearchRight;
    QSlider     *m_sldFrequency;

    bool         m_ignoreChanges;

    QAction      m_accelLeft;
    QAction      m_accelRight;
};



#endif
