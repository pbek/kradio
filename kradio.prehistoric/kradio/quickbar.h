/***************************************************************************
                          quickbar.h  -  description
                             -------------------
    begin                : Mon Feb 11 2002
    copyright            : (C) 2002 by Martin Witte / Frank Schwanz / Klas Kalass
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "utils.h"

#include <qwidget.h>
#include <qlayout.h>
#include <qbuttongroup.h>

#include <kconfig.h>
#include <ktoolbarbutton.h>

#include <list>
#include "radiobase.h"
#include "buttonflowlayout.h"

class RadioStation;

/**
  *@author Martin Witte / Frank Schwanz / Klas Kalass
  */


typedef list<QToolButton *>         ButtonList;
typedef ButtonList::iterator        iButtonList;
typedef ButtonList::const_iterator  ciButtonList;

class QuickBar : public QWidget  {
   Q_OBJECT
protected :

    RadioBase        *radio;
    ButtonList       Buttons;
    ButtonFlowLayout *layout;
    QButtonGroup     *buttonGroup;
    // config
    bool          showShortName;
public:
	QuickBar(RadioBase *radio, QWidget * parent = 0, const char * name = 0);
	~QuickBar();
	
	void    restoreState (KConfig *c);
	void    saveState (KConfig *c);
    void	getState();

    bool	getShowShortName() const { return showShortName; }
    void    setShowShortName(bool b);
	
protected:
	void resizeEvent(QResizeEvent *);

private:
	void rebuildGUI();

public slots:
    void setOn(bool on);
	void slotConfigChanged();
	void slotFrequencyChanged(float, const RadioStation *);
	void show();
	void hide();
	void setGeometry (const QRect &r);
	void setGeometry (int x, int y, int w, int h);

signals:
	void toggled(bool);


private:

    bool		saveSticky;
    int		    saveDesktop;
    QRect		saveGeometry;
};
#endif
