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
	if (!isHidden()) {
		KWin::Info  i = KWin::info(winId());
		saveSticky    = i.onAllDesktops;
		saveDesktop   = i.desktop;
		saveGeometry  = geometry();
	}
}


void QuickBar::rebuildGUI()
{
	for (ciButtonList i = Buttons.begin(); i != Buttons.end(); ++i) {
		delete *i;
	}
	Buttons.clear();

	if (layout) delete layout;
	layout = new ButtonFlowLayout(this);

	layout->setMargin(2);
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
       	KPushButton *b = showShortName ?
							   new KPushButton((*i)->getShortName(), this)
							 : new KPushButton((*i)->name(), this);

	    b->setToggleButton(true);
       	if (iconstr.length()) {
       		b->setIconSet (QPixmap(iconstr));
       	}
        b->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));

        QToolTip::add(b, (*i)->getLongName());
        if (isVisible()) b->show();
        b->resize (b->sizeHint());

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
  bool wasHidden = isHidden();
  QWidget::show();
  emit toggled(true);

  if (wasHidden) {
	KWin::setOnAllDesktops(winId(), saveSticky);
	KWin::setType(winId(), NET::Toolbar);
	setGeometry(saveGeometry);
  }
}


void QuickBar::hide()
{
  getState();
  QWidget::hide();
  emit toggled(false);
}


void QuickBar::slotFrequencyChanged(float, const RadioStation *s)
{
  setCaption ((s ? QString(s->name()) : i18n("KRadio")));
  if (buttonGroup) buttonGroup->setButton(radio->currentStation());
}


void QuickBar::resizeEvent (QResizeEvent *e)
{
  // minimumSize might change because of the flow layout
  if (layout) setMinimumSize(layout->minimumSize());
  QWidget::resizeEvent (e);
}


void QuickBar::setShowShortName (bool b)
{
	showShortName = b;
	rebuildGUI();
}
