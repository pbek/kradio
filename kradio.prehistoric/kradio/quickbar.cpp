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


#include "quickbar.h"

#include <qtooltip.h>
#include <qnamespace.h>
#include <qhbuttongroup.h>
#include <qvbuttongroup.h>
#include <qtoolbutton.h>

#include <kwin.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>

#include "radiostation.h"
#include "buttonflowlayout.h"


QuickBar::QuickBar(QWidget * parent, const char * name)
  : QWidget(parent, name)
{
	layout = NULL;
	buttonGroup = NULL;
	showShortNames = true;
	currentFrequency = -1;
}


QuickBar::~QuickBar()
{
}


void QuickBar::restoreState (KConfig *config)
{
    config->setGroup("QuickBar");

	saveDesktop  = config->readNumEntry ("desktop", 1);
	saveSticky   = config->readBoolEntry("sticky",  false);
	saveGeometry = config->readRectEntry("Geometry");

    if (config->readBoolEntry("hidden", false))
        hide();
    else
        show();

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
}




void QuickBar::getState()
{
	if (isVisible()) {
		KWin::Info  i = KWin::info(winId());
		saveSticky    = i.onAllDesktops;
		saveDesktop   = i.desktop;
		saveGeometry  = geometry();
	}
}


void QuickBar::rebuildGUI()
{
	if (layout) delete layout;
	layout = new ButtonFlowLayout(this);

	layout->setMargin(1);
	layout->setSpacing(2);

	if (buttonGroup) delete buttonGroup;
	buttonGroup = new QButtonGroup(this);
	
	// we use buttonGroup to enable automatic toggle/untoggle
	buttonGroup->setExclusive(true);
	buttonGroup->setFrameStyle(QFrame::NoFrame);

    int index = 0;
    for (ciStationVector i = stations.begin(); i != stations.end(); ++i, ++index) {

        QString iconstr = i->getIconString();
       	QToolButton *b = new QToolButton(this, "");

        b->setText(showShortName ? i->getShortName() : QString(i->name()));

	    b->setToggleButton(true);
       	if (iconstr.length()) {
       		b->setIconSet (QPixmap(iconstr));
       	}
        b->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));

        QToolTip::add(b, i->getLongName());
        if (isVisible()) b->show();

        connect (b, SIGNAL(clicked()), &(*i), SLOT(activate()));
        
        buttonGroup->insert(b, index);
        layout->add(b);
    }
    
    // activate correct button
    buttonGroup->setButton(getButtonID(currentFrequency));

    // calculate geometry
    if (layout) {
		QRect r = geometry();
		int h = layout->heightForWidth( r.width());

		if (h > r.height())
			setGeometry(r.x(), r.y(), r.width(), h);
    }
}


void QuickBar::configurationChanged(const SetupData &d)
{
	showShortName = d.displayOnlyShortNames;
	stations.clear();
	for (ciStationVector i = d.stations.begin(); i != d.stations.end(); ++i) {

		if (i->useQuickSelect()) {

			stations.push_back(*i);
			
			connect (stations.back(), SIGNAL(activated(const RadioStation *)),
					 this, SLOT(stationActivated(const RadioStation *)));
		}
	}
    rebuildGUI();
}


void QuickBar::setOn(bool on)
{
	if (on && !isVisible())
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


void QuickBar::frequencyChanged(float f, const RadioStation *s)
{
	// set caption from paramter "s" if we do not have a quick select button
	setCaption ((s ? QString(s->name()) : "KRadio"));
	
    currentFrequency = f;

    int stID = getButtonID(currentFrequency);
	if (buttonGroup)
		buttonGroup->setButton (stID);
	if (stID >= 0 && stID < stations.size())
		setCaption(stations[stID].name());
}


int QuickBar::getButtonID(float freq)
{
	int k = 0;
	for (ciStationVector i = stations.begin(); i != stations.end(); ++i, ++k) {
		if (i->hasFrequency(f))
			return k;
	}
	return -1;
}


void QuickBar::stationActivated(const RadioStation *st)
{
	if (st)
		emit sigSetFrequency(st->getFrequency());
}


void QuickBar::resizeEvent (QResizeEvent *e)
{
	// minimumSize might change because of the flow layout
	if (layout) {
		QSize marginSize(layout->margin()*2, layout->margin()*2);
		setMinimumSize(layout->minimumSize(e->size() - marginSize) + marginSize);
	}

	QWidget::resizeEvent (e);
}


void QuickBar::setGeometry (int x, int y, int w, int h)
{
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


void    QuickBar::connectInterface(QObjectList &ol)
{
	for (QObject *i = objects.first(); i; i = objects.next()) {
		if (this == i)
			continue;

		// configuration

		quietconnect (i, SIGNAL(sigConfigurationChanged(const SetupData &)),
					  this, SLOT(configurationChanged(const SetupData &)));
        quietconnect (i, SIGNAL(sigSaveState(KConfig *)),
		              this, SLOT(saveState(KConfig *)));
    	quietconnect (i, SIGNAL(sigRestoreState(KConfig *)),
				      this, SLOT(restoreState(KConfig *)));

    	// commands

		quietconnect (this, SIGNAL(sigSetFrequency(float)), i, SLOT(setFrequency(float)));

		// notifications

        quietconnect (i, SIGNAL(sigFrequencyChanged(float, const RadioStation*), this, SLOT(frequencyChanged(float, const RadioStation *)));
    }
}


