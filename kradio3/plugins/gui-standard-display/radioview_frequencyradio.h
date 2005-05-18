/***************************************************************************
                          kradiodisplay.h  -  description
                             -------------------
    begin                : Mit Jan 29 2003
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

#ifndef KRADIO_RADIOVIEW_FREQUENCYRADIO_H
#define KRADIO_RADIOVIEW_FREQUENCYRADIO_H

#include "../../src/interfaces/radiodevice_interfaces.h"
#include "../../src/interfaces/soundstreamclient_interfaces.h"
#include "radioview_element.h"
#include "displaycfg_interfaces.h"

/**
  *@author Martin Witte
  */

class RadioViewFrequencyRadio : public RadioViewElement,  // is a QObject, must be first
                                public IRadioDeviceClient,
                                public IFrequencyRadioClient,
                                public ISoundStreamClient,
                                public IDisplayCfg
{
Q_OBJECT
public:
    RadioViewFrequencyRadio(QWidget *parent, const QString &name);
    ~RadioViewFrequencyRadio();

    float getUsability (Interface *) const;

    virtual void   saveState (KConfig *) const;
    virtual void   restoreState (KConfig *);

    ConfigPageInfo createConfigurationPage();

// Interface

    bool connectI   (Interface *);
    bool disconnectI(Interface *);

// IDisplayCfg

RECEIVERS:
    bool  setDisplayColors(const QColor &activeColor, const QColor &inactiveColor, const QColor &bkgnd);
    bool  setDisplayFont (const QFont &f);

ANSWERS:
    QColor   getDisplayActiveColor()   const { return m_colorActiveText; }
    QColor   getDisplayInactiveColor() const { return m_colorInactiveText; }
    QColor   getDisplayBkgndColor()    const { return m_colorButton; }
    QFont    getDisplayFont()          const { return m_font; }

// IRadioDeviceClient
RECEIVERS:
    bool noticePowerChanged   (bool on, const IRadioDevice *sender = NULL);
    bool noticeStationChanged (const RadioStation &, const IRadioDevice *sender = NULL);
    bool noticeDescriptionChanged (const QString &, const IRadioDevice *sender = NULL);
    bool noticeCurrentSoundStreamIDChanged(SoundStreamID /*id*/, const IRadioDevice */*sender*/) { return false; }

// ISoundStreamClient
RECEIVERS:
    bool noticeSignalQualityChanged(SoundStreamID id, float q);
    bool noticeStereoChanged(SoundStreamID id, bool  s);

// IFrequencyRadioClient
RECEIVERS:
    bool noticeFrequencyChanged(float f, const RadioStation *s);
    bool noticeMinMaxFrequencyChanged(float min, float max);
    bool noticeDeviceMinMaxFrequencyChanged(float min, float max);
    bool noticeScanStepChanged(float s);

// own stuff ;)

public:

    void reparent (QWidget *parent, WFlags f, const QPoint &p, bool showIt = FALSE);

protected:

    void drawContents(QPainter *p);

protected:

    QColor  m_colorActiveText, m_colorInactiveText, m_colorButton;
    QFont   m_font;

    bool  m_power;
    bool  m_valid;
    float m_frequency;
    float m_quality;
    bool  m_stereo;
};

#endif
