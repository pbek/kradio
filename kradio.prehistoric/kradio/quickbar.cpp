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
#include <qnamespace.h>
#include <qhbuttongroup.h>
#include <qvbuttongroup.h>
#include <kglobal.h>
#include <qtoolbutton.h>

#include <kwin.h>
#include <klocale.h>
#include <kglobal.h>

#include "quickbar.h"
#include "radiostation.h"
#include "buttonflowlayout.h"

QuickBar::QuickBar(RadioBase *_radio, QWidget * parent, const char * name)
  : QWidget(parent,name),
    layout(0),
    buttonGroup(0),
    showShortName(true)
{
    radio = _radio;

    connect (radio, SIGNAL(sigConfigChanged()), 
	    this, SLOT(slotConfigChanged()));
    connect(radio, SIGNAL(sigFrequencyChanged(float, const RadioStation *)), 
	    this, SLOT(slotFrequencyChanged(float, const RadioStation *)));

}


QuickBar::~QuickBar()
{
  if (layout){
    delete layout;
  }
}


void QuickBar::restoreState (KConfig *config)
{
    config->setGroup("QuickBar");

	saveDesktop = config->readNumEntry ("desktop", 1);
	saveSticky  = config->readBoolEntry("sticky",  false);

	saveGeometry = config->readRectEntry("Geometry");

/*	fprintf (stderr, "%i, %i -  %i x %i\n",
			 saveGeometry.x(),
			 saveGeometry.y(),
			 saveGeometry.width(),
			 saveGeometry.height());
*/
    if (config->readBoolEntry("hidden", false))
        hide();
    else
        show();

	showShortName = config->readBoolEntry("showShortName", true);

	rebuildGUI();
}


void QuickBar::saveState (KConfig *config)
{
	getState();

    config->setGroup("QuickBar");

    config->writeEntry("hidden", isHidden());

	config->writeEntry("sticky", saveSticky);
	config->writeEntry("desktop", saveDesktop);
	config->writeEntry("Geometry", saveGeometry);

	config->writeEntry("showShortName", showShortName);
}


void QuickBar::getState()
{
	if (isVisible()) {
		KWin::Info  i = KWin::info(winId());
		saveSticky    = i.onAllDesktops;
		saveDesktop   = i.desktop;
		saveGeometry  = geometry();
	}

/*	fprintf (stderr, "%i, %i -  %i x %i\n",
			 geometry().x(),
			 geometry().y(),
			 geometry().width(),
			 geometry().height());

*/
}


void QuickBar::rebuildGUI()
{
	for (ciButtonList i = Buttons.begin(); i != Buttons.end(); ++i) {
		delete *i;
	}
	Buttons.clear();

	if (layout) delete layout;
	layout = new ButtonFlowLayout(this);

	layout->setMargin(1);
	layout->setSpacing(2);

	if (buttonGroup) delete buttonGroup;
	buttonGroup = new QButtonGroup(this);
	
	// we use buttonGroup to enable automatic toggle/untoggle
	buttonGroup->setExclusive(true);
	buttonGroup->setFrameStyle(QFrame::NoFrame);

    const StationVector &stations = radio->getStations();

    int index=0;
    for (ciStationVector i = stations.begin(); i != stations.end(); ++i) {
		if ( !(*i)->useQuickSelect())
			continue;
		
        QString iconstr = (*i)->getIconString();
       	QToolButton *b = new QToolButton(this, "");

        b->setText(showShortName ? (*i)->getShortName() : QString((*i)->name()));

	    b->setToggleButton(true);
       	if (iconstr.length()) {
       		b->setIconSet (QPixmap(iconstr));
       	}
        b->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));

        QToolTip::add(b, (*i)->getLongName());
        if (isVisible()) b->show();
        // b->resize (b->sizeHint());

        Buttons.push_back(b);
        connect (b, SIGNAL(clicked()), (*i), SLOT(activate()));
        buttonGroup->insert(b,index);
        layout->add(b);
        index++;
    }
    
    // activate correct button
    buttonGroup->setButton(radio->currentStation());

    // calculate geometry
    if (layout) {
		QRect r = geometry();
		int h = layout->heightForWidth( r.width());

//		fprintf (stderr, "layout says: need height = %i\n", h);
		
		if (h > r.height())
			setGeometry(r.x(), r.y(), r.width(), h);
    }
}


void QuickBar::slotConfigChanged()
{
  rebuildGUI();
  // adjustSize();
}


void QuickBar::setOn(bool on)
{
  if(on && !isVisible())
    show();
  else if (!on && isVisible())
    hide();
}


void QuickBar::show()
{
  bool wasHidden = !isVisible();

  QWidget::show();

  if (wasHidden) {
	KWin::setOnAllDesktops(winId(), saveSticky);
	KWin::setType(winId(), NET::Toolbar);

/*	fprintf (stderr, "Quickbar::show(): %i, %i - %i x %i\n",
			 saveGeometry.x(), saveGeometry.y(),
			 saveGeometry.width(), saveGeometry.height());
*/	
	setGeometry(saveGeometry);
  }

  emit toggled(true);
}


void QuickBar::hide()
{
  getState();
  QWidget::hide();
  emit toggled(false);
}


void QuickBar::slotFrequencyChanged(float, const RadioStation *s)
{
    const StationVector &stations = radio->getStations();

	setCaption ((s ? QString(s->name()) : i18n("KRadio")));
	int k = -1, _k = -1;
	for (ciStationVector i = stations.begin(); k < 0 && i != stations.end(); ++i) {
		if ( !(*i)->useQuickSelect())
			continue;
		++_k;
		if (s && (*i)->getFrequency() == s->getFrequency())
			k = _k;
	}
	if (buttonGroup) buttonGroup->setButton(k);
}


void QuickBar::resizeEvent (QResizeEvent *e)
{
/*	fprintf (stderr, "QuickBar::resizeEvent: old = %i x %i, new = %i x %i\n",
			 e->oldSize().width(), e->oldSize().height(),
			 e->size().width(), e->size().height());
*/

	// minimumSize might change because of the flow layout
	if (layout) {
		QSize marginSize(layout->margin()*2, layout->margin()*2);
		setMinimumSize(layout->minimumSize(e->size() - marginSize) + marginSize);
	}

	QWidget::resizeEvent (e);
}


void QuickBar::setShowShortName (bool b)
{
	showShortName = b;
	rebuildGUI();
}

void QuickBar::setGeometry (int x, int y, int w, int h)
{
//	fprintf (stderr, "Quickbar::setGeometry(): %i, %i - %i x %i\n",
//			 x,y,w,h);
	if (layout) {
		QSize marginSize(layout->margin()*2, layout->margin()*2);
		setMinimumSize(layout->minimumSize(QSize(w, h) - marginSize) + marginSize);
	}
	QWidget::setGeometry (x, y, w, h);
}

void QuickBar::setGeometry (const QRect &r)
{
	setGeometry (r.x(), r.y(), r.width(), r.height());
}
