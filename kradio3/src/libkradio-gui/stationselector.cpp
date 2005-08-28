/***************************************************************************
                          stationselector.cpp  -  description
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

using namespace std;

#include <kpushbutton.h>
#include <algorithm>

#include "../libkradio/stationlist.h"
#include "../radio-stations/radiostation.h"

#include "stationselector.h"
#include "radiostation-listview.h"

StationSelector::StationSelector (QWidget *parent)
    : StationSelectorUI(parent)
{
    QObject::connect(buttonToLeft,  SIGNAL(clicked()), this, SLOT(slotButtonToLeft()));
    QObject::connect(buttonToRight, SIGNAL(clicked()), this, SLOT(slotButtonToRight()));
    QObject::connect(listAvailable, SIGNAL(sigStationsReceived(const QStringList&)), this, SLOT(slotMoveToLeft(const QStringList&)));
    QObject::connect(listSelected,  SIGNAL(sigStationsReceived(const QStringList&)), this, SLOT(slotMoveToRight(const QStringList&)));

    listSelected->setSelectionMode(QListView::Extended);
    listAvailable->setSelectionMode(QListView::Extended);
}


StationSelector::~StationSelector ()
{
}


bool StationSelector::connectI(Interface *i)
{
    bool a = IStationSelectionClient::connectI(i);
    bool b = IRadioClient::connectI(i);
    return a || b;
}


bool StationSelector::disconnectI(Interface *i)
{
    bool a = IStationSelectionClient::disconnectI(i);
    bool b = IRadioClient::disconnectI(i);
    return a || b;
}


bool StationSelector::noticeStationSelectionChanged(const QStringList &sl)
{
    m_stationIDsNotDisplayed.clear();
    m_stationIDsSelected.clear();
    m_stationIDsAvailable.clear();

    for (unsigned int i = 0; i < m_stationIDsAll.count(); ++i) {
        if (sl.contains(m_stationIDsAll[i])) {
            m_stationIDsSelected.append(m_stationIDsAll[i]);
        } else {
            m_stationIDsAvailable.append(m_stationIDsAll[i]);
        }
    }
    for (unsigned int i = 0; i < sl.count(); ++i) {
        if (!m_stationIDsAll.contains(sl[i]))
            m_stationIDsNotDisplayed.append(sl[i]);
    }
    updateListViews();
    return true;
}


bool StationSelector::noticeStationsChanged(const StationList &sl)
{
    listAvailable->clear();
    listSelected->clear();

    m_stationIDsAvailable.clear();
    m_stationIDsAll.clear();

    for (unsigned int i = 0; i < m_stationIDsSelected.count(); ++i)
        m_stationIDsNotDisplayed.append(m_stationIDsSelected[i]);

    m_stationIDsSelected.clear();

    for (RawStationList::Iterator i(sl.all()); i.current(); ++i) {
        const QString &id = i.current()->stationID();

        m_stationIDsAll.append(id);
        if (m_stationIDsNotDisplayed.contains(id)) {
            m_stationIDsNotDisplayed.remove(id);
            m_stationIDsSelected.append(id);
        } else {
            m_stationIDsAvailable.append(id);
        }
    }

    updateListViews();
    return true;
}


void StationSelector::slotButtonToLeft()
{
    listAvailable->clearSelection();
    QListViewItem *item = listSelected->firstChild();
    int idx_from = 0;
    while (item) {
        QListViewItem *next_item = item->nextSibling();

        if (item->isSelected()) {

            moveItem (listSelected,  m_stationIDsSelected,
                      item,          idx_from,
                      listAvailable, m_stationIDsAvailable);

            --idx_from;
        }
        item = next_item;
        ++idx_from;
    }
}


void StationSelector::slotButtonToRight()
{
    listSelected->clearSelection();
    QListViewItem *item = listAvailable->firstChild();
    int idx_from = 0;
    while (item) {
        QListViewItem *next_item = item->nextSibling();

        if (item->isSelected()) {

            moveItem (listAvailable, m_stationIDsAvailable,
                      item,          idx_from,
                      listSelected,  m_stationIDsSelected);

            --idx_from;
        }
        item = next_item;
        ++idx_from;
    }
}


void StationSelector::slotMoveToRight(const QStringList &list)
{
    listSelected->clearSelection();
    QListViewItem *item = listAvailable->firstChild();
    int idx_from = 0;
    while (item) {
        QListViewItem *next_item = item->nextSibling();

        if (list.contains(m_stationIDsAvailable[idx_from])) {

            moveItem (listAvailable, m_stationIDsAvailable,
                      item,          idx_from,
                      listSelected,  m_stationIDsSelected);

            --idx_from;
        }
        item = next_item;
        ++idx_from;
    }
}


void StationSelector::slotMoveToLeft(const QStringList &list)
{
    listAvailable->clearSelection();
    QListViewItem *item = listSelected->firstChild();
    int idx_from = 0;
    while (item) {
        QListViewItem *next_item = item->nextSibling();

        if (list.contains(m_stationIDsSelected[idx_from])) {

            moveItem (listSelected,  m_stationIDsSelected,
                      item,          idx_from,
                      listAvailable, m_stationIDsAvailable);

            --idx_from;
        }
        item = next_item;
        ++idx_from;
    }
}


void StationSelector::moveItem(
  RadioStationListView *fromListView,
  QStringList          &fromIDList,
  QListViewItem        *item,
  int                   idx_from,
  RadioStationListView *toListView,
  QStringList          &toIDList
)
{
    fromListView->takeItem(item, idx_from);

    QString id = fromIDList[idx_from];
    fromIDList.remove(fromIDList.at(idx_from));

    unsigned int  idx_to  = 0,
                  idx_all = 0;
    bool found = false;
    QListViewItem *item_to      = toListView->firstChild(),
                  *prev_item_to = NULL;

    while (idx_all < m_stationIDsAll.count() &&
           idx_to  < toIDList.count())
    {
        while (m_stationIDsAll[idx_all] != toIDList[idx_to])
        {
            if (m_stationIDsAll[idx_all] == id) {
                found = true;
                break;
            }
            ++idx_all;
        }
        if (found)
            break;

        prev_item_to = item_to;
        item_to = item_to->nextSibling();
        ++idx_to;
    }

    toIDList.insert(toIDList.at(idx_to), id);
    toListView->insertItem(item, id, idx_to);
    if (prev_item_to) {
        item->moveItem(prev_item_to);
    } else {
        item->moveItem(item_to);
        if (item_to) item_to->moveItem(item);
    }
}


void StationSelector::updateListViews()
{
    listAvailable->clear();
    listSelected->clear();
    const StationList &stations = queryStations();
    const RawStationList    &sl = stations.all();

    for (unsigned int i = 0; i < m_stationIDsAvailable.count(); ++i) {
        QString id = m_stationIDsAvailable[i];
        listAvailable->appendStation(sl.stationWithID(id), sl.idxWithID(id)+1);
    }
    for (unsigned int i = 0; i < m_stationIDsSelected.count(); ++i) {
        QString id = m_stationIDsSelected[i];
        listSelected->appendStation(sl.stationWithID(id), sl.idxWithID(id)+1);
    }
}


void StationSelector::slotOK()
{
    QStringList l = m_stationIDsSelected;
    for (unsigned int i = 0; i < m_stationIDsNotDisplayed.count(); ++i)
        l.append(m_stationIDsNotDisplayed[i]);
    sendStationSelection(l);
}


void StationSelector::slotCancel()
{
    noticeStationSelectionChanged(queryStationSelection());
}


void StationSelector::saveState (KConfig *cfg) const
{
    listSelected->saveState(cfg);
    listAvailable->saveState(cfg);
}


void StationSelector::restoreState (KConfig *cfg)
{
    listSelected->restoreState(cfg);
    listAvailable->restoreState(cfg);
}



#include "stationselector.moc"
