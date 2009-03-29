/***************************************************************************
                          radioview_volume.h  -  description
                             -------------------
    begin                : Don Jun 19 2003
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

#ifndef KRADIO_RADIOVIEW_VOLUME_H
#define KRADIO_RADIOVIEW_VOLUME_H

#include <QtGui/QAction>

#include "radiodevice_interfaces.h"
#include "soundstreamclient_interfaces.h"
#include "radioview_element.h"

/**
  *@author Martin Witte
  */

class QSlider;

class RadioViewVolume : public RadioViewElement,  // is a QObject, must be first
                        public IRadioDeviceClient,
                        public ISoundStreamClient,
                        public IErrorLogClient
{
Q_OBJECT
public:
    RadioViewVolume(QWidget *parent, const QString &name);
    ~RadioViewVolume();

    float getUsability(Interface *) const;

// Interface

    bool connectI   (Interface *);
    bool disconnectI(Interface *);

// IRadioDeviceClient
RECEIVERS:
    bool noticePowerChanged         (bool  /*on*/,         const IRadioDevice */*sender*/) { return false; }
    bool noticeStationChanged       (const RadioStation &, const IRadioDevice */*sender*/) { return false; }
    bool noticeDescriptionChanged   (const QString &,      const IRadioDevice */*sender*/) { return false; }

    bool noticeRDSStateChanged      (bool  /*enabled*/,    const IRadioDevice */*sender*/) { return false; }
    bool noticeRDSRadioTextChanged  (const QString &/*s*/, const IRadioDevice */*sender*/) { return false; }
    bool noticeRDSStationNameChanged(const QString &/*s*/, const IRadioDevice */*sender*/) { return false; }

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/, const IRadioDevice */*sender*/) { return false; }
    bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID /*id*/, const IRadioDevice */*sender*/) { return false; }

// ISoundStreamClient
RECEIVERS:
    void noticeConnectedI (ISoundStreamServer *s, bool pointer_valid);
    bool noticePlaybackVolumeChanged(SoundStreamID id, float v);

// own stuff
protected slots:

    void      slotVolumeChanged(int val);
    void      slotStepUp();
    void      slotStepDown();


protected:
    INLINE_IMPL_DEF_noticeConnectedI(IRadioDeviceClient);
    INLINE_IMPL_DEF_noticeConnectedI(IErrorLogClient);

protected:

    int       getSlider4Volume(float volume);
    float     getVolume4Slider(int sl);

    QSlider  *m_slider;
    bool      m_handlingSlot;

    QAction   m_volUpAction;
    QAction   m_volDownAction;

};

#endif
