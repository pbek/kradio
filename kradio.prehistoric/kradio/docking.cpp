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

RadioDocking::RadioDocking(KRadio *_widget, RadioBase *_radio, const char *name)
  : KSystemTray (_widget, name)
{
  radio = _radio;

  miniKRadioPixmap = UserIcon("kradio");

  // connect radio with tray
  connect(radio, SIGNAL(sigUpdateTips()),
	  this, SLOT(slotUpdateToolTips()));
}


void RadioDocking::contextMenuAboutToShow( KPopupMenu* menu )
{
  menu->clear();

  menu->insertTitle (i18n("KRadio: ")+ radio->getStationString(true, true, true));

  buildStationList();

  QDateTime a = radio->nextAlarm();
  if (a.isValid())
    menu->insertTitle (i18n("next alarm: ") + a.toString());
  else
    menu->insertTitle (i18n("<no alarm pending>"));

  menu->insertItem(SmallIcon("forward"), i18n("Search Next Station"),
			       this, SLOT( slotSearchNextStation()));

  menu->insertItem(SmallIcon("back"), i18n("Search Previous Station"),
	               this, SLOT( slotSearchPrevStation()));

  menu->insertItem(radio->isPowerOn() ? i18n("Power Off") : i18n("Power On"), radio, SLOT( PowerToggle()));

  menu->insertSeparator();

  menu->insertItem(SmallIcon("kradio"), i18n("&About"), this, SIGNAL(showAbout()));

  menu->insertSeparator();

  menu->insertItem( parentWidget()->isMinimized() ? i18n("Show radio interface") :
                                                    i18n("Minimize radio interface"),
                    this, SLOT(slotToggleUI()) );
  menu->insertItem( i18n("Show quick buttons"), this, SLOT(slotNOP()) );

  menu->insertItem( SmallIcon("exit"), i18n("&Quit" ), kapp, SLOT(quit()) );
}

RadioDocking::~RadioDocking()
{
}

void RadioDocking::buildStationList()
{
  for (int i = 0; i < radio->nStations(); ++i) {
    const RadioStation *stn = radio->getStation(i);
    QString StationText = stn->getLongName(i+1);

    int id = contextMenu()->insertItem(StationText, stn, SLOT(activate()));
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

void RadioDocking::slotUpdateToolTips ()
{
  QToolTip::add(this, radio->getStationString(true, true, true));
}


void RadioDocking::slotToggleUI ()
{
    if (parentWidget()->isMinimized())
        parentWidget()->showNormal();
    else
        parentWidget()->showMinimized();
}
