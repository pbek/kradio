/***************************************************************************
                          stationselection_interfaces.cpp  -  description
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

#include "stationselection_interfaces.h"
#include <qstringlist.h>

IF_IMPL_SENDER  (   IStationSelection::notifyStationSelectionChanged(const QStringList &sl),
                    noticeStationSelectionChanged(sl)
                )

IF_IMPL_SENDER  (   IStationSelectionClient::sendStationSelection(const QStringList &sl),
                    setStationSelection(sl)
                )

static QStringList emptyList;

IF_IMPL_QUERY   (   const QStringList & IStationSelectionClient::queryStationSelection(),
                    getStationSelection(),
                    emptyList
                )

                
void IStationSelectionClient::noticeConnected    (cmplInterface *, bool /*pointer_valid*/)
{
	noticeStationSelectionChanged(queryStationSelection());
}


void IStationSelectionClient::noticeDisconnected (cmplInterface *, bool /*pointer_valid*/)
{
	noticeStationSelectionChanged(queryStationSelection());
}

