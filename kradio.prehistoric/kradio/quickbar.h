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
#include "radiobase.h"

/**
  *@author Martin Witte / Frank Schwanz
  */

class QuickBar : public QWidget  {
   Q_OBJECT
protected :

    RadioBase   *radio;

public: 
	QuickBar(QWidget *parent, RadioBase *radio, const char *name=0);	
	~QuickBar();
	
	void    readConfig (KConfig *c);
	void    saveConfig (KConfig *c) const;
protected slots:

	void slotConfigChanged();
};

#endif
