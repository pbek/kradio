/***************************************************************************
                          kradiodisplay.cpp  -  description
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

#include <QPainter>
#include <QImage>
#include <QPixmap>
#include <QBitmap>
#include <QResizeEvent>

#include <math.h>

#include <kconfiggroup.h>
#include <kglobal.h>

#ifdef KRADIO_ENABLE_FIXMES
    #warning "FIXME: port kimageeffect stuff"
#endif
//#include <kimageeffect.h>  // fading, blending, ...
//#include <kpixmapio.h>     // fast conversion between QPixmap/QImage
#include "radioview_frequencyradio.h"
#include "displaycfg.h"

#include "internetradio_interfaces.h"
#include "internetradiostation.h"
#include "frequencyradiostation.h"

#include "debug-profiler.h"

RadioViewFrequencyRadio::RadioViewFrequencyRadio(QWidget *parent, const QString &name )
    : RadioViewElement(parent, name, clsRadioDisplay),
      m_power(false),
      m_valid(false),
      m_frequency(0),
      m_quality(0.0),
      m_stereo(false),
//       m_RadioTextRing(QString()),
//       m_RadioTextX0(0),
      m_RadioTextDX(1),
//       m_RadioTextRepaint(false),
//       m_bufferVisualizer(NULL),
      m_radioTextRect(0, 0, 0, 0),
      m_radioTextVisualBufferOverSizeFactor(5),
      m_radioTextVisualBufferSize(0, 0),
      m_radioTextVisualBufferCurrentReadX(0),
      m_radioTextVisualBufferCurrentWriteX(0),
      m_text_am(i18n("AM")),
      m_text_fm(i18n("FM")),
      m_text_rds(i18n("RDS")),
      m_text_net(i18n("NET"))
{
    setFrameStyle(Box | Sunken);
    setLineWidth(1);
    setMidLineWidth(1);
    setAutoFillBackground(true);

    // set some sensless default colors
    // real values are read in restoreState
    setDisplayColors(QColor(20, 244, 20),
                     QColor(10, 117, 10).light(75),
                     QColor(10, 117, 10));
    setDisplayFont(QFont("Helvetica"));

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    m_RadioTextTimer.setInterval(100);
    m_RadioTextTimer.setSingleShot(false);
    QObject::connect(&m_RadioTextTimer, SIGNAL(timeout()), this, SLOT(slotRadioTextTimer()));
}


RadioViewFrequencyRadio::~RadioViewFrequencyRadio(){
}


float RadioViewFrequencyRadio::getUsability (Interface *i) const
{
    if (dynamic_cast<IFrequencyRadio*>(i))
        return 1.0;
    else if (dynamic_cast<IInternetRadio*>(i))
        return 0.5;
    else
        return 0.0;
}


void   RadioViewFrequencyRadio::saveState (KConfigGroup &config) const
{
    config.writeEntry("frequency-view-colorActiveText",   m_colorActiveText);
    config.writeEntry("frequency-view-colorInactiveText", m_colorInactiveText);
    config.writeEntry("frequency-view-colorButton",       m_colorButton);
    config.writeEntry("frequency-view-font",              m_font);
}


void   RadioViewFrequencyRadio::restoreState (const KConfigGroup &config)
{
    QColor defaultActive  (20, 244, 20),
           defaultInactive(QColor(10, 117, 10).light(75)),
           defaultButton  (10, 117, 10);
    QFont  defaultFont ("Helvetica");
    QColor a, b, c;
    QFont  f;
    a = config.readEntry("frequency-view-colorActiveText",
                         defaultActive);
    b = config.readEntry("frequency-view-colorInactiveText",
                         defaultInactive);
    c = config.readEntry("frequency-view-colorButton",
                         defaultButton);
    f = config.readEntry("frequency-view-font",
                         defaultFont);
    setDisplayColors(a, b, c);
    setDisplayFont(f);
}


ConfigPageInfo RadioViewFrequencyRadio::createConfigurationPage()
{
    DisplayConfiguration *a = new DisplayConfiguration(NULL);
    connectI(a);
    return ConfigPageInfo (a,
                           i18n("Frequency Display"),
                           i18n("Frequency Display"),
                           QString::null
                           );
}


// Interface

bool RadioViewFrequencyRadio::connectI(Interface *i)
{
    bool o = IDisplayCfg::connectI(i);
    bool c = ISoundStreamClient::connectI(i);
    bool a = false;
    bool b = false;
    if (dynamic_cast<IFrequencyRadio *>(i)) {

        a = IRadioDeviceClient::connectI(i);
        b = IFrequencyRadioClient::connectI(i);

    } else if (dynamic_cast<IInternetRadio *>(i)) {

        a = IRadioDeviceClient::connectI(i);
        b = IInternetRadioClient::connectI(i);

    }
    return o || a || b || c;
}


bool RadioViewFrequencyRadio::disconnectI(Interface *i)
{
    // no check for IFrequencyRadio, it's just a disconnect

    bool a = IRadioDeviceClient::disconnectI(i);
    bool b = IFrequencyRadioClient::disconnectI(i);
    bool c = IInternetRadioClient::disconnectI(i);
    bool d = ISoundStreamClient::disconnectI(i);
    bool e = IDisplayCfg::disconnectI(i);

    return a || b || c || d || e;
}

void RadioViewFrequencyRadio::noticeConnectedI (ISoundStreamServer *s, bool pointer_valid)
{
    ISoundStreamClient::noticeConnectedI(s, pointer_valid);
    if (s && pointer_valid) {
        s->register4_notifySignalQualityChanged(this);
        s->register4_notifyStereoChanged(this);
    }
}

// IDisplayCfg

bool RadioViewFrequencyRadio::setDisplayColors(const QColor &activeText,
                                               const QColor &inactiveText,
                                               const QColor &button)
{
    bool change = (activeText != m_colorActiveText || inactiveText != m_colorInactiveText || button != m_colorButton);

    m_colorActiveText   = activeText;
    m_colorInactiveText = inactiveText;
    m_colorButton       = button;

    QPalette pl = palette();

    QBrush fg  = pl.brush(QPalette::Inactive, QPalette::Foreground),
           btn = pl.brush(QPalette::Inactive, QPalette::Button),
           lgt = pl.brush(QPalette::Inactive, QPalette::Light),
           drk = pl.brush(QPalette::Inactive, QPalette::Dark),
           mid = pl.brush(QPalette::Inactive, QPalette::Mid),
           txt = pl.brush(QPalette::Inactive, QPalette::Text),
           btx = pl.brush(QPalette::Inactive, QPalette::BrightText),
           bas = pl.brush(QPalette::Inactive, QPalette::Base),
           bg  = pl.brush(QPalette::Inactive, QPalette::Background);

    fg .setColor(m_colorActiveText);
    btn.setColor(m_colorButton);
    lgt.setColor(m_colorButton.light(180));
    drk.setColor(m_colorButton.light( 50));
    mid.setColor(m_colorInactiveText);
    txt.setColor(m_colorActiveText);
    btx.setColor(m_colorActiveText);
    bas.setColor(m_colorButton);
    bg .setColor(m_colorButton);

    pl.setColorGroup(QPalette::Active,   fg, btn, lgt, drk, mid, txt, btx, bas, bg);
    pl.setColorGroup(QPalette::Inactive, fg, btn, lgt, drk, mid, txt, btx, bas, bg);
    setPalette(pl);

    #ifdef KRADIO_ENABLE_FIXMES
        #warning "FIXME: port KImageEffect stuff"
    #endif
/*    if (parentWidget() && parentWidget()->backgroundPixmap() ){
        KPixmapIO io;
        QImage  i = io.convertToImage(*parentWidget()->backgroundPixmap());
        KImageEffect::fade(i, 0.5, colorGroup().color(QColorGroup::Dark));
        setPaletteBackgroundPixmap(io.convertToPixmap(i));
        setBackgroundOrigin(WindowOrigin);
    } else {*/
        setBackgroundRole(QPalette::Button);
 //        setBackgroundColor(palette().color(QPalette::Button));
