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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "utils.h"

#include <kpopupmenu.h>
#include <ksystemtray.h>
#include "quickbar.h"
#include "timecontrol.h"

class KRadio;
// *REALLY DIRTY HACK* for development only.
//#define KRadio KRadioMW
//class KRadioMW;

class RadioDocking : public KSystemTray {
	Q_OBJECT
public:
	RadioDocking (KRadio *widget, QuickBar *qb, RadioBase *radio, TimeControl *tc, const char *name = 0);
	virtual ~RadioDocking();

	virtual void showEvent (QShowEvent *);

private slots:

public slots:
    virtual void slotSearchPrevStation(void);
    virtual void slotSearchNextStation(void);
    virtual void slotNOP();
    virtual void slotToggleUI();
    virtual void slotToggleQB();

    virtual void slotUpdateToolTips ();
    virtual void slotAlarm ();
    virtual void slotConfigChanged();
    virtual void slotSleepChanged();

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
	int			sleepID;
	IntVector   StationIDs;

	RadioBase 	*radio;
	QWidget		*radioControl;
	QuickBar    *quickbar;
	TimeControl *timeControl;

    QPixmap     miniKRadioPixmap;

    // configuration
    bool leftMouseTogglesQB;
    bool leftMouseTogglesUI;
};

#endif
