/***************************************************************************
                          radiostation.h  -  description
                             -------------------
    begin                : Sat Feb 2 2002
    copyright            : (C) 2002 by Martin Witte / Frank Schwanz
                               2003 by Klas Kalass
    email                : witte@kawo1.rwth-aachen.de / schwanz@fh-brandenburg.de
                           / klas@kde.org
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

// forward declarations
class KURL;

class RadioDevice;
/**
  *@author Martin Witte, Klas Kalass
  */

class RadioStation : public QObject  {
Q_OBJECT
public:
	RadioStation(QObject *parent, QString name);
	RadioStation(const RadioStation &);
	RadioStation(QObject *_parent, const RadioStation &);
	virtual ~RadioStation();

    // several settings
	bool	useInQuickSelect() const { return m_useInQuickSelect; }
	void	setUseInQuickSelect(bool useInQuickSelect)  { m_useInQuickSelect = useInQuickSelect; }

        virtual QString	longName() const;

	QString	shortName() const  { return m_shortName; }
	void	setShortName(QString shortName)  { m_shortName = shortName; }

	QString	iconName() { return m_iconName; }
	void	setIconName(QString iconName) { m_iconName = iconName; }

	float	initialVolume() const { return m_initialVolume; }
	void	setInitialVolume(float initialVolume) { m_initialVolume = initialVolume; }

	bool    useInDockingMenu () const { return m_useInDockingMenu; }
	void    setUseInDockingMenu (bool useInDockingMenu) { m_useInDockingMenu = useInDockingMenu; }

    // station functionality
    virtual RadioDevice *radio() = 0;
    virtual bool urlMatch(KURL const &url) {return false;};
    virtual bool frequencyMatch(float frequency) {return false;};
    virtual bool isValid() const = 0;

    /** returns an exact copy of this station, but the parent is the one given */
    virtual RadioStation *copy(QObject *parent) = 0;

public slots:
    // activate this station
	virtual void slotActivate () = 0;

signals:
    // this signal is emitted when this station has been sucessfully activated
	void signalActivated (const RadioStation *);

protected :
	float	m_frequency;
	QString	m_shortName;
	bool	m_useInQuickSelect;
	bool    m_useInDockingMenu;

	float	m_initialVolume;		// <0: => Don't use

	QString	m_iconName;
};

#endif