//     }

    if (change)
        notifyDisplayColorsChanged(m_colorActiveText, m_colorInactiveText, m_colorButton);
    return true;
}

bool RadioViewFrequencyRadio::setDisplayFont (const QFont &f)
{
    if (m_font != f) {
        m_font = f;
        notifyDisplayFontChanged(m_font);
        RadioViewElement::setFont(f);
    }
    return true;
}

// IRadioDeviceClient


bool RadioViewFrequencyRadio::noticePowerChanged (bool on, const IRadioDevice */*sender*/)
{
    m_power = on;
    if (m_power) {
        m_RadioTextTimer.start();
    } else {
        m_RadioTextTimer.stop();
    }

    SoundStreamID ssid = queryCurrentSoundStreamSinkID();
    float q = 0.0;
    bool  s = false;
    querySignalQuality(ssid, q);
    noticeSignalQualityChanged(ssid, q);
    queryIsStereo(ssid, s);
    noticeStereoChanged(ssid, s);

    resetRadioTextVisualBuffer();
    
    update();
    return true;
}


bool RadioViewFrequencyRadio::noticeStationChanged (const RadioStation &/*rs*/, const IRadioDevice */*sender*/)
{
/*    const InternetRadioStation  *irs = dynamic_cast<const InternetRadioStation  *>(&rs);
    const FrequencyRadioStation *frs = dynamic_cast<const FrequencyRadioStation *>(&rs);
    if (irs) {
        noticeURLChanged      (irs->url(),       irs);
    } else if (frs) {
        noticeFrequencyChanged(frs->frequency(), frs);
    }*/
//     m_RadioTextRing = QString();
    return false;   // we don't care
}


