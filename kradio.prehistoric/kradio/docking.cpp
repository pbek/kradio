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

// this is the position of the very first station in the list
// if the list is empty, it *needs* to be the position of the Item with
// stationSeperatorID
#define STATION_0_POSITION 1

RadioDocking::RadioDocking(KRadio *_widget, RadioBase *_radio, const char *name)
  : KSystemTray (_widget, name)
{
  radio = _radio;
  widget = _widget;

  miniKRadioPixmap = UserIcon("kradio");

  // popup menu for right mouse button
  KPopupMenu *menu = contextMenu();

  currID   = menu->idAt(0);

  alarmID = menu->insertTitle (i18n("Wake Up"));
  stationSeparatorID = alarmID; // make sure the position of the corresponding item is STATION_0_POSITION !

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

}


void RadioDocking::contextMenuAboutToShow( KPopupMenu* menu )
{
  clearStationList();
  buildStationList();

  menu->changeTitle (currID, i18n("KRadio: ")+ widget->getStationString(true, true, true));
  menu->changeItem (powerID, radio->isPowerOn() ? i18n("Power Off") : i18n("Power On"));

  QDateTime a = radio->nextAlarm();

  if (a.isValid())
    menu->changeTitle (alarmID, i18n("next alarm: ") + a.toString());
  else
    menu->changeTitle (alarmID, i18n("<no alarm pending>"));

  // stolen from kmix
  for ( unsigned n=0; n<menu->count(); n++ )
    {
      if ( QString( menu->text( menu->idAt(n) ) )==i18n("&Quit") )
	menu->removeItemAt( n );
    }

  menu->insertItem( SmallIcon("exit"), i18n("&Quit" ), kapp, SLOT(quit()) );
}

RadioDocking::~RadioDocking()
{
}

void RadioDocking::clearStationList()
{
  // FIXME:: THIS IS *REALLY* NOT FLEXIBLE!!!!!
  while (contextMenu()->idAt(STATION_0_POSITION) != stationSeparatorID)
    contextMenu()->removeItemAt(STATION_0_POSITION);
}

void RadioDocking::buildStationList()
{
  for (int i = 0; i < radio->nStations(); ++i) {
    const RadioStation *stn = radio->getStation(i);
    QString StationText = stn->getLongName(i+1);

    int id = contextMenu()->insertItem(StationText, stn, SLOT(activate()), 0, -1, i+1);
    contextMenu()->setItemChecked (id, radio->getCurrentStation() == stn);
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

void RadioDocking::slotPowerToggle()
{
  widget->slotPowerClicked();
}

