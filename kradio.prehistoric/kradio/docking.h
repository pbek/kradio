/***************************************************************************
                          docking.h  -  description
                             -------------------
    begin                : Mon Jan 14 2002
    copyright            : (C) 2001, 2002 by Frank Schwanz, Ernst Martin Witte
    email                : schwanz@fh-brandenburg.de, witte@kawo1.rwth-aachen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DOCKING_H_
#define _DOCKING_H_

#include <kpopupmenu.h>
#include <ksystemtray.h>
#include "kradio.h"

class RadioDocking : public KSystemTray {
	Q_OBJECT
public:
	RadioDocking (class KRadio *widget, RadioBase *radio, const char *name = 0);
	virtual ~RadioDocking();

private slots:
    void toggleWindowState();
    void mousePressEvent(QMouseEvent *e);

public slots:
    void slotSearchPrevStation(void);
    void slotSearchNextStation(void);
//    void slotSelStation(int);
    void slotPowerToggle();
    void slotNOP();

private:
  RadioBase 	*radio;
  KRadio		*widget;

  KPopupMenu	*menu;
  int 			toggleID;
  int			stationSeparatorID;
  int 			nextID;
  int 			prevID;
  int			currID;
  int			powerID;
  int			alarmID;

  QPixmap 		miniKRadioPixmap;
};

#endif
