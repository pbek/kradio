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
#include <kicon.h>
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
    QObject::connect(m_ui->listAvailable->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                     this, SLOT(slotSetDirty()));
    QObject::connect(m_ui->listAvailable->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
                     this, SLOT(slotSetDirty()));
    QObject::connect(m_ui->listSelected->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                     this, SLOT(slotSetDirty()));
    QObject::connect(m_ui->listSelected->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
                     this, SLOT(slotSetDirty()));

    m_ui->listSelected->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_ui->listAvailable->setSelectionMode(QAbstractItemView::ExtendedSelection);
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
    moveSelectedRows(m_ui->listSelected, m_ui->listAvailable);
}


void StationSelector::slotButtonToRight()
{
    moveSelectedRows(m_ui->listAvailable, m_ui->listSelected);
}


QGridLayout   *StationSelector::getGridLayout()
{
    return m_ui ? m_ui->StationSelectorUILayout : NULL;
}


void StationSelector::moveSelectedRows(
  RadioStationListView *fromListView,
  RadioStationListView *toListView
)
{
    QSet<int> rowsSet;
    foreach (const QModelIndex &index, fromListView->selectionModel()->selectedRows()) {
        if (index.isValid()) {
            rowsSet << index.row();
        }
    }
    QList<int> rows = rowsSet.toList();
    // iterate the list of rows by last to first, otherwise
    // removing lower rows will invalidate following ones
    qSort(rows.begin(), rows.end(), qGreater<int>());
    const StationList &stations = queryStations();
    foreach (int row, rows) {
        const QModelIndex fromIndex = fromListView->model()->index(row, 0);
        const QString id = fromIndex.data(RadioStationModel::StationIdRole).toString();
        const RadioStation &rs = stations.stationWithID(id);
        const int nr = stations.idxWithID(id)+1;

        toListView->insertStationByNr(rs, nr);
        fromListView->removeStation(row);
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
        QStringList l;
        const int count = m_ui->listSelected->childCount();
        for (int i = 0; i < count; ++i) {
            QModelIndex mi = m_ui->listSelected->model()->index(i, 0);
            l.append(mi.data(RadioStationModel::StationIdRole).toString());
        }
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
