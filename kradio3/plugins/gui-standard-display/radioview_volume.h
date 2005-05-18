/***************************************************************************
                          radioview_volume.h  -  description
                             -------------------
    begin                : Don Jun 19 2003
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

#ifndef KRADIO_RADIOVIEW_VOLUME_H
#define KRADIO_RADIOVIEW_VOLUME_H

#include <kradio/interfaces/radiodevice_interfaces.h>
#include <kradio/interfaces/soundstreamclient_interfaces.h>
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
    bool noticePowerChanged (bool /*on*/, const IRadioDevice */*sender*/)                        { return false; }
    bool noticeStationChanged (const RadioStation &, const IRadioDevice */*sender*/)             { return false; }
    bool noticeDescriptionChanged (const QString &, const IRadioDevice */*sender*/)              { return false; }
    bool noticeCurrentSoundStreamIDChanged(SoundStreamID /*id*/, const IRadioDevice */*sender*/) { return false; }

// ISoundStreamClient
RECEIVERS:
    bool noticePlaybackVolumeChanged(SoundStreamID id, float v);

// own stuff
protected slots:

    void slotVolumeChanged(int val);

protected:

    int   getSlider4Volume(float volume);
    float getVolume4Slider(int sl);

    QSlider  *m_slider;
    bool      m_handlingSlot;

};

#endif
