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
#include <qtooltip.h>
#include <kdebug.h>

#include "docking.h"
#include "kradioapp.h"
#include "kradio.h"

RadioDocking::RadioDocking(KRadio *_widget, QuickBar * qb, RadioBase *_radio, const char *name)
  : KSystemTray (0, name),
    leftMouseTogglesQB(false),
    leftMouseTogglesUI(true)
{
  radio = _radio;
  radioControl = _widget;
  quickbar = qb;

  miniKRadioPixmap = UserIcon("kradio");

  // connect radio with tray
  connect(radio, SIGNAL(sigUpdateTips()),
	  this, SLOT(slotUpdateToolTips()));
  connect(radio, SIGNAL(sigAlarm()),
	  this, SLOT(slotAlarm()));
  connect(radio, SIGNAL(sigConfigChanged()),
  	  this, SLOT(slotConfigChanged()));
  	
  buildContextMenu ();
}

RadioDocking::~RadioDocking()
{
}


void RadioDocking::buildContextMenu()
{
  KPopupMenu *menu = contextMenu();
  menu->clear();

  titleID = menu->insertTitle (i18n("KRadio: ")+ radio->getStationString(true, true, true));

  buildStationList();

  alarmID = menu->insertTitle ("alarm-dummy");
  slotAlarm();

  menu->insertItem(SmallIcon("forward"), i18n("Search Next Station"),
			       this, SLOT( slotSearchNextStation()));

  menu->insertItem(SmallIcon("back"), i18n("Search Previous Station"),
	               this, SLOT( slotSearchPrevStation()));

  powerID = menu->insertItem("power-dummy", radio, SLOT( PowerToggle()));

  menu->insertSeparator();

  menu->insertItem(SmallIcon("kradio"), i18n("&About"), this, SIGNAL(showAbout()));

  menu->insertSeparator();

  guiID = menu->insertItem("gui-show-dummy", this, SLOT(slotToggleUI()) );
  qbID  = menu->insertItem("quickbar-dummy", this, SLOT(slotToggleQB()) );
//  showAllID = menu->insertItem("showall-dummy", this, SLOT(slotToogleAll()) );

  menu->insertItem( SmallIcon("exit"), i18n("&Quit" ), kapp, SLOT(quit()) );

  slotUpdateToolTips();
}

void RadioDocking::contextMenuAboutToShow( KPopupMenu* menu )
{
    bool b = radioControl->isMinimized() || radioControl->isHidden();
	menu->changeItem(guiID, b ? i18n("Show radio interface") :
                                i18n("Minimize radio interface"));
    b = quickbar->isMinimized() || quickbar->isHidden();
    menu->changeItem(qbID,  b ? i18n("Show station quickbar") :
                                i18n("Minimize station quickbar"));

}

void RadioDocking::buildStationList()
{
	KPopupMenu *m = contextMenu();
	StationIDs.clear();
	const RadioStation *c = radio->getCurrentStation();
  	for (int i = 0; i < radio->nStations(); ++i) {
    	const RadioStation *stn = radio->getStation(i);
    	QString StationText = QString().setNum(i+1) + " " + stn->getLongName();

    	int id = m->insertItem(StationText, stn, SLOT(activate()));
    	StationIDs.push_back(id);
    	m->setItemChecked (id, c == stn);
  	}
}

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

void RadioDocking::slotUpdateToolTips ()
{
  	QToolTip::add(this, radio->getStationString(true, true, true));

  	KPopupMenu *menu = contextMenu();
  	
	menu->changeTitle (titleID, i18n("KRadio: ")+ radio->getStationString(true, true, true));
  	
  	menu->changeItem(powerID, radio->isPowerOn() ? i18n("Power Off") : i18n("Power On"));
	
	const RadioStation *c = radio->getCurrentStation();
	for (uint i = 0; i < StationIDs.size(); ++i) {
    	const RadioStation *stn = radio->getStation(i);
    	menu->setItemChecked (StationIDs[i], c == stn);
	}
}


void RadioDocking::slotToggleUI ()
{
    if (radioControl->isMinimized() || radioControl->isHidden())
        radioControl->show();
    else
        radioControl->hide();
}


void RadioDocking::slotToggleQB ()
{
    if (quickbar->isMinimized() || quickbar->isHidden())
        quickbar->showNormal();
    else
        quickbar->hide();
}


void RadioDocking::slotAlarm()
{
  KPopupMenu *menu = contextMenu();
  QDateTime a = radio->nextAlarm();
  if (a.isValid())
    menu->changeTitle (alarmID, i18n("next alarm: ") + a.toString());
  else
    menu->changeTitle (alarmID, i18n("<no alarm pending>"));
}

void RadioDocking::slotConfigChanged()
{
	buildContextMenu();
}


void RadioDocking::showEvent (QShowEvent *)
{
}

void RadioDocking::mousePressEvent( QMouseEvent *e )
{
  KSystemTray::mousePressEvent(e);

  switch ( e->button() ) {
  case LeftButton:
    if (leftMouseTogglesQB) slotToggleQB ();
    if (leftMouseTogglesUI) slotToggleUI ();
    break;
  default:
	// nothing
	break;
  }
}
