/***************************************************************************
                          radiostation-listview.h  -  description
                             -------------------
    begin                : Mi Mar 03 2004
    copyright            : (C) 2003 by Martin Witte
    email                : emw-kradio@nocabal.de
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

#ifdef KRADIO_ENABLE_FIXMES
    #warning "FIXME: konvert from k3listview to KListWidget"
#endif
#include <k3listview.h>

class RadioStation;
class StationList;

class KDE_EXPORT RadioStationListView : public K3ListView
{
Q_OBJECT
public:
    RadioStationListView(QWidget *parent=0, const char *name=0);
    virtual ~RadioStationListView();

    Q3ListViewItem *getItemForIndex(int idx) const;
    int             getIndexForItem(Q3ListViewItem *) const;

    void setStation(int idx, const RadioStation &, int nr = -1);
    void appendStation(const RadioStation &, int nr = -1);
    void setStations(const StationList &);

    void removeStation(int idx);
    void takeItem(Q3ListViewItem *item, int idx);
    void takeItem(Q3ListViewItem *item) { K3ListView::takeItem(item); } // just that compiler is quiet... we won't need it otherwise
    void insertItem(Q3ListViewItem *item, const QString &stationid, int idx);
    void insertItem(Q3ListViewItem *item) { K3ListView::insertItem(item); } // just that compiler is quiet... we won't need it otherwise

    void setCurrentStation(int idx);
    int  currentStationIndex() const;

    int count() const { return childCount(); }

    void   saveState    (KConfigGroup &) const;
    void   restoreState (const KConfigGroup &);


protected:

    Q3DragObject *dragObject();
    void          dragEnterEvent(QDragEnterEvent* event);
    void          dropEvent(QDropEvent* event);
    void          contentsDragEnterEvent(QDragEnterEvent* event);
    void          contentsDragMoveEvent(QDragMoveEvent* event);
    void          contentsDropEvent(QDropEvent* event);

protected slots:

    void slotStationActivation(Q3ListViewItem *);
    void slotCurrentStationChanged(Q3ListViewItem *);

signals:
    void sigStationActivated(int idx);
    void sigCurrentStationChanged(int idx);
    void sigStationsReceived(const QStringList &stationIDs);

protected:

    QStringList  m_StationIDs;

};

#endif
