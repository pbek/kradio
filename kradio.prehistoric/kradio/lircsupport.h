/***************************************************************************
                          lircsupport.h  -  description
                             -------------------
    begin                : Mon Feb 4 2002
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

#ifndef LIRCSUPPORT_H
#define LIRCSUPPORT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "utils.h"

#include <qobject.h>
#include <qsocketnotifier.h>

#ifdef HAVE_LIRC_CLIENT
#include <lirc/lirc_client.h>
#endif


class RadioBase;

class LircSupport : public QObject  {
Q_OBJECT
protected:
#ifdef HAVE_LIRC_CLIENT
	QSocketNotifier		*lirc_notify;
	int					fd_lirc;	
	struct lirc_config	*lircConfig;
#endif	
	QTimer				*kbdTimer;
	int					addIndex;
	
	RadioBase			*radio;
	
public:
	LircSupport(RadioBase *parent);
	~LircSupport();

protected:
	void	activateStation(int i);
		
protected slots:
	void slotLIRC(int socket);
	void slotKbdTimedOut();
};



#endif
