/***************************************************************************
                          radiostation.h  -  description
                             -------------------
    begin                : Sat Feb 2 2002
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

#ifndef RADIOSTATION_H
#define RADIOSTATION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "utils.h"

#include <qobject.h>
#include <vector>

/**
  *@author Martin Witte
  */

class RadioStation : public QObject  {
	Q_OBJECT
protected :	
	float	Frequency;
	QString	ShortName;
	bool	QuickSelect;
	bool    DockingMenu;

	float	VolumePreset;		// <0: => Don't use
	
	QString	iconString;

public:
	RadioStation(QObject *parent = NULL);
	RadioStation(QObject *parent, QString Name, QString ShortName, QString iconString,
				 float Frequency, float VolumePreset = -1);
	RadioStation(const RadioStation &);
	RadioStation(QObject *_parent, const RadioStation &);
	virtual ~RadioStation();
	
	bool    isValid() const;
	
	QString getShortName() const     { return ShortName; }
	QString getLongName() const;
	QString getIconString() const    { return iconString; }
	float	getFrequency() const     { return Frequency; }
	float	getVolumePreset() const  { return VolumePreset; }
	bool	useQuickSelect() const   { return QuickSelect; }
	bool    useInDockingMenu() const { return DockingMenu; }
	
	void	setQuickSelect(bool qs)  { QuickSelect = qs; }
	void 	setFrequency(float f)    { Frequency = f; }
	void	setShortName(QString n)  { ShortName = n; }
	void	setIconString(QString s) { iconString = s; }
	void	setVolumePreset(float v) { VolumePreset = v; }
	void    setUseInDockingMenu (bool b) { DockingMenu = b; }
	
public slots:
	void activate ();
	
signals:
	void activated (const RadioStation *);
};


typedef vector<RadioStation*>			StationVector;
typedef StationVector::iterator			iStationVector;
typedef StationVector::const_iterator	ciStationVector;


struct StationListMetaData
{
	QString    Maintainer;
	QDateTime  LastChange;
	QString    Country;
	QString    City;
	QString    Media;
	QString    Comment;
};


#endif
