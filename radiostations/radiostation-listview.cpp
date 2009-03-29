/***************************************************************************
                          radiostation-listview.cpp  -  description
                             -------------------
    begin                : Mi Feb 3 2004
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

#include "radiostation-listview.h"
#include "stationlist.h"
#include "radiostation.h"
#include "station-drag-object.h"

#include <klocale.h>
#include <QtCore/QFile>
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QtGui/QDragMoveEvent>

#include <kconfiggroup.h>

RadioStationListView::RadioStationListView(QWidget *parent, const char *name)
  : K3ListView(parent)
{
    K3ListView::setObjectName(name);
    addColumn(i18n("No."));
    addColumn(i18n("Icon"));
    addColumn(i18n("Station"));
    addColumn(i18n("Description"));
    setAllColumnsShowFocus(true);
    setSorting(-1);

    QObject::connect(this, SIGNAL(spacePressed(Q3ListViewItem*)),
                     this, SLOT(slotStationActivation(Q3ListViewItem* )));
    QObject::connect(this, SIGNAL(returnPressed(Q3ListViewItem*)),
                     this, SLOT(slotStationActivation(Q3ListViewItem* )));
    QObject::connect(this, SIGNAL(doubleClicked(Q3ListViewItem*)),
                     this, SLOT(slotStationActivation(Q3ListViewItem *)));
    QObject::connect(this, SIGNAL(currentChanged(Q3ListViewItem*)),
                     this, SLOT(slotCurrentStationChanged(Q3ListViewItem *)));

    setAcceptDrops(true);
}


RadioStationListView::~RadioStationListView()
{
}


Q3ListViewItem *RadioStationListView::getItemForIndex(int idx) const
{
    Q3ListViewItem *item = NULL;

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


int RadioStationListView::getIndexForItem(Q3ListViewItem *queryItem) const
{
    int idx = -1;

    if (queryItem) {
        Q3ListViewItem *item = firstChild();
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
    Q3ListViewItem *item = getItemForIndex(idx);

    if (idx < 0) {
        item = new Q3ListViewItem(this, firstChild());
        firstChild()->moveItem(item);
        m_StationIDs.prepend(s.stationID());
        idx = 0;
    } else if (idx >= childCount()) {
        item = new Q3ListViewItem(this, lastChild());
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
            item->setPixmap(1, QPixmap::fromImage(img.scaled((int)(img.width()*f), (int)(h * f))));
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
    
    for (StationList::const_iterator it = stations.begin(); it != stations.end(); ++it) {
        setStation(childCount(), **it);
    }
}


void RadioStationListView::removeStation(int idx)
{
    Q3ListViewItem *item = getItemForIndex(idx);
    if (item) {
        delete item;
        m_StationIDs.erase(m_StationIDs.begin() + idx);
    }
}

void RadioStationListView::takeItem(Q3ListViewItem *item, int idx)
{
    Q3ListView::takeItem(item);
    m_StationIDs.erase(m_StationIDs.begin() + idx);
}

void RadioStationListView::insertItem(Q3ListViewItem *item, const QString &stationid, int idx_to)
{
    Q3ListView::insertItem(item);
    m_StationIDs.insert(idx_to, stationid);
}

void RadioStationListView::setCurrentStation(int idx)
{
    Q3ListViewItem *item = getItemForIndex(idx);
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


void RadioStationListView::slotStationActivation(Q3ListViewItem *item)
{
    emit sigStationActivated(getIndexForItem(item));
}


void RadioStationListView::slotCurrentStationChanged(Q3ListViewItem *item)
{
    emit sigCurrentStationChanged(getIndexForItem(item));
}


void RadioStationListView::saveState (KConfigGroup &cfg) const
{
    for (int i = 0; i < 4; ++i)
        cfg.writeEntry(QString(objectName()) + "_radiostation_listview_col_" + QString::number(i), columnWidth(i));
}


void RadioStationListView::restoreState (const KConfigGroup &cfg)
{
    for (int i = 0; i < 4; ++i)
        setColumnWidth(i, cfg.readEntry(QString(objectName()) + "_radiostation_listview_col_" + QString::number(i), -1));
}


Q3DragObject *RadioStationListView::dragObject()
{
    QStringList list;
    Q3ListViewItem *item = firstChild();
    for (int idx = 0; item; ++idx, item = item->nextSibling()) {
        if (item->isSelected()) {
            list.append(m_StationIDs[idx]);
        }
    }
    return new StationDragObject(list, this);
}

void RadioStationListView::dragEnterEvent(QDragEnterEvent* event)
{
    bool a = StationDragObject::canDecode(event);
    if (a) {
        IErrorLogClient::staticLogDebug(i18n("dragEnterEvent accepted"));
        event->accept();
    }
    else {
        IErrorLogClient::staticLogDebug(i18n("dragEnterEvent rejected"));
        event->ignore();
    }
}

void RadioStationListView::contentsDragEnterEvent(QDragEnterEvent* event)
{
    bool a = StationDragObject::canDecode(event);
    if (a) {
        IErrorLogClient::staticLogDebug(i18n("contentsDragEnterEvent accepted"));
        event->accept();
    }
    else {
        IErrorLogClient::staticLogDebug(i18n("contentsDragEnterEvent rejected"));
        event->ignore();
    }
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
