/***************************************************************************
                          kradiodisplay.cpp  -  description
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

#include "utils.h"
 
#include <qpainter.h>
#include <qimage.h>
#include <qpixmap.h>
#include <kimageeffect.h>  // fading, blending, ...
#include <kpixmapio.h>     // fast conversion between QPixmap/QImage
#include "radioview_frequencyradio.h"

RadioViewFrequencyRadio::RadioViewFrequencyRadio(QWidget *parent, const QString &name )
	: RadioViewElement(parent, name, clsRadioDisplay),
	  m_power(false),
	  m_valid(false),
	  m_frequency(0),
	  m_quality(false),
	  m_stereo(false)
{
	setFrameStyle(Box | Sunken);
	setLineWidth(1);
	setMidLineWidth(1);

	QPalette pl = palette();
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
	       
	fg.setColor (QColor( 20, 244,  20));
	btn.setColor(QColor( 77, 117,  77));
	lgt.setColor(QColor(134, 214, 134));
	drk.setColor(QColor(  4,  66,   4));
	mid.setColor(QColor( 36,  87,  36));
	txt.setColor(QColor( 20, 244,  20));
	btx.setColor(QColor( 20, 244,  20));
	bas.setColor(QColor(  4,  66,   4));
	bg.setColor (QColor(  4,  66,   4));

	/* Benötigte Farben

	* active text => button text, text
	* inactive text <= mid
	* Button => *2=light, *.5=dark, *.75=mid

	nicht benötigte Farben:

	* foreground == text
	* background == button
	* base == background

	*/

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
}


RadioViewFrequencyRadio::~RadioViewFrequencyRadio(){
}


float RadioViewFrequencyRadio::getUsability (Interface *i) const
{
	if (dynamic_cast<IFrequencyRadio*>(i))
		return 1.0;
	else
		return 0.0;
}


// Interface

bool RadioViewFrequencyRadio::connect(Interface *i)
{
	if (dynamic_cast<IFrequencyRadio *>(i)) {

		bool a = IRadioDeviceClient::connect(i);
		bool b = IRadioSoundClient::connect(i);
		bool c = IFrequencyRadioClient::connect(i);
		return a || b || c;
		
	} else {
		return false;
	}
}


bool RadioViewFrequencyRadio::disconnect(Interface *i)
{
	// no check for IFrequencyRadio, it's just a disconnect

	bool a = IRadioDeviceClient::disconnect(i);
	bool b = IRadioSoundClient::disconnect(i);
	bool c = IFrequencyRadioClient::disconnect(i);

	return a || b || c;
}


// IRadioDeviceClient


bool RadioViewFrequencyRadio::noticePowerChanged (bool on, const IRadioDevice */*sender*/)
{
	m_power = on;
	repaint();
	return true;
}


bool RadioViewFrequencyRadio::noticeStationChanged (const RadioStation &, const IRadioDevice */*sender*/)
{
	return false;   // we don't care
}


// IRadioSoundClient


bool RadioViewFrequencyRadio::noticeVolumeChanged(float /*v*/)
{
	return false;   // we don't care
}


bool RadioViewFrequencyRadio::noticeSignalQualityChanged(float q)
{
	m_quality = q;
	repaint ();
	return true;
}


bool RadioViewFrequencyRadio::noticeSignalQualityChanged(bool /*good*/)
{
	return false;   // we don't care
}


bool RadioViewFrequencyRadio::noticeSignalMinQualityChanged(float /*q*/)
{
	return false;   // we don't care
}


bool RadioViewFrequencyRadio::noticeStereoChanged(bool  s)
{
	m_stereo = s;
	repaint ();
	return true;
}


bool RadioViewFrequencyRadio::noticeMuted(bool /*m*/)
{
	return false;   // we don't care
}



// IFrequencyRadioClient


