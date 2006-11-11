/***************************************************************************
                          radiostation-listview.cpp  -  description
                             -------------------
    begin                : Mi Feb 3 2004
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

#include "radiostation-listview.h"
#include "../libkradio/stationlist.h"
#include "../radio-stations/radiostation.h"
#include "station-drag-object.h"

#include <klocale.h>
#include <qfile.h>
#include <qimage.h>
#include <qpixmap.h>

#include <kconfig.h>

RadioStationListView::RadioStationListView(QWidget *parent, const char *name)
  : KListView(parent, name)
{
    addColumn(i18n("No."));
    addColumn(i18n("Icon"));
    addColumn(i18n("Station"));
    addColumn(i18n("Description"));
    setAllColumnsShowFocus(true);
    setSorting(-1);

    QObject::connect(this, SIGNAL(spacePressed(QListViewItem*)),
                     this, SLOT(slotStationActivation(QListViewItem* )));
    QObject::connect(this, SIGNAL(returnPressed(QListViewItem*)),
                     this, SLOT(slotStationActivation(QListViewItem* )));
    QObject::connect(this, SIGNAL(doubleClicked(QListViewItem*)),
                     this, SLOT(slotStationActivation(QListViewItem *)));
    QObject::connect(this, SIGNAL(currentChanged(QListViewItem*)),
                     this, SLOT(slotCurrentStationChanged(QListViewItem *)));

    setAcceptDrops(true);
}


RadioStationListView::~RadioStationListView()
{
}


QListViewItem *RadioStationListView::getItemForIndex(int idx) const
{
    QListViewItem *item = NULL;

    if (idx >= 0 && idx < childCount()) {
        item = firstChild();
        int i = 0;
        while (item && i < idx) {
            item = item->nextSibling();
            ++i;
        }
    }
    return item;
}


int RadioStationListView::getIndexForItem(QListViewItem *queryItem) const
{
    int idx = -1;

    if (queryItem) {
        QListViewItem *item = firstChild();
        ++idx;
        while (item && item != queryItem) {
            item = item->nextSibling();
            ++idx;
        }
        if (!item)
            idx = -1;
    }

    return idx;
}


void RadioStationListView::setStation(int idx, const RadioStation &s, int nr)
{
    QListViewItem *item = getItemForIndex(idx);

    if (idx < 0) {
        item = new QListViewItem(this, firstChild());
        firstChild()->moveItem(item);
        m_StationIDs.prepend(s.stationID());
        idx = 0;
    } else if (idx >= childCount()) {
        item = new QListViewItem(this, lastChild());
        m_StationIDs.append(s.stationID());
        idx = childCount() - 1;
    }

    if (item) {
        item->setDragEnabled(true);
        item->setDropEnabled(true);

        item->setText(0, QString::number(nr > 0 ? nr : idx+1));
        item->setText(2, s.name());
        item->setText(3, s.description());

        m_StationIDs[idx] = s.stationID();

        QImage  img(s.iconName());
        if (!img.isNull()) {
            int   h = img.height();
            float f = 0.9 * (float)(item->height()) / (h ? (float)h : 1.0);
            item->setPixmap(1, img.smoothScale((int)(img.width()*f), (int)(h * f)));
        } else {
            item->setPixmap(1, QPixmap());
        }
    }
}


void RadioStationListView::appendStation(const RadioStation &st, int nr)
{
    setStation(childCount(), st, nr);
}


void RadioStationListView::setStations(const StationList &stations)
{
    clear();
    for (RawStationList::Iterator it(stations.all()); it.current(); ++it) {
        setStation(childCount(), *it.current());
    }
}


void RadioStationListView::removeStation(int idx)
{
    QListViewItem *item = getItemForIndex(idx);
    if (item) {
        delete item;
        m_StationIDs.remove(m_StationIDs.at(idx));
    }
}

void RadioStationListView::takeItem(QListViewItem *item, int idx)
{
    QListView::takeItem(item);
    m_StationIDs.remove(m_StationIDs.at(idx));
}

void RadioStationListView::insertItem(QListViewItem *item, const QString &stationid, int idx_to)
{
    QListView::insertItem(item);
    m_StationIDs.insert(m_StationIDs.at(idx_to), stationid);
}

void RadioStationListView::setCurrentStation(int idx)
{
    QListViewItem *item = getItemForIndex(idx);
    if (item) {
        clearSelection();
        setSelected(item, true);
        setCurrentItem(item);
    }
}


int RadioStationListView::currentStationIndex() const
{
    return getIndexForItem(currentItem());
}


void RadioStationListView::slotStationActivation(QListViewItem *item)
{
    emit sigStationActivated(getIndexForItem(item));
}


void RadioStationListView::slotCurrentStationChanged(QListViewItem *item)
{
    emit sigCurrentStationChanged(getIndexForItem(item));
}


void RadioStationListView::saveState (KConfig *cfg) const
{
    if (!cfg)
        return;
    for (int i = 0; i < 4; ++i)
        cfg->writeEntry(QString(name()) + "_radiostation_listview_col_" + QString::number(i), columnWidth(i));
}


void RadioStationListView::restoreState (KConfig *cfg)
{
    if (!cfg)
        return;
    for (int i = 0; i < 4; ++i)
        setColumnWidth(i, cfg->readNumEntry(QString(name()) + "_radiostation_listview_col_" + QString::number(i), -1));
}


QDragObject *RadioStationListView::dragObject()
{
    QStringList list;
    QListViewItem *item = firstChild();
    for (int idx = 0; item; ++idx, item = item->nextSibling()) {
        if (item->isSelected()) {
            list.append(m_StationIDs[idx]);
        }
    }
    return new StationDragObject(list, this);
}

void RadioStationListView::dragEnterEvent(QDragEnterEvent* event)
{
    event->accept(StationDragObject::canDecode(event));
}

void RadioStationListView::contentsDragEnterEvent(QDragEnterEvent* event)
{
    bool a = StationDragObject::canDecode(event);
    if (a)
        IErrorLogClient::staticLogDebug(i18n("contentsDragEnterEvent accepted"));
    else
        IErrorLogClient::staticLogDebug(i18n("contentsDragEnterEvent rejected"));
    event->accept(a);
}

void RadioStationListView::dropEvent(QDropEvent* event)
{
    QStringList list;

    if ( StationDragObject::decode(event, list) ) {
        emit sigStationsReceived(list);
    }
}

void RadioStationListView::contentsDropEvent(QDropEvent* event)
{
    dropEvent(event);
}

void RadioStationListView::contentsDragMoveEvent(QDragMoveEvent* event)
{
    event->accept();
}

#include "radiostation-listview.moc"
