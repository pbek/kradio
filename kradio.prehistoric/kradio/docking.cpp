/***************************************************************************
                          docking.cpp  -  description
                             -------------------
    begin                : Don Mär  8 21:57:17 CET 2001
    copyright            : (C) 2002 by Ernst Martin Witte
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

#include <kiconloader.h>
#include <kwin.h>
#include <kapp.h>
#include <klocale.h>
#include <kaction.h>
#include <kstdaction.h>
#include <math.h>

#include "docking.h"
#include "kradio.h"

RadioDocking::RadioDocking(KRadio *_widget, RadioBase *_radio, const char *name)
	: KSystemTray (_widget, name)
{
	radio = _radio;
	widget = _widget;

  	miniKRadioPixmap = UserIcon("kradio");

  	// popup menu for right mouse button
  	menu = new KPopupMenu();
  	
  	menu->insertTitle (SmallIcon("kradio"), "kradio tray control");
  	currID   = menu->insertTitle (i18n("Current"));
  	alarmID = menu->insertTitle (i18n("Wake Up"));
  	
	stationSeparatorID = menu->insertSeparator();
	
  	nextID = menu->insertItem(i18n("Search Next Station"), this, SLOT( slotSearchNextStation()));
  	menu->changeItem(nextID, SmallIcon("forward"), i18n("Search Next Station"));
	
  	prevID = menu->insertItem(i18n("Search Previous Station"), this, SLOT( slotSearchPrevStation()));
  	menu->changeItem(prevID, SmallIcon("back"), i18n("Search Previous Station"));

  	powerID = menu->insertItem(i18n("Power On"), this, SLOT( slotPowerToggle()));

  	menu->insertSeparator();

  	menu->changeItem(menu->insertItem(i18n("&About"),
  	                                        parentWidget(),
  	                                        SLOT( slotAbout())),
  	                    SmallIcon("kradio"), i18n("About"));
  	
  	toggleID = menu->insertItem(i18n("Restore"), this, SLOT (toggleWindowState()));
  	
  	(KStdAction::quit (widget, SLOT(close()))) -> plug (menu);
}


RadioDocking::~RadioDocking()
{
}

void RadioDocking::mousePressEvent(QMouseEvent *e)
{
  	if ( e->button() == LeftButton )
		toggleWindowState();

  	if ( e->button() == RightButton  || e->button() == MidButton) {
    	int x = e->x();
    	int y = e->y();

    	QString text;
    	if (widget->isVisible())
      		text = i18n("&Minimize");
    	else
      		text = i18n("&Restore");
    	menu->changeItem (toggleID, text);
    	
    	while (menu->idAt(3) != stationSeparatorID)
    		menu->removeItemAt(3);
    	
	  	for (int i = 0; i < radio->nStations(); ++i) {
  			const RadioStation *x = radio->getStation(i);
	  		QString StationText = x->name();
  			if (!StationText.isEmpty())
  				StationText += ", ";
  			StationText = QString().setNum(i+1) + " " +  StationText +
  			              QString().setNum(x->getFrequency(), 'f', 2) + " MHz";
  			
  			int id = menu->insertItem(i18n(StationText), x, SLOT(activate()), 0, -1, i+3);
  			menu->setItemChecked (id, radio->getCurrentStation() == x);
	  	}
	  	
	  	menu->changeTitle (currID, i18n(widget->getStationString(true, true, true)));
	  	
	  	menu->changeItem (powerID, i18n(radio->isPowerOn() ? "Power Off" : "Power On"));
	  	
	  	QDateTime a = radio->nextAlarm();
	  	if (a.isValid())
		  	menu->changeTitle (alarmID, "next alarm: " + a.toString());
		else
		  	menu->changeTitle (alarmID, "<no alarm pending>");
		
    	
    	menu->popup (mapToGlobal(QPoint(x, y)));
    	menu->exec();
  	}
}

void RadioDocking::toggleWindowState() {
    if (radio != 0) {
        if (widget->isVisible()) {
            widget->showMinimized();
        } else {
            widget->showNormal();
        }
    }
}

/*void RadioDocking::slotSelStation(int id)
{
	for (int i = 0; i < MAX_STATIONS; ++i) {
		if (StationIDs[i] == id) {
		  	StationButton ** Stations = radio->getStations();
		  	Stations[i]->animateClick();
		}
	}
}
*/

void RadioDocking::slotSearchNextStation()
{
	radio->startSeek(true);
}

void RadioDocking::slotSearchPrevStation()
{
	radio->startSeek(false);
}

void RadioDocking::slotNOP()
{
}

void RadioDocking::slotPowerToggle()
{
	widget->slotPowerClicked();
}

