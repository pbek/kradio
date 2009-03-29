/***************************************************************************
                          stationselector.cpp  -  description
                             -------------------
    begin                : Son Aug 3 2003
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

using namespace std;

#include <kpushbutton.h>
#include <algorithm>

#include "stationlist.h"
#include "radiostation.h"

#include "stationselector.h"
#include "radiostation-listview.h"

#include <ui_stationselector-ui.h>

StationSelector::StationSelector (QWidget *parent)
    : QWidget(parent),
      m_dirty(true)
{
    m_ui = new Ui_StationSelectorUI();
    m_ui->setupUi(this);

    m_ui->buttonToRight->setIcon(KIcon("arrow-right"));
    m_ui->buttonToLeft->setIcon(KIcon("arrow-left"));

    QObject::connect(m_ui->buttonToLeft,  SIGNAL(clicked()), this, SLOT(slotButtonToLeft()));
    QObject::connect(m_ui->buttonToRight, SIGNAL(clicked()), this, SLOT(slotButtonToRight()));
    QObject::connect(m_ui->listAvailable, SIGNAL(sigStationsReceived(const QStringList&)), this, SLOT(slotMoveToLeft(const QStringList&)));
    QObject::connect(m_ui->listSelected,  SIGNAL(sigStationsReceived(const QStringList&)), this, SLOT(slotMoveToRight(const QStringList&)));

    m_ui->listSelected->setSelectionMode(Q3ListView::Extended);
    m_ui->listAvailable->setSelectionMode(Q3ListView::Extended);
}


StationSelector::~StationSelector ()
{
    delete m_ui;
    m_ui = NULL;
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

    for (int i = 0; i < m_stationIDsAll.count(); ++i) {
        if (sl.contains(m_stationIDsAll[i])) {
            m_stationIDsSelected.append(m_stationIDsAll[i]);
        } else {
            m_stationIDsAvailable.append(m_stationIDsAll[i]);
        }
    }
    for (int i = 0; i < sl.count(); ++i) {
        if (!m_stationIDsAll.contains(sl[i]))
            m_stationIDsNotDisplayed.append(sl[i]);
    }
    updateListViews();
    m_dirty = false;
    return true;
}


bool StationSelector::noticeStationsChanged(const StationList &sl)
{
    slotSetDirty();

    m_ui->listAvailable->clear();
    m_ui->listSelected->clear();

    m_stationIDsAvailable.clear();
    m_stationIDsAll.clear();

    for (int i = 0; i < m_stationIDsSelected.count(); ++i)
        m_stationIDsNotDisplayed.append(m_stationIDsSelected[i]);

    m_stationIDsSelected.clear();

    for (StationList::const_iterator i = sl.begin(); i != sl.end(); ++i) {
        const QString &id = (*i)->stationID();

        m_stationIDsAll.append(id);
        if (m_stationIDsNotDisplayed.contains(id)) {
            m_stationIDsNotDisplayed.removeAll(id);
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
    slotSetDirty();
    m_ui->listAvailable->clearSelection();
    Q3ListViewItem *item = m_ui->listSelected->firstChild();
    int idx_from = 0;
    while (item) {
        Q3ListViewItem *next_item = item->nextSibling();

        if (item->isSelected()) {

            moveItem (m_ui->listSelected,  m_stationIDsSelected,
                      item,                idx_from,
                      m_ui->listAvailable, m_stationIDsAvailable);

            --idx_from;
        }
        item = next_item;
        ++idx_from;
    }
}


void StationSelector::slotButtonToRight()
{
    slotSetDirty();
    m_ui->listSelected->clearSelection();
    Q3ListViewItem *item = m_ui->listAvailable->firstChild();
    int idx_from = 0;
    while (item) {
        Q3ListViewItem *next_item = item->nextSibling();

        if (item->isSelected()) {

            moveItem (m_ui->listAvailable, m_stationIDsAvailable,
                      item,                idx_from,
                      m_ui->listSelected,  m_stationIDsSelected);

            --idx_from;
        }
        item = next_item;
        ++idx_from;
    }
}


void StationSelector::slotMoveToRight(const QStringList &list)
{
    slotSetDirty();
    m_ui->listSelected->clearSelection();
    Q3ListViewItem *item = m_ui->listAvailable->firstChild();
    int idx_from = 0;
    while (item) {
        Q3ListViewItem *next_item = item->nextSibling();

        if (list.contains(m_stationIDsAvailable[idx_from])) {

            moveItem (m_ui->listAvailable, m_stationIDsAvailable,
                      item,                idx_from,
                      m_ui->listSelected,  m_stationIDsSelected);

            --idx_from;
        }
        item = next_item;
        ++idx_from;
    }
}


void StationSelector::slotMoveToLeft(const QStringList &list)
{
    slotSetDirty();
    m_ui->listAvailable->clearSelection();
    Q3ListViewItem *item = m_ui->listSelected->firstChild();
    int idx_from = 0;
    while (item) {
        Q3ListViewItem *next_item = item->nextSibling();

        if (list.contains(m_stationIDsSelected[idx_from])) {

            moveItem (m_ui->listSelected,  m_stationIDsSelected,
                      item,                idx_from,
                      m_ui->listAvailable, m_stationIDsAvailable);

            --idx_from;
        }
        item = next_item;
        ++idx_from;
    }
}


QGridLayout   *StationSelector::getGridLayout()
{
    return m_ui ? m_ui->StationSelectorUILayout : NULL;
}

void StationSelector::moveItem(
  RadioStationListView *fromListView,
  QStringList          &fromIDList,
  Q3ListViewItem       *item,
  int                   idx_from,
  RadioStationListView *toListView,
  QStringList          &toIDList
)
{
    fromListView->takeItem(item, idx_from);

    QString id = fromIDList[idx_from];
    fromIDList.removeAt(idx_from);

    int             idx_to  = 0,
                    idx_all = 0;
    bool            found   = false;
    Q3ListViewItem *item_to      = toListView->firstChild(),
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

    toIDList.insert(idx_to, id);
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
    m_ui->listAvailable->clear();
    m_ui->listSelected->clear();
    const StationList &stations = queryStations();

    for (int i = 0; i < m_stationIDsAvailable.count(); ++i) {
        QString id = m_stationIDsAvailable[i];
        m_ui->listAvailable->appendStation(stations.stationWithID(id), stations.idxWithID(id)+1);
    }
    for (int i = 0; i < m_stationIDsSelected.count(); ++i) {
        QString id = m_stationIDsSelected[i];
        m_ui->listSelected->appendStation(stations.stationWithID(id), stations.idxWithID(id)+1);
    }
}


void StationSelector::slotOK()
{
    if (m_dirty) {
        QStringList l = m_stationIDsSelected;
        for (int i = 0; i < m_stationIDsNotDisplayed.count(); ++i)
            l.append(m_stationIDsNotDisplayed[i]);
        sendStationSelection(l);
    }
    m_dirty = false;
}


void StationSelector::slotCancel()
{
    if (m_dirty) {
        noticeStationSelectionChanged(queryStationSelection());
    }
    m_dirty = false;
}


void StationSelector::saveState (KConfigGroup &cfg) const
{
    m_ui->listSelected->saveState(cfg);
    m_ui->listAvailable->saveState(cfg);
}


void StationSelector::restoreState (KConfigGroup &cfg)
{
    m_ui->listSelected->restoreState(cfg);
    m_ui->listAvailable->restoreState(cfg);
}

void StationSelector::slotSetDirty()
{
    if (!m_dirty) {
        m_dirty = true;
        emit sigDirty();
    }
}

#include "stationselector.moc"
