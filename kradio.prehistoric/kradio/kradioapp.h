/***************************************************************************
                          kradioapp.h  -  description
                             -------------------
    begin                : Sa Feb  9 2002
    copyright            : (C) 2002 by Klas Kalass / Martin Witte / Frank Schwanz
    email                : klas.kalass@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



#ifndef KRADIOAPP_H
#define KRADIOAPP_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapplication.h>
#include <qstring.h>
#include <kaboutapplication.h>

#include "quickbar.h" // HACK!!

#include "v4lradio.h"
#include "timecontrol.h"
#include "lircsupport.h"
#include "setupdialog.h"

class RadioDocking;

class KRadio;
// *REALLY DIRTY HACK* for development only.
//#define KRadio KRadioMW
//class KRadioMW;




class KRadioApp : public KApplication
{
Q_OBJECT
public:
    KRadioApp();
    virtual ~KRadioApp();

public slots:

    virtual void slotRunConfigure();
    virtual void slotApplyConfig ();

    // interface connection slot

    virtual void    connectPlugin(QObjectList &otherPlugins);

    // configuration slots

	virtual void    restoreState (KConfig *c);
	virtual void    saveState    (KConfig *c);
    virtual void    configurationChanged (const SetupData &sud);

	// radio interface notification slots

    virtual void    alarm(const Alarm *);

signals:

	// radio interface notification signals
	// should be transferred to setupdialog
	
	void sigConfigurationChanged(const SetupData &d);
    void sigSaveState           (KConfig *config);
    void sigRestoreState        (KConfig *config);

private:
    void restoreState();
    void saveState();

    void readConfiguration();
    void saveConfiguration();

    void addPlugin(QObject *o);

    KAboutApplication AboutApplication;

    KConfig       *config;

/*
    KRadio        *kradio;
    RadioDocking  *tray;
    QuickBar      *quickbar;
    TimeControl   *timeControl;
    V4LRadio      *radio;

#ifdef HAVE_LIRC_CLIENT
    LircSupport   *lircHelper;
#endif
*/

    SetupDialog    setupDialog;
    SetupData	   setupData;
    QObjectList    plugins;

};


void readXMLCfg (const QString &url,
                 StationVector &sl,
                 StationListMetaData &info,
                 AlarmVector &al
                );

void writeXMLCfg (const QString &FileName,
                  const StationVector &sl,
                  const StationListMetaData &info
                 );

QString writeXMLCfg (const StationVector &sl,
                     const StationListMetaData &info
                    );

#endif
