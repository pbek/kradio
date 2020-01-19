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

#include <QTimer>
#include <QtCore/QUrl>
#include <QtGui/QPen>
#include <QtGui/QBrush>

#include "radiodevice_interfaces.h"
#include "frequencyradio_interfaces.h"
#include "internetradio_interfaces.h"
#include "soundstreamclient_interfaces.h"
#include "radioview_element.h"
#include "displaycfg_interfaces.h"

/**
  *@author Martin Witte
  */

// // START: DEBUG
// #include <QPainter>
// class TmpWidget : public QWidget
// {
// Q_OBJECT
// public:
//     TmpWidget() {};
//     ~TmpWidget() {};
//     void setPixmap(const QPixmap &p) { m_pixmap = p; update(); }
//     void paintEvent(QPaintEvent *e) {
//         QPainter paint(this);
//         paint.setCompositionMode(QPainter::CompositionMode_Source);
//         paint.drawPixmap(0, 0, m_pixmap);
//     }
//     QPixmap m_pixmap;
// };
// // END: DEBUG




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

    float getUsability (Interface *) const override;

    virtual void   saveState   (      KConfigGroup &) const override;
    virtual void   restoreState(const KConfigGroup &)       override;

    ConfigPageInfo createConfigurationPage() override;

// Interface

    bool connectI   (Interface *) override;
    bool disconnectI(Interface *) override;

// IDisplayCfg

RECEIVERS:
    bool  setDisplayColors(const QColor &activeColor, const QColor &inactiveColor, const QColor &bkgnd) override;
    bool  setDisplayFont  (const QFont &f) override;

ANSWERS:
    const QColor   &getDisplayActiveColor()   const override { return m_colorActiveText; }
    const QColor   &getDisplayInactiveColor() const override { return m_colorInactiveText; }
    const QColor   &getDisplayBkgndColor()    const override { return m_colorButton; }
    const QFont    &getDisplayFont()          const override { return m_font; }

// IRadioDeviceClient
RECEIVERS:
    bool noticePowerChanged         (bool  on,             const IRadioDevice *sender = NULL) override;
    bool noticeStationChanged       (const RadioStation &, const IRadioDevice *sender = NULL) override;
    bool noticeDescriptionChanged   (const QString &,      const IRadioDevice *sender = NULL) override;

    bool noticeRDSStateChanged      (bool  enabled,        const IRadioDevice *sender = NULL) override;
    bool noticeRDSRadioTextChanged  (const QString &s,     const IRadioDevice *sender = NULL) override;
    bool noticeRDSStationNameChanged(const QString &s,     const IRadioDevice *sender = NULL) override;

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/, const IRadioDevice */*sender*/) override { return false; }
    bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID /*id*/, const IRadioDevice */*sender*/) override { return false; }

// ISoundStreamClient
RECEIVERS:
    void noticeConnectedI (ISoundStreamServer *s, bool pointer_valid) override;

    bool noticeSignalQualityChanged(SoundStreamID id,     float q    ) override;
    bool noticeSignalQualityChanged(SoundStreamID /*id*/, bool  /*q*/) override { return false; }
    bool noticeStereoChanged(SoundStreamID id, bool  s)                override;

// IFrequencyRadioClient
RECEIVERS:
    bool noticeFrequencyChanged            (float f,   const FrequencyRadioStation *s) override;
    bool noticeMinMaxFrequencyChanged      (float min, float max) override;
    bool noticeDeviceMinMaxFrequencyChanged(float min, float max) override;
    bool noticeScanStepChanged             (float s)              override;


// IInternetRadioClient
RECEIVERS:
    bool noticeURLChanged(const QUrl &url, const InternetRadioStation *irs) override;

// own stuff ;)

protected:
    INLINE_IMPL_DEF_noticeConnectedI(IRadioDeviceClient);
    INLINE_IMPL_DEF_noticeConnectedI(IFrequencyRadioClient);
    INLINE_IMPL_DEF_noticeConnectedI(IInternetRadioClient);
    INLINE_IMPL_DEF_noticeConnectedI(IDisplayCfg);

    void paintEvent (QPaintEvent  *e) override;
    void resizeEvent(QResizeEvent *e) override;
    bool event      (QEvent       *e) override;

//     void updateRadioTextRing();

protected slots:

    void slotRadioTextTimer();

protected:

    QColor   m_colorActiveText, m_colorInactiveText, m_colorButton;
    QFont    m_font;

    bool     m_power;
    bool     m_valid;

    float    m_frequency;
    QUrl     m_url;
    QString  m_station_name;

    float    m_quality;
    bool     m_stereo;
    bool     m_RDS_enabled;
    QString  m_RDSRadioText;
    QString  m_RDSStationName;

    QTimer   m_RadioTextTimer;
//     QString  m_RadioTextRing;
//     qreal    m_RadioTextX0;
    qreal    m_RadioTextDX;
//     bool     m_RadioTextRepaint;

    QPen     m_activePen;
    QPen     m_inactivePen;
    QBrush   m_activeBrush;
    QBrush   m_inactiveBrush;

    // sliding radio text
    void     resetRadioTextVisualBuffer();
    void     updateRadioTextVisualBuffer(QRectF newVisualRect);
    void     advanceRadioTextVisualBuffer();
    QRectF   drawTextInRadioTextVisualBuffer(QPainter &paint);
    void     paintRadioTextVisualBuffer(QPainter &paint);
//     TmpWidget *m_bufferVisualizer;

    QRectF   m_radioTextRect;
    int      m_radioTextVisualBufferOverSizeFactor;
    QSize    m_radioTextVisualBufferSize;
    qreal    m_radioTextVisualBufferCurrentReadX;
    qreal    m_radioTextVisualBufferCurrentWriteX;
    QPixmap  m_radioTextVisualBuffer;

    QString  m_text_am;
    QString  m_text_fm;
    QString  m_text_rds;
    QString  m_text_net;
    QString  m_text_frequency;
};

#endif
