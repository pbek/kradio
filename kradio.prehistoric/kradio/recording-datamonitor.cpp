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
#include "recording-context.h"
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
	  m_channels(0)
{
	setFrameStyle(Box | Sunken);
	setLineWidth(1);
	setMidLineWidth(1);
	
	setChannels(2);

	setColors(QColor(20, 244, 20),
			  QColor(10, 117, 10));

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}


RecordingDataMonitor::~RecordingDataMonitor()
{
	setChannels(0);
}


bool RecordingDataMonitor::connect(Interface *i)
{
	bool a = IRecordingClient::connect(i);
	return a;
}

bool RecordingDataMonitor::disconnect(Interface *i)
{
	bool a = IRecordingClient::disconnect(i);
	return a;
}


// IRecordingClient

bool RecordingDataMonitor::noticeRecordingStarted()
{
	setEnabled(true);
	repaint();
	return true;
}


bool RecordingDataMonitor::noticeMonitoringStarted()
{
	setEnabled(true);
	repaint();
	return true;
}


bool RecordingDataMonitor::noticeRecordingStopped()
{
	setEnabled(false);
	repaint();
	return true;
}


bool RecordingDataMonitor::noticeMonitoringStopped()
{
	setEnabled(false);
	repaint();
	return true;
}


bool RecordingDataMonitor::noticeRecordingConfigChanged(const RecordingConfig &)
{
	return false;
}


bool RecordingDataMonitor::noticeRecordingContextChanged(const RecordingContext &ctx)
{
	const int            *buffer   = ctx.buffer();
	      int             nSamples = ctx.samplesInBuffer();
	const RecordingConfig &cfg     = ctx.config();

	int bias = 0;
	setChannels(cfg.channels);
	m_maxValue = cfg.maxValue();
	if (!cfg.sign) {
		m_maxValue /= 2;
		bias = -m_maxValue;
	}

	int c = 0;
	for (int s = 0; s < nSamples; ++s, ++c) {
		if (c >= m_channels) c -= m_channels;   // avoid slow c = s % m_channels

		int &m = m_channelsMax[c];
		int x = abs(buffer[s] + bias);
		if (m < x) m = x;
		m_channelsAvg[c] += x;
	}
	for (int i = 0; i < m_channels; ++i)
		m_channelsAvg[i] /= nSamples;
//	kdDebug() << "maxVal: " << m_maxValue << endl;
//	kdDebug() << "max:    " << m_channelsMax[0] << endl;
//	kdDebug() << "avg:    " << m_channelsAvg[0] << endl;

	repaint(false);
	return true;
}


// own stuff

void RecordingDataMonitor::setChannels(int n)
{
	if (n != m_channels) {
		if (m_channelsMax) delete m_channelsMax;
		if (m_channelsAvg) delete m_channelsAvg;
		m_channels = n > 0 ? n : 0;
		if (m_channels > 0) {
			m_channelsMax = new int[m_channels];
			m_channelsAvg = new double[m_channels];
		} else {
			m_channelsMax = NULL;
			m_channelsAvg = NULL;
		}
	}

	for (int i = 0; i < m_channels; ++i) {
		m_channelsMax[i] = 0;
		m_channelsAvg[i] = 0;
	}
	setMinimumSize(QSize(W_MIN, (m_channels + 1 )* CHANNEL_H_MIN));
}


// QT/KDE ...

void RecordingDataMonitor::drawContents(QPainter *painter)
{
	if (!painter) return;
	if (m_channels <= 0) return;

	QRect r = contentsRect();

	QPen activePen (colorGroup().color(QColorGroup::Text), 1);
	QPen inactivePen (colorGroup().color(QColorGroup::Mid), 1);
	QBrush activeBrush = colorGroup().brush(QColorGroup::Text);
	QBrush inactiveBrush = colorGroup().brush(QColorGroup::Mid);
	QBrush yellowBrush(QColor(255,255,0));
	QBrush orangeBrush(QColor(255,192,0));
	QBrush redBrush   (QColor(255,0, 0));


	double  ranges [5] = { 0.75, 0.83, 0.91, 1.0, 999 };
	QBrush *brushes[5] = { &activeBrush, &yellowBrush, &orangeBrush, &redBrush, &redBrush };

	painter->setBrush( isEnabled() ? activeBrush : inactiveBrush);

	int nBlocks  = (r.width()-1)  / BLOCK_W_MIN;
	int xoffs    = (r.width()-1)  % BLOCK_W_MIN;
	int chHeight = (r.height()-1-CHANNEL_H_MIN) / m_channels;
	int yoffs    = (r.height()-1) % m_channels;

	double min_dB = 20*log10(1 / (double)m_maxValue );
	
	int x0 = xoffs/2 + r.top();
	int y = yoffs/2 + r.left();
	for (int c = 0; c < m_channels; ++c) {
		int x = x0;

//		double dB    = 20*log10(m_channelsAvg[c] / (double)m_maxValue );
		double dBMax = 20*log10(m_channelsMax[c] / (double)m_maxValue );

		int activeBlocks = (int)rint(nBlocks * (min_dB - dBMax) / min_dB);
//		int activeBlocks = (int)rint(nBlocks * (min_dB - dB)    / min_dB);
//		int maxBlock     = (int)rint(nBlocks * (min_dB - dBMax) / min_dB);
		int range = 0;
		
		for (int b = 0; b < nBlocks; ++b) {
		    if (b >= nBlocks * ranges[range]) ++range;
			painter->fillRect(x+1, y+1, BLOCK_W_MIN-1, chHeight-1,
			                  b < activeBlocks ? *brushes[range] : inactiveBrush);
/*			if (b == maxBlock) {
				painter->fillRect(x + BLOCK_W_MIN - 3, y+1, 3, chHeight-1,
			                      *brushes[range]);
			}
*/			x += BLOCK_W_MIN;
		}

		y += chHeight;
	}

	QFont f("Helvetica");
	painter->setPen (activePen);
	f.setPixelSize(CHANNEL_H_MIN);
	painter->setFont(f);

    int maxW = QFontMetrics(f).width(QString().setNum((int)min_dB) + " dB");
	int delta_dB  = 5;
	while (abs(min_dB) / delta_dB * maxW * 2 > r.width()) delta_dB *= 2;
	
	for (int dB = 0; dB >= min_dB; dB -= delta_dB) {
        QString txt = QString().setNum(dB) + " dB";
        int w = QFontMetrics(f).width(txt);
		int x = x0 + (int)(nBlocks * BLOCK_W_MIN * (min_dB - dB) / min_dB) - w;
		if (x < x0) continue;		
		painter->drawText(x, y + CHANNEL_H_MIN, txt);
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
