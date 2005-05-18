/***************************************************************************
                          radioview-configuration.h  -  description
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

#ifndef KRADIO_RADIOVIEW_CONFIGURATION_H
#define KRADIO_RADIOVIEW_CONFIGURATION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif



#include <qtabwidget.h>

class RadioViewConfiguration : public QTabWidget
{
Q_OBJECT
public :
	RadioViewConfiguration(QWidget *parent = NULL);
	~RadioViewConfiguration();

	void addTab    (QWidget *child, const QString &label);
	void addTab    (QWidget *child, const QIconSet &iconset, const QString &label);
	void addTab    (QWidget *child, QTab *tab);
	void insertTab (QWidget *child, const QString &label, int index = -1);
	void insertTab (QWidget *child, const QIconSet &iconset, const QString &label, int index = -1);
	void insertTab (QWidget *child, QTab *tab, int index = -1);
	void removePage(QWidget *w);
	
public slots:

	void slotOK();
	void slotCancel();

signals:

	void sigOK();
	void sigCancel();
};



#endif
