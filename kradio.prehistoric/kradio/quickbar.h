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

class RadioStation;
class ButtonFlowLayout;
class QButtonGroup;
class KConfig;

/**
  *@author Martin Witte / Frank Schwanz / Klas Kalass
  */

class QuickBar : public QWidget  {
Q_OBJECT
protected :

    ButtonFlowLayout *layout;
    QButtonGroup     *buttonGroup;

    // config
    StationVector    stations;
    bool             showShortName;
    float            currentFrequency;
    
    // temporary data

    bool		saveSticky;
    int		    saveDesktop;
    QRect		saveGeometry;
    
public:
	QuickBar(QWidget * parent = 0, const char * name = 0);
	~QuickBar();

public slots:
	
    void    setOn(bool on);
	void    show();
	void    hide();
	void    setGeometry (const QRect &r);
	void    setGeometry (int x, int y, int w, int h);

    // interface connection slot

    virtual void    connectInterface(QObjectList &ol);

	// radio interface notification slots
	
	virtual void    frequencyChanged(float, const RadioStation *);
	
    // configuration slots

	virtual void    restoreState (KConfig *c);
	virtual void    saveState    (KConfig *c);
    virtual void    configurationChanged (const SetupData &sud);


protected slots:

	// this slot should be connected to each stations activated signal
	void    stationActivated (const RadioStation *);

protected:
    void	getState();
	void    rebuildGUI();
	void    resizeEvent(QResizeEvent *);

	int		getButtonID(float freq);

signals:
	void    toggled(bool);

	// radio interfaces command signals

	void    sigSetFrequency (float f);
};
#endif
