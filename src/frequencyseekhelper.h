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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QtCore/QObject>
#include <QtCore/QTimer>
//#include "radiodevice_interfaces.h"
#include "frequencyradio_interfaces.h"
#include "seekhelper.h"
#include <kdemacros.h>

class KDE_EXPORT FrequencySeekHelper : public QObject,
                                       public SeekHelper,
                                       public IFrequencyRadioClient
{
Q_OBJECT
public:

    FrequencySeekHelper(ISeekRadio &parent);
    virtual ~FrequencySeekHelper();

// IFrequencyRadioClient
RECEIVERS:
    bool noticeFrequencyChanged(float /*f*/, const FrequencyRadioStation */*s*/)  { return false; }
    bool noticeMinMaxFrequencyChanged(float /*min*/, float /*max*/)      { return false; }
    bool noticeDeviceMinMaxFrequencyChanged(float /*min*/, float /*max*/){ return false; }
    bool noticeScanStepChanged(float /*s*/)                              { return false; }

public:

    virtual bool     connectI   (Interface *i);
    virtual bool     disconnectI(Interface *i);

    virtual void     start(const SoundStreamID &, direction_t dir);

public slots:

    virtual void step() { SeekHelper::step(); }

protected:
    virtual void abort();
    virtual bool isGood() const;
    virtual bool isBetter() const;
    virtual bool isWorse() const;
    virtual bool bestFound() const;
    virtual void getData();
    virtual void rememberBest();
    virtual bool nextSeekStep();
    virtual void applyBest();

protected:
    QTimer  *m_timer;

    float    m_currentSignal, m_oldSignal;
    bool     m_goodSignal;
    float    m_currentFrequency, m_oldFrequency;
    float    m_bestFrequency;
};

#endif
