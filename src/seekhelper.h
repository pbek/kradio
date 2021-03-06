/***************************************************************************
                          seekhelper.h  -  description
                             -------------------
    begin                : Sam Mai 10 2003
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

#ifndef KRADIO_SEEKHELPER_H
#define KRADIO_SEEKHELPER_H

#include "radiodevice_interfaces.h"
#include "seekradio_interfaces.h"
#include "soundstreamclient_interfaces.h"


class KRADIO5_EXPORT SeekHelper : public IRadioDeviceClient,
                                  public ISoundStreamClient
{
public:
    typedef enum { off, searchWorse, searchBest } state_t;
    typedef enum { up, down } direction_t;

    SeekHelper(ISeekRadio &parent);
    virtual ~SeekHelper();

    virtual bool     connectI   (Interface *i) override;
    virtual bool     disconnectI(Interface *i) override;

    virtual void start(const SoundStreamID &id, direction_t dir);
    virtual void step();
    virtual void stop();

    bool isRunning()     const { return m_state != off; }
    bool isRunningUp()   const { return m_state != off && m_direction == up; }
    bool isRunningDown() const { return m_state != off && m_direction == down; }


// IRadioDeviceClient
RECEIVERS:
    bool noticePowerChanged         (bool  /*on*/,         const IRadioDevice */*sender*/) override { return false; }
    bool noticeStationChanged       (const RadioStation &, const IRadioDevice */*sender*/) override { return false; }
    bool noticeDescriptionChanged   (const QString &,      const IRadioDevice */*sender*/) override { return false; }

    bool noticeRDSStateChanged      (bool  /*enabled*/,    const IRadioDevice */*sender*/) override { return false; }
    bool noticeRDSRadioTextChanged  (const QString &/*s*/, const IRadioDevice */*sender*/) override { return false; }
    bool noticeRDSStationNameChanged(const QString &/*s*/, const IRadioDevice */*sender*/) override { return false; }

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/, const IRadioDevice */*sender*/) override { return false; }
    bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID /*id*/, const IRadioDevice */*sender*/) override { return false; }

protected:

    virtual void finish();

    virtual void abort() = 0;
    virtual bool isGood() const = 0;
    virtual bool isBetter() const = 0;
    virtual bool isWorse() const = 0;
    virtual bool bestFound() const = 0;
    virtual void getData() = 0;
    virtual void rememberBest() = 0;
    virtual bool nextSeekStep() = 0;
    virtual void applyBest() = 0;

protected:
    state_t        m_state;
    direction_t    m_direction;
    bool           m_oldMute;

    ISeekRadio    &m_parent;
    SoundStreamID  m_SoundStreamID;
};

#endif

