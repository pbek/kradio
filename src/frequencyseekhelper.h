/***************************************************************************
                          frequencyseekhelper.h  -  description
                             -------------------
    begin                : Fre Mai 9 2003
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

#ifndef KRADIO_FREQUENCY_SEEKHELPER_H
#define KRADIO_FREQUENCY_SEEKHELPER_H

#include <QObject>
//#include "radiodevice_interfaces.h"
#include "frequencyradio_interfaces.h"
#include "seekhelper.h"

class QTimer;

class KRADIO5_EXPORT FrequencySeekHelper : public QObject,
                                           public SeekHelper,
                                           public IFrequencyRadioClient
{
Q_OBJECT
public:

    FrequencySeekHelper(ISeekRadio &parent);
    virtual ~FrequencySeekHelper();

// IFrequencyRadioClient
RECEIVERS:
    bool noticeFrequencyChanged(float /*f*/, const FrequencyRadioStation */*s*/) override { return false; }
    bool noticeMinMaxFrequencyChanged(float /*min*/, float /*max*/)              override { return false; }
    bool noticeDeviceMinMaxFrequencyChanged(float /*min*/, float /*max*/)        override { return false; }
    bool noticeScanStepChanged(float /*s*/)                                      override { return false; }

public:

    virtual bool     connectI   (Interface *i) override;
    virtual bool     disconnectI(Interface *i) override;

    virtual void     start(const SoundStreamID &, direction_t dir) override;

public slots:

    virtual void step() override { SeekHelper::step(); }

protected:
    virtual void abort       ()       override;
    virtual bool isGood      () const override;
    virtual bool isBetter    () const override;
    virtual bool isWorse     () const override;
    virtual bool bestFound   () const override;
    virtual void getData     ()       override;
    virtual void rememberBest()       override;
    virtual bool nextSeekStep()       override;
    virtual void applyBest   ()       override;

protected:
    QTimer  *m_timer;

    float    m_currentSignal, m_oldSignal;
    bool     m_goodSignal;
    float    m_currentFrequency, m_oldFrequency;
    float    m_bestFrequency;
};

#endif
