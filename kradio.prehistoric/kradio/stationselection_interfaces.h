/***************************************************************************
                          stationselection_interfaces.h  -  description
                             -------------------
    begin                : Son Aug 3 2003
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

#ifndef KRADIO_STATIONSELECTION_INTERFACES_H
#define KRADIO_STATIONSELECTION_INTERFACES_H

#include "interfaces.h"

class QStringList;

INTERFACE(IStationSelection, IStationSelectionClient)
{
public :
	IF_CON_DESTRUCTOR(IStationSelection, -1)

RECEIVERS:
    IF_RECEIVER(    setStationSelection(const QStringList &sl)              )

SENDERS:
    IF_SENDER  (    notifyStationSelectionChanged(const QStringList &sl)    )

ANSWERS:
    IF_ANSWER  (    const QStringList & getStationSelection () const        )

};


INTERFACE(IStationSelectionClient, IStationSelection)
{
public :
	IF_CON_DESTRUCTOR(IStationSelectionClient, 1)

SENDERS:
    IF_SENDER  (    sendStationSelection(const QStringList &sl)             )

RECEIVERS:
    IF_RECEIVER(    noticeStationSelectionChanged(const QStringList &sl)    )

QUERIES:
    IF_QUERY   (    const QStringList & queryStationSelection ()            )


RECEIVERS:
	virtual void noticeConnected    (cmplInterface *, bool /*pointer_valid*/);
	virtual void noticeDisconnected (cmplInterface *, bool /*pointer_valid*/);
};


#endif