bool RadioViewFrequencyRadio::noticeFrequencyChanged(float f, const RadioStation *)
{
	m_frequency = f;
	repaint ();
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



void RadioViewFrequencyRadio::drawContents(QPainter *paint)
{
	if (!paint) return;

	QRect r = contentsRect();

	int  margin = QMAX(4, QMIN(r.width() / 50, r.height() / 50)),
		 tmp    = QMIN(r.height(), (r.width() - 2*margin) / 4),
		 xd_st  = QMIN((r.height() - margin * 2) / 3, tmp/3),
		 xw     = QMIN(tmp / 2, xd_st * 3 / 2),
		 penw   = QMAX(1, xw / 25),
		 xh_st = xd_st,
		 xx_st = r.x() + margin + xw + 2 * margin + penw/2,
		 xy_st = r.y() + margin + penw/2,

		 xx_am = xx_st,
		 xy_am = xy_st + xh_st + margin / 2,
		 xh_am = (r.bottom() - margin - xy_am + 1 - margin/2) / 2,
		 
		 xx_fm = xx_am,
		 xy_fm = xy_am + xh_am + margin/2,
		 xh_fm = xh_am,

		 xh_sg = r.height() - margin * 2,
		 xx_sg = r.x() + margin,
		 xy_sg = r.y() + margin;
		 
	QPen activePen (colorGroup().color(QColorGroup::Text), penw);
	QPen inactivePen (colorGroup().color(QColorGroup::Mid), penw);
	QBrush activeBrush = colorGroup().brush(QColorGroup::Text);
	QBrush inactiveBrush = colorGroup().brush(QColorGroup::Mid);

	// draw stereo symbol
	paint->setPen(  (m_stereo && m_power) ? activePen : inactivePen);
	paint->drawArc((int)xx_st, (int)xy_st,
				   (int)(xd_st - penw), (int)(xd_st - penw),
				   0, 360*16);
	paint->drawArc((int)(xx_st + xd_st/2), (int)xy_st,
				   (int)(xd_st - penw), (int)(xd_st - penw),
				   0, 360*16);
				   
	// draw signal quality symbol
	float cx = xx_sg,
		  cy = xy_sg,
		  cw = xw,
		  ch = xw;

	float open_a = 30.0;
	// outer circle
	paint->setPen(  (m_quality > 0.75 && m_power) ? activePen : inactivePen);
	paint->drawArc((int)round(cx),       (int)round(cy),
	               (int)round(cw),       (int)round(ch),
	               (int)(-90+open_a)*16, (int)(360 - 2*open_a)*16
	               );

    // mid circle
	paint->setPen(  (m_quality > 0.50 && m_power) ? activePen : inactivePen);
	cx += (float)xw/5.0;  cy += (float)xw/5.0;
	cw -= (float)xw/2.5;  ch -= (float)xw/2.5;
	paint->drawArc((int)round(cx),       (int)round(cy),
	               (int)round(cw),       (int)round(ch),
	               (int)(-90+open_a)*16, (int)(360 - 2*open_a)*16
	               );

	// inner circle
	paint->setPen(  (m_quality > 0.25 && m_power) ? activePen : inactivePen);
	cx += (float)xw/5.0;  cy += (float)xw/5.0;
	cw -= (float)xw/2.5;  ch -= (float)xw/2.5;
	paint->drawArc((int)round(cx),       (int)round(cy),
	               (int)round(cw),       (int)round(ch),
	               (int)(-90+open_a)*16, (int)(360 - 2*open_a)*16
	               );

	// triangle
	QPen tmppen =   (m_quality > 0.1 && m_power) ? activePen : inactivePen;
	tmppen.setWidth(1);
	paint->setPen(tmppen);
	paint->setBrush(  (m_quality > 0.1 && m_power) ? activeBrush : inactiveBrush);
	QPointArray pts(3);
	pts.setPoint(0, (int)(xx_sg + xw / 4),  (int)(xy_sg + xh_sg - penw/2));
	pts.setPoint(1, (int)(xx_sg + xw *3/4), (int)(xy_sg + xh_sg - penw/2));
	pts.setPoint(2, (int)(xx_sg + xw / 2),  (int)(xy_sg + xw/2  + penw));
	paint->drawConvexPolygon(pts);



	// AM/FM display

	QFont f("Helvetica");
	paint->setPen (  (m_frequency <= 10 && m_power) ? activePen : inactivePen);
	f.setPixelSize(xh_am);
	paint->setFont(f);
	paint->drawText(xx_am, xy_am + xh_am - 1, i18n("AM"));
	int xw_am = QFontMetrics(f).width(i18n("AM"));

	paint->setPen (  (m_frequency > 10 && m_power) ? activePen : inactivePen);
	f.setPixelSize(xh_fm);
	paint->setFont(f);
	paint->drawText(xx_fm, xy_fm + xh_fm - 1, i18n("FM"));
	int xw_fm = QFontMetrics(f).width(i18n("FM"));

	int xx_f = QMAX(xx_fm + xw_fm, QMAX(xw_am + xx_am, QMAX(xx_st + xw, xw + xx_sg))) + margin,
		xy_f = r.y() + margin,
		xw_f = r.right() - margin - xx_f + 1,
		xh_f = r.bottom() - margin - xy_f + 1;

	// Frequency Display

	QString s;
	if (m_frequency < 10) {
		s = QString().setNum((int)(m_frequency * 1000)) + " kHz";
	} else {
		s = QString().setNum((int)m_frequency) + ".";
		int r = (int)round((m_frequency-(int)m_frequency) * 100);
		s = s + (r < 10 ? "0" : "") + QString().setNum(r) + " MHz";
	}

	float pxs = xh_f;
	paint->setPen (  m_power ? activePen : inactivePen);
	f.setPixelSize((int)pxs);
	int n = 30;
	while (1) {	
		QFontMetrics fm(f);
		int sw = fm.boundingRect(xx_f, xy_f, xw_f, xh_f, Qt::AlignRight | Qt::AlignVCenter, s).width();
		if (sw <= xw_f || --n <= 0) break;
		
		float fact = (float)xw_f / (float)sw;
		pxs = QMIN(pxs - 1, pxs * fact);
		f.setPixelSize(QMAX(1,(int)pxs));
	}
	paint->setFont(f);
	paint->drawText(xx_f, xy_f, xw_f, xh_f, Qt::AlignRight | Qt::AlignVCenter, s);
}

