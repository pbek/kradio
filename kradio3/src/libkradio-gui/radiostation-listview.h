/***************************************************************************
                          radiostation-listview.h  -  description
                             -------------------
    begin                : Mi Mar 03 2004
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef _KRADIO_RADIOSTATION_LISTVIEW_H_
#define _KRADIO_RADIOSTATION_LISTVIEW_H_

#include <klistview.h>

class RadioStation;
class StationList;

class RadioStationListView : public KListView
{
Q_OBJECT
public:
    RadioStationListView(QWidget *parent=0, const char *name=0);
    virtual ~RadioStationListView();

    QListViewItem *getItemForIndex(int idx) const;
    int            getIndexForItem(QListViewItem *) const;

    void setStation(int idx, const RadioStation &, int nr = -1);
    void appendStation(const RadioStation &, int nr = -1);
    void setStations(const StationList &);

    void removeStation(int idx);
    void takeItem(QListViewItem *item, int idx);
    void insertItem(QListViewItem *item, const QString &stationid, int idx);

    void setCurrentStation(int idx);
    int  currentStationIndex() const;

    int count() const { return childCount(); }

    void   saveState    (KConfig *) const;
    void   restoreState (KConfig *);


protected:

    QDragObject *dragObject();
    void         dragEnterEvent(QDragEnterEvent* event);
    void         dropEvent(QDropEvent* event);
    void         contentsDragEnterEvent(QDragEnterEvent* event);
    void         contentsDragMoveEvent(QDragMoveEvent* event);
    void         contentsDropEvent(QDropEvent* event);

protected slots:

    void slotStationActivation(QListViewItem *);
    void slotCurrentStationChanged(QListViewItem *);

signals:
    void sigStationActivated(int idx);
    void sigCurrentStationChanged(int idx);
    void sigStationsReceived(const QStringList &stationIDs);

protected:

    QStringList  m_StationIDs;

};

#endif
