/***************************************************************************
                          kradiodisplay.h  -  description
                             -------------------
    begin                : Mit Jan 29 2003
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

#ifndef KRADIO_RADIOVIEW_FREQUENCYRADIO_H
#define KRADIO_RADIOVIEW_FREQUENCYRADIO_H

#include <QtCore/QTimer>

#include <kurl.h>

#include "radiodevice_interfaces.h"
#include "frequencyradio_interfaces.h"
#include "internetradio_interfaces.h"
#include "soundstreamclient_interfaces.h"
#include "radioview_element.h"
#include "displaycfg_interfaces.h"

/**
  *@author Martin Witte
  */

class RadioViewFrequencyRadio : public RadioViewElement,  // is a QObject, must be first
                                public IRadioDeviceClient,
                                public IFrequencyRadioClient,
                                public IInternetRadioClient,
                                public ISoundStreamClient,
                                public IDisplayCfg
{
Q_OBJECT
public:
    RadioViewFrequencyRadio(QWidget *parent, const QString &name);
    ~RadioViewFrequencyRadio();

    float getUsability (Interface *) const;

    virtual void   saveState   (      KConfigGroup &) const;
    virtual void   restoreState(const KConfigGroup &);

    ConfigPageInfo createConfigurationPage();

// Interface

    bool connectI   (Interface *);
    bool disconnectI(Interface *);

// IDisplayCfg

RECEIVERS:
    bool  setDisplayColors(const QColor &activeColor, const QColor &inactiveColor, const QColor &bkgnd);
    bool  setDisplayFont (const QFont &f);

ANSWERS:
    const QColor   &getDisplayActiveColor()   const { return m_colorActiveText; }
    const QColor   &getDisplayInactiveColor() const { return m_colorInactiveText; }
    const QColor   &getDisplayBkgndColor()    const { return m_colorButton; }
    const QFont    &getDisplayFont()          const { return m_font; }

// IRadioDeviceClient
RECEIVERS:
    bool noticePowerChanged         (bool  on,             const IRadioDevice *sender = NULL);
    bool noticeStationChanged       (const RadioStation &, const IRadioDevice *sender = NULL);
    bool noticeDescriptionChanged   (const QString &,      const IRadioDevice *sender = NULL);

    bool noticeRDSStateChanged      (bool  enabled,        const IRadioDevice *sender = NULL);
    bool noticeRDSRadioTextChanged  (const QString &s,     const IRadioDevice *sender = NULL);
    bool noticeRDSStationNameChanged(const QString &s,     const IRadioDevice *sender = NULL);

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/, const IRadioDevice */*sender*/) { return false; }
    bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID /*id*/, const IRadioDevice */*sender*/) { return false; }

// ISoundStreamClient
RECEIVERS:
    void noticeConnectedI (ISoundStreamServer *s, bool pointer_valid);

    bool noticeSignalQualityChanged(SoundStreamID id, float q);
    bool noticeSignalQualityChanged(SoundStreamID /*id*/, bool  /*q*/) { return false; }
    bool noticeStereoChanged(SoundStreamID id, bool  s);

// IFrequencyRadioClient
RECEIVERS:
    bool noticeFrequencyChanged(float f, const FrequencyRadioStation *s);
    bool noticeMinMaxFrequencyChanged(float min, float max);
    bool noticeDeviceMinMaxFrequencyChanged(float min, float max);
    bool noticeScanStepChanged(float s);


// IInternetRadioClient
RECEIVERS:
    bool noticeURLChanged(const KUrl &url, const InternetRadioStation *irs);

// own stuff ;)

public:

    void setParent(QWidget * parent);
    void setParent(QWidget * parent, Qt::WindowFlags f);

protected:
    INLINE_IMPL_DEF_noticeConnectedI(IRadioDeviceClient);
    INLINE_IMPL_DEF_noticeConnectedI(IFrequencyRadioClient);
    INLINE_IMPL_DEF_noticeConnectedI(IInternetRadioClient);
    INLINE_IMPL_DEF_noticeConnectedI(IDisplayCfg);

    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);

    void updateRadioTextRing();

protected slots:

    void slotRadioTextTimer();

protected:

    QColor   m_colorActiveText, m_colorInactiveText, m_colorButton;
    QFont    m_font;

    bool     m_power;
    bool     m_valid;

    float    m_frequency;
    KUrl     m_url;
    QString  m_station_name;

    float    m_quality;
    bool     m_stereo;
    bool     m_RDS_enabled;
    QString  m_RDSRadioText;
    QString  m_RDSStationName;

    QTimer   m_RadioTextTimer;
    QString  m_RadioTextRing;
    qreal    m_RadioTextX0;
    qreal    m_RadioTextDX;
    bool     m_RadioTextRepaint;
};

#endif
