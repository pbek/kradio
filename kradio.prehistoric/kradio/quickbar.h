/***************************************************************************
                          quickbar.h  -  description
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

#ifndef QUICKBAR_H
#define QUICKBAR_H

#include <qwidget.h>
#include <kconfig.h>
#include <kpushbutton.h>
#include <list>
#include "radiobase.h"

/**
  *@author Martin Witte / Frank Schwanz
  */


typedef list<KPushButton *>         ButtonList;
typedef ButtonList::iterator        iButtonList;
typedef ButtonList::const_iterator  ciButtonList;

class QuickBar : public QWidget  {
   Q_OBJECT
protected :

    RadioBase   *radio;
    ButtonList  Buttons;

public:
	QuickBar(RadioBase *radio);	
	~QuickBar();
	
	void    restoreState (KConfig *c);
	void    saveState (KConfig *c) const;
	
protected:
    void resizeEvent(QResizeEvent *);

protected slots:

	void slotConfigChanged();
};

#endif