bool RadioViewFrequencyRadio::noticeDescriptionChanged (const QString &, const IRadioDevice */*sender*/)
{
    return false;   // we don't care
}

bool RadioViewFrequencyRadio::noticeRDSStateChanged(bool enabled, const IRadioDevice */*sender*/)
{
    m_RDS_enabled = enabled;
    update();
    return true;
}

bool RadioViewFrequencyRadio::noticeRDSRadioTextChanged(const QString &s, const IRadioDevice */*sender*/)
{
    QString tmp = s;
    tmp.replace(QRegExp("\\s"), " ");
    if (m_RDSRadioText != tmp) {
        m_RDSRadioText           = tmp;
//         updateRadioTextRing();
        update();
    }
    return true;
}

bool RadioViewFrequencyRadio::noticeRDSStationNameChanged(const QString &s, const IRadioDevice */*sender*/)
{
    if (m_RDSStationName != s) {
        m_RDSStationName = s;
        update();
    }
    return true;
}


// IRadioSoundClient

bool RadioViewFrequencyRadio::noticeSignalQualityChanged(SoundStreamID id, float q)
{
    if (queryCurrentSoundStreamSinkID() != id)
        return false;
    m_quality = q;
    update();
    return true;
}


bool RadioViewFrequencyRadio::noticeStereoChanged(SoundStreamID id, bool  s)
{
    if (queryCurrentSoundStreamSinkID() != id)
        return false;
    m_stereo = s;
    update();
    return true;
}




// IInternetRadioClient

bool RadioViewFrequencyRadio::noticeURLChanged(const KUrl &url, const InternetRadioStation *irs)
{
    m_url          = url;
    m_station_name = "";
    if (irs) {
        m_station_name = irs->name();
    }
    update();
    return true;
}

// IFrequencyRadioClient

bool RadioViewFrequencyRadio::noticeFrequencyChanged(float f, const FrequencyRadioStation *frs)
{
    m_frequency = f;
    m_station_name = "";
    if (frs) {
        m_station_name = frs->name();
    }
    if (m_frequency > 0) {
        if (m_frequency < 10) {
            m_text_frequency = i18n("%1 kHz", KGlobal::locale()->formatNumber((int)(m_frequency * 1000), 0));
        } else {
            m_text_frequency = i18n("%1 MHz", KGlobal::locale()->formatNumber(m_frequency, 2));
        }
    } else {
        m_text_frequency = QString();
    }
    update();
    return true;
}


