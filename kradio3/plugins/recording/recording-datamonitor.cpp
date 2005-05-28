/***************************************************************************
                          recording-monitor-widget.cpp  -  description
                             -------------------
    begin                : So Sep 7 2003
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

#include "recording-datamonitor.h"
//#include "recording-context.h"
#include <math.h>

#include <qpainter.h>
#include <qimage.h>
#include <qpixmap.h>
#include <kimageeffect.h>  // fading, blending, ...
#include <kpixmapio.h>     // fast conversion between QPixmap/QImage
#include <limits.h>
#include <stdlib.h>

#define CHANNEL_H_MIN   20
#define BLOCK_W_MIN     10
#define W_MIN           (20 * (BLOCK_W_MIN))

RecordingDataMonitor::RecordingDataMonitor(QWidget *parent, const char *name)
    : QFrame(parent, name),
      m_channelsMax(NULL),
      m_channelsAvg(NULL),
      m_maxValue(INT_MAX),
      m_channels(0),
      m_pActiveBlocks(NULL)
{
    setFrameStyle(Box | Sunken);
    setLineWidth(1);
    setMidLineWidth(1);

    setChannels(2);

    setColors(QColor(20, 244, 20),
              QColor(10, 117, 10));

    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
}


RecordingDataMonitor::~RecordingDataMonitor()
{
    if (m_channelsMax)   delete[] m_channelsMax;
    if (m_channelsAvg)   delete[] m_channelsAvg;
    if (m_pActiveBlocks) delete[] m_pActiveBlocks;
}



// own stuff

void RecordingDataMonitor::setChannels(int n)
{
    if (n != m_channels) {
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
    setMinimumSize(QSize(W_MIN, (m_channels + 1 )* CHANNEL_H_MIN));
}


// QT/KDE ...

void RecordingDataMonitor::drawContents(QPainter *painter)
{
    if (painter)
        internalDrawContents(*painter, true);
}

void RecordingDataMonitor::internalDrawContents(QPainter &painter, bool repaintAll)
{
    if (m_channels <= 0) return;
    QRect r = contentsRect();

    QPen   activePen      (colorGroup().color(QColorGroup::Text), 1);
    QPen   inactivePen    (colorGroup().color(QColorGroup::Mid),  1);
    QBrush activeBrush   = colorGroup().brush(QColorGroup::Text);
    QBrush inactiveBrush = colorGroup().brush(QColorGroup::Mid);
    QBrush yellowBrush(QColor(255,255,0));
    QBrush orangeBrush(QColor(255,192,0));
    QBrush redBrush   (QColor(255,0, 0));


    double  ranges [5] = { 0.75, 0.83, 0.91, 1.0, 999 };
    QBrush *brushes[5] = { &activeBrush, &yellowBrush, &orangeBrush, &redBrush, &redBrush };

    painter.setBrush( isEnabled() ? activeBrush : inactiveBrush);

    int nBlocks  = (r.width()-1)  / BLOCK_W_MIN;
    int xoffs    = (r.width()-1)  % BLOCK_W_MIN;
    int chHeight = (r.height()-1-CHANNEL_H_MIN) / m_channels;
    int yoffs    = (r.height()-1) % m_channels;

    double min_dB = 20*log10(1 / (double)m_maxValue );

    int x0 = xoffs/2 + r.top();
    int y = yoffs/2 + r.left();
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
        f.setPixelSize(CHANNEL_H_MIN);
        painter.setFont(f);

        int maxW = QFontMetrics(f).width(QString().setNum((int)min_dB) + " dB");
        int delta_dB  = 5;
        while (abs((long)min_dB) / delta_dB * maxW * 2 > r.width()) delta_dB *= 2;

        for (int dB = 0; dB >= min_dB; dB -= delta_dB) {
            QString txt = QString().setNum(dB) + " dB";
            int w = QFontMetrics(f).width(txt);
            int x = x0 + (int)(nBlocks * BLOCK_W_MIN * (min_dB - dB) / min_dB) - w;
            if (x < x0) continue;
            painter.drawText(x, y + CHANNEL_H_MIN, txt);
        }
    }
}


bool RecordingDataMonitor::setColors(const QColor &activeText,
                                     const QColor &button)
{
    m_colorActiveText = activeText;
    m_colorButton     = button;

    QPalette pl    = palette();
    QColorGroup cg = pl.inactive();

    QBrush fg  = cg.brush(QColorGroup::Foreground),
           btn = cg.brush(QColorGroup::Button),
           lgt = cg.brush(QColorGroup::Light),
           drk = cg.brush(QColorGroup::Dark),
           mid = cg.brush(QColorGroup::Mid),
           txt = cg.brush(QColorGroup::Text),
           btx = cg.brush(QColorGroup::BrightText),
           bas = cg.brush(QColorGroup::Base),
           bg  = cg.brush(QColorGroup::Background);

    fg.setColor (m_colorActiveText);
    btn.setColor(m_colorButton);
    lgt.setColor(m_colorButton.light(180));
    drk.setColor(m_colorButton.light( 50));
    mid.setColor(m_colorButton.light( 75));
    txt.setColor(m_colorActiveText);
    btx.setColor(m_colorActiveText);
    bas.setColor(m_colorButton);
    bg.setColor (m_colorButton);

    QColorGroup ncg(fg, btn, lgt, drk, mid, txt, btx, bas, bg);
    pl.setInactive(ncg);
    pl.setActive(ncg);
    setPalette(pl);

    if (parentWidget() && parentWidget()->backgroundPixmap() ){
        KPixmapIO io;
        QImage  i = io.convertToImage(*parentWidget()->backgroundPixmap());
        KImageEffect::fade(i, 0.5, colorGroup().color(QColorGroup::Dark));
        setPaletteBackgroundPixmap(io.convertToPixmap(i));
        setBackgroundOrigin(WindowOrigin);
    } else {
        setBackgroundColor(colorGroup().color(QColorGroup::Button));
    }

    return true;
}


bool RecordingDataMonitor::noticeSoundStreamData(SoundStreamID /*id*/,
    const SoundFormat &sf, const char *data, unsigned size,
    const SoundMetaData &/*md*/
)
{
    if (!isEnabled())
        return false;
    int nSamples = size / sf.frameSize();
    int sample_size = sf.sampleSize();

    int bias = 0;
    setChannels(sf.m_Channels);
    int old_max = m_maxValue;
    m_maxValue = sf.maxValue();
    if (!sf.m_IsSigned) {
        m_maxValue /= 2;
        bias = -m_maxValue;
    }

    int c = 0;
    for (int s = 0; s < nSamples; ++s, ++c, data += sample_size) {
        if (c >= m_channels) c -= m_channels;   // avoid slow c = s % m_channels

        int &m = m_channelsMax[c];
        int x = abs(sf.convertSampleToInt(data, false) + bias);
        if (m < x) m = x;
        m_channelsAvg[c] += x;
    }
    for (int i = 0; i < m_channels; ++i)
        m_channelsAvg[i] /= nSamples;

    QPainter paint(this);
    if (m_maxValue != old_max) {
        repaint(true);
    } else {
        internalDrawContents(paint, false);
    }
    return true;
}


#include "recording-datamonitor.moc"
