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
#include "quickbar.h"

class KRadioApp;

class RadioDocking : public KSystemTray {
	Q_OBJECT
public:
	RadioDocking (KRadio *widget, QuickBar *qb, RadioBase *radio, const char *name = 0);
	virtual ~RadioDocking();

	virtual void showEvent (QShowEvent *);
	
private slots:

public slots:
    void slotSearchPrevStation(void);
    void slotSearchNextStation(void);
    void slotNOP();
    void slotToggleUI();
    void slotToggleQB();

    void slotUpdateToolTips ();
    void slotAlarm ();
    void slotConfigChanged();

signals:
    void showAbout();

protected:
    void mousePressEvent( QMouseEvent *e );

private:
	void buildContextMenu();
	void buildStationList();
	void contextMenuAboutToShow( KPopupMenu* menu );

	int			titleID;	
	int 		alarmID;
	int			powerID;
	int			guiID;
	int     	qbID;
	IntVector   StationIDs;

	RadioBase 	*radio;
	QWidget		*radioControl;
	QuickBar    *quickbar;

    QPixmap     miniKRadioPixmap;

    // configuration
    bool leftMouseTogglesQB;
    bool leftMouseTogglesUI;
};

#endif
