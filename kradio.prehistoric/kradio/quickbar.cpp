/***************************************************************************
                          quickbar.cpp  -  description
                             -------------------
    begin                : Mon Feb 11 2002
    copyright            : (C) 2002 by Martin Witte / Frank Schwanz
    email                : witte@kawo1.rwth-aachen.de / schwanz@fh-brandenburg.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qtooltip.h>
#include <kwin.h>
#include "quickbar.h"

QuickBar::QuickBar(RadioBase *_radio)
    : QWidget(0)
{
    radio = _radio;
    connect (radio, SIGNAL(sigConfigChanged()), this, SLOT(slotConfigChanged()));
    slotConfigChanged();
}

QuickBar::~QuickBar()
{
}

void QuickBar::restoreState (KConfig *c)
{
    c->setGroup("QuickBar");

    int x = c->readNumEntry("x", 0);
    int y = c->readNumEntry("y", 0);
    int w = c->readNumEntry("w", 0);
    int h = c->readNumEntry("h", 0);
    if (h && w)
        resize (w, h);
    move (x, y);
	
	int Desktop = c->readNumEntry("desktop", -1);
	bool sticky = c->readBoolEntry("sticky", true);
	KWin::setOnAllDesktops(winId(), sticky);
	if (sticky) KWin::setOnDesktop(winId(), Desktop);
}

void QuickBar::saveState (KConfig *c) const
{
    c->setGroup("QuickBar");
	
	KWin::Info  i = KWin::info(winId());
	c->writeEntry("sticky", i.onAllDesktops);
	c->writeEntry("desktop", i.desktop);
	
    c->writeEntry("x", pos().x());
    c->writeEntry("y", pos().y());
    c->writeEntry("w", size().width());
    c->writeEntry("h", size().height());
}

void QuickBar::slotConfigChanged()
{
	for (ciButtonList i = Buttons.begin(); i != Buttons.end(); ++i) {
		delete *i;
	}
	Buttons.clear();
	
    const StationVector &stations = radio->getStations();
    for (ciStationVector i = stations.begin(); i != stations.end(); ++i) {
        if ((*i)->useQuickSelect()) {
        	QString iconstr = (*i)->getIconString();
        	KPushButton *b = new KPushButton((*i)->getShortName(), this);

        	if (iconstr.length()) {
        		b->setIconSet (QPixmap(iconstr));
        	}

            QToolTip::add(b, (*i)->getLongName());
            b->resize (b->sizeHint());
    		Buttons.push_back(b);
    		connect (b, SIGNAL(clicked()), (*i), SLOT(activate()));
    	}
    }
    resize(size());
}


void QuickBar::resizeEvent (QResizeEvent *e)
{
    QWidget::resizeEvent (e);

    IntVector   c_w;
    int         c = 0,
                nc = Buttons.size();

    int x, y, maxy;
    QSize mys = size();
restart1:
    c_w.clear();
    c_w.insert(c_w.end(), nc, 0);
restart2:
    c = 0;
    x = y = maxy = 0;
	for (ciButtonList i = Buttons.begin(); i != Buttons.end(); ++i) {
  	    QSize s = (*i)->sizeHint();  	
        if (x && x + s.width() > mys.width()) {
   	        nc = c;
   	        goto restart1;
	    } else {
	        if (c_w[c] < s.width()) {
	            c_w[c] = s.width();
	            goto restart2;
	        }
	        s.setWidth(c_w[c]);
	    }
	    (*i)->move(x, y);
	    (*i)->resize(s.width(), s.height());
        maxy = s.height() > maxy ? s.height() : maxy;	
   	    x += s.width();
   	    if (++c >= nc) {
   	        c = 0;
   	        x = 0;
   	        y += maxy;
   	    }
	}
}
