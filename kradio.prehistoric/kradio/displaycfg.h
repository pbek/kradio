/***************************************************************************
                          displaycfg.h  -  description
                             -------------------
    begin                : Fr Aug 15 2003
    copyright            : (C) 2003 by Martin Witte
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

#ifndef KRADIO_DISPLAYCFG_H
#define KRADIO_DISPLAYCFG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "displaycfg_interfaces.h"
#include <qwidget.h>

class KColorButton;
class KFontChooser;

class DisplayConfiguration : public QWidget,
                             public IDisplayCfgClient
{
Q_OBJECT
public:
	DisplayConfiguration(QWidget *parent);
	~DisplayConfiguration();

// Interface

    bool connect (Interface *i) { return IDisplayCfgClient::connect(i); }
    bool disconnect (Interface *i)  { return IDisplayCfgClient::disconnect(i); }

// IDisplayCfgClient
    
RECEIVERS:
	bool noticeDisplayColorsChanged(const QColor &activeColor, const QColor &inactiveColor, const QColor &bkgnd);
	bool noticeDisplayFontChanged(const QFont &f);


public slots:

	void slotOK();
	void slotCancel();

protected:
	KColorButton *m_btnActive;
	KColorButton *m_btnInactive;
	KColorButton *m_btnBkgnd;
	KFontChooser *m_fontChooser;
};


#endif
