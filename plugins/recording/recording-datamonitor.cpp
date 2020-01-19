/***************************************************************************
                          recording-monitor-widget.cpp  -  description
                             -------------------
    begin                : So Sep 7 2003
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

#include "recording-datamonitor.h"
//#include "recording-context.h"
#include <math.h>

#include <QPainter>
#include <QImage>
#include <QPixmap>
#include <QPalette>

// #include <kimageeffect.h>  // fading, blending, ...
// #include <kpixmapio.h>     // fast conversion between QPixmap/QImage
#include <klocalizedstring.h>

#include <limits.h>
#include <stdlib.h>


#define CHANNEL_H_MIN   20
#define TEXT_H          15
#define TIC_H            4
#define MARGIN           3
#define BLOCK_W_MIN     10
#define W_MIN           (20 * (BLOCK_W_MIN))

RecordingDataMonitor::RecordingDataMonitor(QWidget *parent)
    : QFrame(parent),
      m_channelsMax(NULL),
      m_channelsAvg(NULL),
      m_maxValue(INT_MAX),
      m_channels(0),
      m_pActiveBlocks(NULL),
      m_paintEventUpdateMode(updateDefault)
{
    setFrameStyle(QFrame::Box | QFrame::Raised);
    setLineWidth(1);
    setMidLineWidth(1);

    setChannels(2);

    setColors(QColor(20, 244, 20),
              QColor(10, 117, 10)
             );

    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
}


RecordingDataMonitor::~RecordingDataMonitor()
{
    if (m_channelsMax)   delete[] m_channelsMax;
    if (m_channelsAvg)   delete[] m_channelsAvg;
    if (m_pActiveBlocks) delete[] m_pActiveBlocks;
}



// own stuff

void RecordingDataMonitor::setChannels(int n, bool force)
{
    if (n != m_channels || force) {
        if (m_channelsMax)   delete[] m_channelsMax;
        if (m_channelsAvg)   delete[] m_channelsAvg;
        if (m_pActiveBlocks) delete[] m_pActiveBlocks;
        m_channels = n > 0 ? n : 0;
        if (m_channels > 0) {
            m_channelsMax   = new int[m_channels];
            m_channelsAvg   = new double[m_channels];
            m_pActiveBlocks = new int[m_channels];
            for (int i = 0; i < m_channels; ++i) {
                m_pActiveBlocks[i] = 0;
            }
        } else {
            m_channelsMax   = NULL;
            m_channelsAvg   = NULL;
            m_pActiveBlocks = NULL;
        }
    }

    for (int i = 0; i < m_channels; ++i) {
        m_channelsMax[i]   = 0;
        m_channelsAvg[i]   = 0;
    }
    setMinimumSize(QSize(W_MIN, m_channels* CHANNEL_H_MIN + TEXT_H + TIC_H));

    update(updateAllForced);
}


// QT/KDE ...

void RecordingDataMonitor::paintEvent(QPaintEvent *e)
{
    bool updateAll = m_paintEventUpdateMode != updatePartially;

    QPainter painter(this);
    painter.setRenderHints(painter.renderHints() | QPainter::Antialiasing);

    internalDrawContents(painter, updateAll);
    if (updateAll) {
        QFrame::paintEvent(e);
    }
    m_paintEventUpdateMode = updateDefault;
}


void RecordingDataMonitor::internalDrawContents(QPainter &painter, bool repaintAll)
{
    if (m_channels <= 0) return;
    QRect r  = contentsRect();
    int   fw = frameWidth();
    r.moveTo(fw, fw);
    r.setSize(r.size() - QSize(2*fw, 2*fw));

    const QPalette &pl = palette();

    QPen   activePen      (pl.color(QPalette::Text), 1);
    QBrush activeBrush   = pl.brush(QPalette::Text);
    QBrush inactiveBrush = pl.brush(QPalette::Mid);

    bool a = pl.currentColorGroup() == QPalette::Active;
    QBrush yellowBrush(a ? m_yellowActive : m_yellowInactive);
    QBrush orangeBrush(a ? m_orangeActive : m_orangeInactive);
    QBrush redBrush   (a ? m_redActive    : m_redInactive);

    if (repaintAll) {
        painter.fillRect(r, pl.brush(QPalette::Background));
    }

    r.moveTo(MARGIN, MARGIN);
    r.setSize(r.size() - QSize(2*MARGIN, 2*MARGIN));

    double  ranges [5] = { 0.75, 0.83, 0.91, 1.0, 9e99 };
    QBrush *brushes[5] = { &activeBrush, &yellowBrush, &orangeBrush, &redBrush, &redBrush};

    painter.setBrush( activeBrush);

    int nBlocks  = (r.width ()-1) / BLOCK_W_MIN;
    int xoffs    = (r.width ()-1) % BLOCK_W_MIN;
    int chHeight = (r.height()-1  - TEXT_H - TIC_H) / m_channels;
    int yoffs    = (r.height()-1) % m_channels;

    double min_dB = 20*log10(1 / (double)m_maxValue );

    int x0 = xoffs/2 + r.left();
    int y  = yoffs/2 + r.top();
    for (int c = 0; c < m_channels; ++c) {
        int x = x0;


        int startBlock      = 0;
        int endBlock        = nBlocks - 1;
        int oldActiveBlocks = m_pActiveBlocks[c];

        double dBMax = isEnabled() ? 20*log10(m_channelsMax[c] / (double)m_maxValue ) : min_dB;

        m_pActiveBlocks[c] = m_channelsMax[c] ? (int)rint(nBlocks * (min_dB - dBMax) / min_dB) : 0;

        if (!repaintAll) {
            if (oldActiveBlocks > m_pActiveBlocks[c]) {
                startBlock = m_pActiveBlocks[c];
                endBlock   = oldActiveBlocks - 1;
            } else {
                startBlock = oldActiveBlocks;
                endBlock   = m_pActiveBlocks[c]-1;
            }
        }

        int range = 0;

        x += BLOCK_W_MIN * startBlock;
        for (int b = startBlock; b <= endBlock; ++b) {
            while (b >= nBlocks * ranges[range]) ++range;
            painter.fillRect(x+1, y+1, BLOCK_W_MIN-1, chHeight-1,
                             b < m_pActiveBlocks[c] ? *brushes[range] : inactiveBrush);
            x += BLOCK_W_MIN;
        }

        y += chHeight;
    }

    if (repaintAll) {
        QFont f("Helvetica");
        painter.setPen (activePen);
        f.setPixelSize(TEXT_H);
        painter.setFont(f);

        int maxW = QFontMetrics(f).width(i18n("%1 dB", (int)min_dB));
        int delta_dB  = 5;
        while (abs((long)min_dB) / delta_dB * maxW * 2 > r.width()) delta_dB *= 2;

        for (int dB = 0; dB >= min_dB; dB -= delta_dB) {
            // print text label
            QString txt = i18n("%1 dB", dB);
            int w = QFontMetrics(f).width(txt);
            int x = x0 + (int)(nBlocks * BLOCK_W_MIN * (min_dB - dB) / min_dB);
            if (x - w < x0) continue;
            painter.drawText(x - w, y + TEXT_H + TIC_H, txt);

            // print tick for more precise indication
            painter.drawLine(x, y, x, y + TIC_H);
        }
    }
}


bool RecordingDataMonitor::setColors(const QColor &activeText,
                                     const QColor &button)
{
    m_colorActiveText          = activeText;
    m_colorActiveButton        = button;

    QColor colorDisabledText   = activeText.lighter(50);
    QColor colorDisabledButton = button    .lighter(70);

    qreal inactive_fact_s = 0.60;
    qreal inactive_fact_v = 0.80;
    qreal th, ts, tv;
    qreal bh, bs, bv;
    activeText.getHsvF(&th, &ts, &tv);
    button    .getHsvF(&bh, &bs, &bv);
    ts *= inactive_fact_s;    bv *= inactive_fact_v;
    bs *= inactive_fact_s;    tv *= inactive_fact_v;

    QColor colorInactiveText   = QColor::fromHsvF(th, ts, tv);
    QColor colorInactiveButton = QColor::fromHsvF(bh, bs, bv);

    m_redActive    = QColor(255,0, 0);
    m_orangeActive = QColor(255,192,0);
    m_yellowActive = QColor(255,255,0);

    qreal rh, rs, rv;
    qreal oh, os, ov;
    qreal yh, ys, yv;
    m_redActive   .getHsvF(&rh, &rs, &rv);
    m_orangeActive.getHsvF(&oh, &os, &ov);
    m_yellowActive.getHsvF(&yh, &ys, &yv);
    rs *= inactive_fact_s;    rv *= inactive_fact_v;
    os *= inactive_fact_s;    ov *= inactive_fact_v;
    ys *= inactive_fact_s;    yv *= inactive_fact_v;

    m_redInactive    = QColor::fromHsvF(rh, rs, rv);
    m_orangeInactive = QColor::fromHsvF(oh, os, ov);
    m_yellowInactive = QColor::fromHsvF(yh, ys, yv);

    QPalette pl    = palette();

    QBrush a_fg  = pl.brush(QPalette::Active,   QPalette::Foreground),
           a_btn = pl.brush(QPalette::Active,   QPalette::Button),
           a_lgt = pl.brush(QPalette::Active,   QPalette::Light),
           a_drk = pl.brush(QPalette::Active,   QPalette::Dark),
           a_mid = pl.brush(QPalette::Active,   QPalette::Mid),
           a_txt = pl.brush(QPalette::Active,   QPalette::Text),
           a_btx = pl.brush(QPalette::Active,   QPalette::BrightText),
           a_bas = pl.brush(QPalette::Active,   QPalette::Base),
           a_bg  = pl.brush(QPalette::Active,   QPalette::Background);

    QBrush i_fg  = pl.brush(QPalette::Inactive, QPalette::Foreground),
           i_btn = pl.brush(QPalette::Inactive, QPalette::Button),
           i_lgt = pl.brush(QPalette::Inactive, QPalette::Light),
           i_drk = pl.brush(QPalette::Inactive, QPalette::Dark),
           i_mid = pl.brush(QPalette::Inactive, QPalette::Mid),
           i_txt = pl.brush(QPalette::Inactive, QPalette::Text),
           i_btx = pl.brush(QPalette::Inactive, QPalette::BrightText),
           i_bas = pl.brush(QPalette::Inactive, QPalette::Base),
           i_bg  = pl.brush(QPalette::Inactive, QPalette::Background);

    QBrush d_fg  = pl.brush(QPalette::Disabled, QPalette::Foreground),
           d_btn = pl.brush(QPalette::Disabled, QPalette::Button),
           d_lgt = pl.brush(QPalette::Disabled, QPalette::Light),
           d_drk = pl.brush(QPalette::Disabled, QPalette::Dark),
           d_mid = pl.brush(QPalette::Disabled, QPalette::Mid),
           d_txt = pl.brush(QPalette::Disabled, QPalette::Text),
           d_btx = pl.brush(QPalette::Disabled, QPalette::BrightText),
           d_bas = pl.brush(QPalette::Disabled, QPalette::Base),
           d_bg  = pl.brush(QPalette::Disabled, QPalette::Background);

    a_fg .setColor(m_colorActiveText);
    a_btn.setColor(m_colorActiveButton);
    a_lgt.setColor(m_colorActiveButton.lighter(180));
    a_drk.setColor(m_colorActiveButton.lighter( 50));
    a_mid.setColor(m_colorActiveButton.lighter( 75));
    a_txt.setColor(m_colorActiveText);
    a_btx.setColor(m_colorActiveText);
    a_bas.setColor(m_colorActiveButton);
    a_bg .setColor(m_colorActiveButton);

    i_fg .setColor(colorInactiveText);
    i_btn.setColor(colorInactiveButton);
    i_lgt.setColor(colorInactiveButton.lighter(180));
    i_drk.setColor(colorInactiveButton.lighter( 50));
    i_mid.setColor(colorInactiveButton.lighter( 75));
    i_txt.setColor(colorInactiveText);
    i_btx.setColor(colorInactiveText);
    i_bas.setColor(colorInactiveButton);
    i_bg .setColor(colorInactiveButton);

    d_fg .setColor(colorDisabledText);
    d_btn.setColor(colorDisabledButton);
    d_lgt.setColor(colorDisabledButton.lighter(180));
    d_drk.setColor(colorDisabledButton.lighter( 50));
    d_mid.setColor(colorDisabledButton.lighter( 75));
    d_txt.setColor(colorDisabledText);
    d_btx.setColor(colorDisabledText);
    d_bas.setColor(colorDisabledButton);
    d_bg .setColor(colorDisabledButton);

    pl.setColorGroup(QPalette::Active,   a_fg, a_btn, a_lgt, a_drk, a_mid, a_txt, a_btx, a_bas, a_bg);
    pl.setColorGroup(QPalette::Inactive, i_fg, i_btn, i_lgt, i_drk, i_mid, i_txt, i_btx, i_bas, i_bg);
    pl.setColorGroup(QPalette::Disabled, d_fg, d_btn, d_lgt, d_drk, d_mid, d_txt, d_btx, d_bas, d_bg);
    setPalette(pl);

    #ifdef KRADIO_ENABLE_FIXMES
        #warning "FIXME: port shading of background pixmap"
    #endif
/*    if (parentWidget() && parentWidget()->backgroundPixmap() ){
        KPixmapIO io;
        QImage  i = io.convertToImage(*parentWidget()->backgroundPixmap());
        KImageEffect::fade(i, 0.5, colorGroup().color(QColorGroup::Dark));
        setPaletteBackgroundPixmap(io.convertToPixmap(i));
        setBackgroundOrigin(WindowOrigin);
    } else {*/