bool RadioViewFrequencyRadio::noticeMinMaxFrequencyChanged(float /*min*/, float /*max*/)
{
    return false;   // we don't care
}


bool RadioViewFrequencyRadio::noticeDeviceMinMaxFrequencyChanged(float /*min*/, float /*max*/)
{
    return false;   // we don't care
}


bool RadioViewFrequencyRadio::noticeScanStepChanged(float /*s*/)
{
    return false;   // we don't care
}



void RadioViewFrequencyRadio::paintEvent(QPaintEvent *e)
{
    BlockProfiler p("RadioViewFrequencyRadio::paintEvent");

    QRect eraseRect = e->rect();
    bool  onlyReadioText = m_radioTextRect.toRect().contains(eraseRect);

    // guarantee that the painter is destroyed before we call the QFrame::paintEvent
    {

    QPainter paint(this);
    paint.setRenderHints(paint.renderHints() | QPainter::Antialiasing);

    QRect   cr = contentsRect();

    qreal   fw     = frameWidth();
    qreal   height = cr.height() - 2 * fw;
    qreal   width  = cr.width()  - 2 * fw;
    qreal   bottom = cr.bottom() - fw;
    qreal   right  = cr.right()  - fw;
    qreal   x      = cr.x() + fw;
    qreal   y      = cr.y() + fw;

    QFont   f = m_font;

    // auxiliary variables
    qreal   margin = qMax(4.0, qMin(width / 50.0, height / 50.0));
    qreal   tmp    = qMin(height, (width - 2 * margin) / qreal(6.0));
    qreal   xd_st  = qMin((height - margin * 2) / 3.0, tmp / 3.0);
    qreal   xw     = qMin(tmp / 2.0, xd_st * 1.5);
    qreal   penw   = qMax(1.0, xw / 25.0);

    // position and height of stereo symbol
    qreal   xh_st = xd_st;
    qreal   xx_st = x + margin + xw + 2 * margin + penw/2.0;
    qreal   xy_st = y + margin + penw/2.0;
    qreal   xw_st = 1.5 * xd_st;

    // position and height of AM symbol
    qreal   xx_am = xx_st + xw_st + 2 * margin;
    qreal   xy_am = y + margin;
    qreal   xh_am = xh_st;
    f.setPixelSize(xh_am);
    qreal   xw_am = QFontMetricsF(f).width(m_text_am);

    // position and height of FM symbol
    qreal   xx_fm = xx_am + xw_am + 2 * margin;
    qreal   xy_fm = y + margin;
    qreal   xh_fm = xh_st;
    f.setPixelSize(xh_fm);
    qreal   xw_fm = QFontMetricsF(f).width(m_text_fm);

    // position and height of FM symbol
    qreal   xx_net = xx_fm + xw_fm + 2 * margin;
    qreal   xy_net = y + margin;
    qreal   xh_net = xh_st;
    f.setPixelSize(xh_net);
    qreal   xw_net = QFontMetricsF(f).width(m_text_net);

    // position and height of RDS symbol
    qreal   xx_rds = xx_net + xw_net + 2 * margin;
    qreal   xy_rds = y + margin;
    qreal   xh_rds = xh_st;
    f.setPixelSize(xh_rds);
//     qreal   xw_rds = QFontMetricsF(f).width(m_text_rds);

    // position and height of triangle
    qreal   xh_sg = height - margin * 2;
    qreal   xx_sg = x + margin;
    qreal   xy_sg = y + margin;

    // position and height of frequency
    qreal   xx_f = xx_st;                        // left aligned with stereo symb
    qreal   xy_f = xy_st + xh_st + margin/2.0;       // directly below stereo symbol
    qreal   xw_f = right  - margin - xx_f + 1;
    qreal   xh_f = (bottom - margin - xy_f + 1) * 2.0/3.0;

    // position and height of radio text
    qreal   xx_rt = xx_st;
    qreal   xy_rt = xy_f + xh_f + margin/2.0;
    qreal   xw_rt = right  - margin - xx_rt + 1;
    qreal   xh_rt = (bottom - margin - xy_f + 1) * 1.0/3.0;


    const QPalette &pl = palette();
    QPalette::ColorGroup cg = isEnabled() ? (isActiveWindow() ? QPalette::Active : QPalette::Inactive) : QPalette::Disabled;

    m_activePen     = QPen(pl.brush(cg, QPalette::Text), penw, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    m_inactivePen   = QPen(pl.brush(cg, QPalette::Mid),  penw, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    m_activeBrush   = pl.brush(cg, QPalette::Text);
    m_inactiveBrush = pl.brush(QPalette::Mid);

    if (!onlyReadioText) {

        // draw stereo symbol
        paint.setPen(  (m_stereo && m_power) ? m_activePen : m_inactivePen);
        paint.drawArc(QRectF( xx_st,            xy_st, (xd_st - penw), (xd_st - penw)), 0, 360*16);
        paint.drawArc(QRectF((xx_st + xd_st/2), xy_st, (xd_st - penw), (xd_st - penw)), 0, 360*16);

        // draw signal quality symbol
        qreal cx = xx_sg,
              cy = xy_sg,
              cw = xw,
              ch = xw;

        qreal open_a = 30.0;
        // outer circle
        paint.setPen(  (m_quality > 0.75 && m_power) ? m_activePen : m_inactivePen);
        paint.drawArc(QRectF(cx, cy, cw, ch), (int)(-90+open_a)*16, (int)(360 - 2*open_a)*16);

        // mid circle
        paint.setPen(  (m_quality > 0.50 && m_power) ? m_activePen : m_inactivePen);
        cx += xw/5.0;  cy += xw/5.0;
        cw -= xw/2.5;  ch -= xw/2.5;
        paint.drawArc(QRectF(cx, cy, cw, ch), (int)(-90+open_a)*16, (int)(360 - 2*open_a)*16);

        // inner circle
        paint.setPen(  (m_quality > 0.25 && m_power) ? m_activePen : m_inactivePen);
        cx += xw/5.0;  cy += xw/5.0;
        cw -= xw/2.5;  ch -= xw/2.5;
        paint.drawArc(QRectF(cx, cy, cw, ch), (int)(-90+open_a)*16, (int)(360 - 2*open_a)*16);

        // triangle
        QPen tmppen =    (m_quality > 0.1 && m_power) ? m_activePen   : m_inactivePen;
        tmppen.setWidth(1);
        paint.setPen(tmppen);
        paint.setBrush(  (m_quality > 0.1 && m_power) ? m_activeBrush : m_inactiveBrush);
        QPointF pts[3];
        pts[0] = QPointF((xx_sg + xw      / 4.0), (xy_sg + xh_sg - penw/2.0));
        pts[1] = QPointF((xx_sg + xw *3.0 / 4.0), (xy_sg + xh_sg - penw/2.0));
        pts[2] = QPointF((xx_sg + xw      / 2.0), (xy_sg + xw/2.0  + penw));
        paint.drawConvexPolygon(pts, 3);


        // AM/FM display


        paint.setPen (  (m_frequency > 0 && m_frequency <= 10 && m_power) ? m_activePen : m_inactivePen);
        f.setPixelSize(xh_am);
        paint.setFont(f);
        paint.drawText(QPointF(xx_am, xy_am + xh_am - 1), m_text_am);

        paint.setPen (  (m_frequency > 0 && m_frequency >  10 && m_power) ? m_activePen : m_inactivePen);
        f.setPixelSize(xh_fm);
        paint.setFont(f);
        paint.drawText(QPointF(xx_fm, xy_fm + xh_fm - 1), m_text_fm);

        paint.setPen (  (m_url.isValid() > 0 && m_power) ? m_activePen : m_inactivePen);
        f.setPixelSize(xh_net);
        paint.setFont(f);
        paint.drawText(QPointF(xx_net, xy_net + xh_net - 1), m_text_net);

        // RDS display
        paint.setPen (  (m_RDS_enabled && m_power) ? m_activePen : m_inactivePen);
        f.setPixelSize(xh_rds);
        paint.setFont(f);
        paint.drawText(QPointF(xx_rds, xy_rds + xh_rds - 1), m_text_rds);

        // Frequency Display

        QString s;
        if (m_frequency > 0 && !m_url.isValid()) {
            s = m_text_frequency;
        } else if (m_url.isValid()) {
            s = m_station_name;
        }

        qreal pxs = xh_f;
        paint.setPen (  m_power ? m_activePen : m_inactivePen);
        f.setPixelSize((int)pxs);
        int n = 30;
        while (1) {
            QFontMetricsF fm(f);
            qreal sw = fm.boundingRect(QRectF(xx_f, xy_f, xw_f, xh_f), Qt::AlignRight | Qt::AlignVCenter, s).width();
            if (sw <= xw_f || --n <= 0) break;

            qreal fact = xw_f / sw;
            pxs = qMin(pxs - 1, pxs * fact);
            f.setPixelSize(qMax(1,(int)pxs));
        }
        paint.setFont(f);
        paint.drawText(QRectF(xx_f, xy_f, xw_f, xh_f), Qt::AlignRight | Qt::AlignVCenter, s);
    }

    // RDS Radio Text
    if (m_power && m_RDSRadioText.length()) {
        updateRadioTextVisualBuffer(QRectF(xx_rt, xy_rt, xw_rt, xh_rt));
        paintRadioTextVisualBuffer(paint);
    }



    } // end of the block in which the QPainter exists. It will be destroyed before calling QFrame::paintEvent
      // otherwise, QPainter class will complain that more than one painter exists for this widget
    QFrame::paintEvent(e);
}


void RadioViewFrequencyRadio::resizeEvent(QResizeEvent *e)
{
    RadioViewElement::resizeEvent(e);
}

bool RadioViewFrequencyRadio::event(QEvent *e)
{
    const bool ret = RadioViewElement::event(e);
    if (e->type() == QEvent::ParentChange) {
        setDisplayColors(m_colorActiveText, m_colorInactiveText, m_colorButton);
    }
    return ret;
}

void RadioViewFrequencyRadio::slotRadioTextTimer()
{
    //IErrorLogClient::staticLogDebug("RadioViewFrequencyRadio::slotRadioTextTimer()");
    BlockProfiler p("RadioViewFrequencyRadio::slotRadioTextTimer");

    advanceRadioTextVisualBuffer();

//     m_RadioTextX0      -= m_RadioTextDX;
    if (m_RDSRadioText.length()) {
//         m_RadioTextRepaint =  true;
        update(m_radioTextRect.toRect());
    }
}

void RadioViewFrequencyRadio::resetRadioTextVisualBuffer()
{
    // reset radio text buffer
    updateRadioTextVisualBuffer(QRectF(0, 0, 0, 0));
}



void RadioViewFrequencyRadio::updateRadioTextVisualBuffer(QRectF newVisualRect)
{
    QSize newBufferSize(newVisualRect.width() * m_radioTextVisualBufferOverSizeFactor, ceil(newVisualRect.height()));
    if (m_radioTextVisualBufferSize != newBufferSize) {
        m_radioTextRect             = newVisualRect;
        m_radioTextVisualBufferSize = newBufferSize;
        m_radioTextVisualBuffer     = QPixmap(newBufferSize);
        QBitmap    transpMask(newBufferSize);
        transpMask.clear();
        m_radioTextVisualBuffer.setMask(transpMask);
        m_radioTextVisualBuffer.fill(Qt::transparent);
        m_radioTextVisualBufferCurrentReadX  = 0;
        m_radioTextVisualBufferCurrentWriteX = 0;

        QFont f = m_font;
        f.setPixelSize(qMax(1, m_radioTextVisualBufferSize.height()));
        QFontMetricsF fm(f);
        m_RadioTextDX = qMax(1.0, floor(fm.width(QString(" ")) / 2.0 + 0.5));

//         // DEBUG
//         if (!m_bufferVisualizer) {
//             m_bufferVisualizer = new TmpWidget();
//             m_bufferVisualizer->setWindowTitle("visualizer test");
//         }
//         m_bufferVisualizer->resize(m_radioTextVisualBufferSize);
//         m_bufferVisualizer->show();
    }
}


void RadioViewFrequencyRadio::advanceRadioTextVisualBuffer()
{
    // unwrapped investigation
    while (m_radioTextRect.isValid() && m_radioTextVisualBufferSize.isValid()) {
        qreal uw_newReadX  = m_radioTextVisualBufferCurrentReadX + m_RadioTextDX;
        qreal uw_newReadX2 = uw_newReadX + m_radioTextRect.width();
        qreal uw_writeX    = m_radioTextVisualBufferCurrentWriteX + (m_radioTextVisualBufferCurrentWriteX < m_radioTextVisualBufferCurrentReadX ? m_radioTextVisualBufferSize.width() : 0);
        if (uw_newReadX2 > uw_writeX) {
            QFont    f     = m_font;
            QPainter paint;
            paint.begin(&m_radioTextVisualBuffer);
            paint.setBackgroundMode(Qt::TransparentMode);
            paint.setBackground(Qt::transparent);
            paint.setCompositionMode(QPainter::CompositionMode_Source);

            paint.setPen        (  m_power ? m_activePen : m_inactivePen);
            f.setPixelSize(qMax(1, m_radioTextVisualBufferSize.height()));
            paint.setFont(f);
            QRectF textBoundingRect = drawTextInRadioTextVisualBuffer(paint);
            if (m_radioTextVisualBufferCurrentWriteX >= m_radioTextVisualBufferSize.width()) {
                m_radioTextVisualBufferCurrentWriteX -= m_radioTextVisualBufferSize.width() + textBoundingRect.width();
                drawTextInRadioTextVisualBuffer(paint);
            }
        } else {
            break;
        }
    }
    m_radioTextVisualBufferCurrentReadX = m_radioTextVisualBufferCurrentReadX + m_RadioTextDX;
    if (m_radioTextVisualBufferCurrentReadX > m_radioTextVisualBufferSize.width()) {
        m_radioTextVisualBufferCurrentReadX -= m_radioTextVisualBufferSize.width();
    }
//     if (m_bufferVisualizer) {
//         m_bufferVisualizer->setPixmap(m_radioTextVisualBuffer);
//     }
}


QRectF RadioViewFrequencyRadio::drawTextInRadioTextVisualBuffer(QPainter &paint)
{
    // FIXME: eraseBackground
    QString       theText = m_RDSRadioText + "     ";
    QRectF        textBoundingRect;
    QFontMetricsF fm(paint.font());
    paint.eraseRect(QRectF(m_radioTextVisualBufferCurrentWriteX, 0, fm.width(theText), m_radioTextVisualBufferSize.height()));
    paint.drawText(QRectF(m_radioTextVisualBufferCurrentWriteX,
                          0,
                          m_radioTextVisualBufferSize.width() - m_radioTextVisualBufferCurrentWriteX,
                          m_radioTextVisualBufferSize.height()),
                   Qt::AlignVCenter,
                   theText,
                   &textBoundingRect
                  );
    m_radioTextVisualBufferCurrentWriteX += textBoundingRect.width();
    return textBoundingRect;
}


void RadioViewFrequencyRadio::paintRadioTextVisualBuffer(QPainter &paint)
{
    qreal w    = m_radioTextRect.width();
    qreal dstx = m_radioTextRect.left();
    qreal srcx = m_radioTextVisualBufferCurrentReadX;
    if (w > m_radioTextVisualBufferSize.width() - m_radioTextVisualBufferCurrentReadX) {
        qreal w1 = m_radioTextVisualBufferSize.width() - m_radioTextVisualBufferCurrentReadX;
        paint.drawPixmap(QRectF(dstx, m_radioTextRect.top(), w1, m_radioTextRect.height()),
                         m_radioTextVisualBuffer,
                         QRectF(srcx, 0, w1, m_radioTextRect.height()));
        w    -= w1;
        dstx += w1;
        srcx  = 0;
    }
    paint.drawPixmap(QRectF(dstx, m_radioTextRect.top(), w, m_radioTextRect.height()),
                     m_radioTextVisualBuffer,
                     QRectF(srcx, 0, w, m_radioTextRect.height()));
}



#include "radioview_frequencyradio.moc"
