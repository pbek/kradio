/***************************************************************************
                          radiostation.cpp  -  description
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

#include "radiostation.h"

RadioStation::RadioStation(QObject *_parent)
	: QObject (_parent)
{
	Frequency = 0;
	ShortName = "";
	VolumePreset = -1;
	QuickSelect = false;
	iconString = "";
	DockingMenu = true;
}

RadioStation::RadioStation(QObject *_parent, QString _name, QString _ShortName,
						   QString _iconString,
                           float _Frequency, float _volumePreset)
 : QObject (_parent, _name)
{
	Frequency = _Frequency;
	ShortName = _ShortName;
	VolumePreset = _volumePreset;
	QuickSelect = false;
	iconString = _iconString;
	DockingMenu = true;
}


RadioStation::RadioStation(const RadioStation &s)
	: QObject (s.parent(), s.name())
{
	Frequency = s.Frequency;
	ShortName = s.ShortName;
	VolumePreset = s.VolumePreset;
	QuickSelect = s.QuickSelect;
	iconString = s.iconString;
	DockingMenu = s.DockingMenu;
}


RadioStation::RadioStation(QObject *_parent, const RadioStation &s)
	: QObject (_parent, s.name())
{
	Frequency = s.Frequency;
	ShortName = s.ShortName;
	VolumePreset = s.VolumePreset;
	QuickSelect = s.QuickSelect;
	iconString = s.iconString;
	DockingMenu = s.DockingMenu;
}


RadioStation::~RadioStation()
{
}

QString RadioStation::getLongName() const
{
	QString longName = name();
	if (!longName.isEmpty())
        longName += ", ";

    float   cf = getFrequency();
    QString f;
    if (cf >= 10)
		f = QString().setNum(cf, 'f', 2) + " MHz";
	else
		f = QString().setNum(cf * 1000, 'f', 0) + " kHz";
    
    longName = longName + f;
	return longName;
}

void RadioStation::activate()
{
	if (isValid()) {
		emit activated (this);
	}
}


bool RadioStation::isValid() const
{
	return Frequency > 0;
}