//         setBackgroundColor(palette().color(QPalette::Button));
//     }

    return true;
}


void RecordingDataMonitor::reset()
{
    setChannels(m_channels, true);
}

bool RecordingDataMonitor::noticeSoundStreamData(SoundStreamID /*id*/,
    const SoundFormat &sf, const char *data, size_t size, size_t &/*consumed_size*/,
    const SoundMetaData &/*md*/
)
{
    if (!isEnabled())
        return false;
    int nFrames  = size / sf.frameSize();

//     int bias = 0;
    setChannels(sf.m_Channels);
    int old_max = m_maxValue;
    m_maxValue = sf.maxValue();
    if (!sf.m_IsSigned) {
        m_maxValue /= 2;
//         bias = -m_maxValue;
    }

    double  minMag[m_channels];
    double  maxMag[m_channels];
    double  avgMag[m_channels];
    sf.minMaxAvgMagnitudePerChannel(data, nFrames, minMag, maxMag, avgMag);
    for (int ch = 0; ch < m_channels; ++ch) {
        m_channelsAvg[ch] = avgMag[ch];
        m_channelsMax[ch] = maxMag[ch];
    }

    update(m_maxValue != old_max ? updateAllForced : updatePartially);
    return true;
}


void RecordingDataMonitor::setUpdateMode(PaintEventUpdateOptions opt)
{
    if ((opt == updateAllForced) || (m_paintEventUpdateMode == updateDefault)) {
        m_paintEventUpdateMode = opt;
    }
}

void RecordingDataMonitor::update(PaintEventUpdateOptions opt)
{
    setUpdateMode(opt);
    QWidget::update();
}


