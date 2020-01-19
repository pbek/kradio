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

#include <klocalizedstring.h>
#include <QDataStream>
#include <QPixmap>
#include <QDragMoveEvent>
#include <QHeaderView>
#include <QMimeData>

#include <kconfiggroup.h>

static const char rsm_mime[] = "application/x-radiostationmodel-list";

static QDataStream &operator<<(QDataStream &out, const RadioStationModel::RS &rs)
{
    out << rs.nr << rs.id << rs.name << rs.description << rs.icon;
    return out;
}


static QDataStream &operator>>(QDataStream &in, RadioStationModel::RS &rs)
{
    in >> rs.nr >> rs.id >> rs.name >> rs.description >> rs.icon;
    return in;
}


Q_DECLARE_METATYPE(RadioStationModel::RS)


RadioStationModel::RadioStationModel(QObject *parent)
  : QAbstractItemModel(parent)
{
    qRegisterMetaTypeStreamOperators<RadioStationModel::RS>("RadioStationModel::RS");
}


RadioStationModel::~RadioStationModel()
{
}


int RadioStationModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 3;
}


QVariant RadioStationModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_stations.count()) {
        return QVariant();
    }

    const RS &rs = m_stations.at(index.row());
    switch(role) {
        case StationIdRole: return rs.id;
        case Qt::DisplayRole:
            switch(index.column()) {
                case 1: return rs.name;
                case 2: return rs.description;
            }
            break;
        case Qt::DecorationRole:
            switch(index.column()) {
                case 0: return QIcon(rs.icon);
            }
            break;
    }

    return QVariant();
}


bool RadioStationModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int /*row*/, int column, const QModelIndex &/*parent*/)
{
    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasFormat(rsm_mime) || column >= 3)
        return false;

    QByteArray encodedData = data->data(rsm_mime);
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QList<RS> newItems;
    stream >> newItems;

    foreach (const RS &rs, newItems) {
        const int beginRow = indexForNr(rs.nr);

        beginInsertRows(QModelIndex(), beginRow, beginRow);
        if (beginRow < m_stations.count() - 1) {
            m_stations.insert(beginRow, rs);
        } else {
            m_stations.append(rs);
        }
        endInsertRows();
    }

    return true;
}

Qt::ItemFlags RadioStationModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

    if (index.isValid()) {
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    } else {
        return Qt::ItemIsDropEnabled | defaultFlags;
    }
}


QVariant RadioStationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QVariant();
    }

    switch(section) {
        case 0: return i18n("Icon");
        case 1: return i18n("Station");
        case 2: return i18n("Description");
    }
    return QVariant();
}


QModelIndex RadioStationModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() || row < 0 || row >= m_stations.count()
        || column < 0 || column >= 3) {
        return QModelIndex();
    }

    return createIndex(row, column);
}


bool RadioStationModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || row < 0 || row > m_stations.count() || count <= 0) {
        return false;
    }

    beginInsertRows(parent, row, row + count - 1);
    if (row < m_stations.count() - 1) {
        m_stations.insert(row, count, RS());
    } else {
        m_stations.resize(m_stations.size() + count);
    }
    endInsertRows();
    return true;
}


QMimeData *RadioStationModel::mimeData(const QModelIndexList &indexes) const
{
    QSet<int> rowsSet;
    rowsSet.reserve(indexes.count());
    foreach (const QModelIndex &index, indexes) {
        if (index.isValid()) {
            rowsSet << index.row();
        }
    }
    QList<int> rows = rowsSet.toList();
    qSort(rows);

    QList<RS> items;
    QStringList ids;
    foreach (int row, rows) {
        items << m_stations.at(row);
        ids << m_stations.at(row).id;
    }

    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    stream << items;
    mimeData->setData(rsm_mime, encodedData);

    StationDragObject::encode(mimeData, ids);

    return mimeData;
}


QStringList RadioStationModel::mimeTypes() const
{
    return QStringList() << QLatin1String(rsm_mime);
}


QModelIndex RadioStationModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}


bool RadioStationModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || row < 0 || row >= m_stations.count() || count <= 0) {
        return false;
    }

    count = qMin(count, m_stations.count() - row);
    beginRemoveRows(parent, row, row + count - 1);
    m_stations.remove(row, count);
    endRemoveRows();

    return true;
}


int RadioStationModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_stations.count();
}


Qt::DropActions RadioStationModel::supportedDropActions() const
{
    return Qt::MoveAction;
}


void RadioStationModel::setStation(int idx, const RadioStation &station, int nr)
{
    setRS(m_stations[idx], station, nr);

    emit dataChanged(createIndex(idx, 0), createIndex(idx, 3));
}


void RadioStationModel::setStations(const StationList &list)
{
    beginResetModel();
    m_stations.resize(list.count());
    for (int i = 0; i < list.count(); ++i) {
        setRS(m_stations[i], list.at(i), i+1);
    }
    endResetModel();
}


void RadioStationModel::insertStationByNr(const RadioStation &st, int nr)
{
    RS rs;
    setRS(rs, st, nr);

    const int beginRow = indexForNr(nr);
    beginInsertRows(QModelIndex(), beginRow, beginRow);
    if (beginRow < m_stations.count() - 1) {
        m_stations.insert(beginRow, rs);
    } else {
        m_stations.append(rs);
    }
    endInsertRows();
}


void RadioStationModel::setRS(RS &rs, const RadioStation &station, int nr)
{
    rs.nr = nr;
    rs.id = station.stationID();
    rs.name = station.name();
    rs.description = station.description();
    rs.icon = QPixmap(station.iconName());
}


int RadioStationModel::indexForNr(int nr) const
{
    int i = 0;
    for (; i < m_stations.count(); ++i) {
        if (m_stations.at(i).nr > nr)
            break;
    }
    return i;
}



RadioStationListView::RadioStationListView(QWidget *parent, const char *name)
  : QTreeView(parent)
{
    setObjectName(name);
    setRootIsDecorated(false);
    setDragEnabled(true);
    setDragDropMode(DragDrop);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);

    QObject::connect(this, SIGNAL(doubleClicked(QModelIndex)),
                     this, SLOT(slotStationActivation(QModelIndex)));

    m_model = new RadioStationModel(this);
    setModel(m_model);
    header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
}


RadioStationListView::~RadioStationListView()
{
}


void RadioStationListView::setStation(int idx, const RadioStation &s, int nr)
{
    if (idx < 0) {
        m_model->insertRow(0);
        idx = 0;
    } else if (idx >= childCount()) {
        m_model->insertRow(childCount());
        idx = childCount() - 1;
    }

    m_model->setStation(idx, s, nr > 0 ? nr : idx+1);
}


void RadioStationListView::appendStation(const RadioStation &st, int nr)
{
    setStation(childCount(), st, nr);
}


void RadioStationListView::setStations(const StationList &stations)
{
    m_model->setStations(stations);
}


void RadioStationListView::insertStationByNr(const RadioStation &st, int nr)
{
    m_model->insertStationByNr(st, nr);
}


void RadioStationListView::removeStation(int idx)
{
    m_model->removeRow(idx);
}


void RadioStationListView::setCurrentStation(int idx)
{
    setCurrentIndex(m_model->index(idx, 0));
}


int RadioStationListView::currentStationIndex() const
{
    const QModelIndex mi = currentIndex();
    return mi.isValid() ? mi.row() : -1;
}


QString RadioStationListView::currentStationID() const
{
    const QModelIndex mi = currentIndex();
    return mi.isValid() ? mi.data(RadioStationModel::StationIdRole).toString() : QString();
}


int RadioStationListView::childCount() const
{
    return m_model->rowCount();
}


void RadioStationListView::clear()
{
    m_model->setStations(StationList());
}


void RadioStationListView::slotStationActivation(const QModelIndex &index)
{
    emit sigStationActivated(index.row());
}


void RadioStationListView::saveState (KConfigGroup &cfg) const
{
    for (int i = 0; i < 3; ++i)
        cfg.writeEntry(QString(objectName()) + "_radiostation_listview_col_" + QString::number(i), columnWidth(i));
}


void RadioStationListView::restoreState (const KConfigGroup &cfg)
{
    for (int i = 0; i < 3; ++i)
        setColumnWidth(i, cfg.readEntry(QString(objectName()) + "_radiostation_listview_col_" + QString::number(i), -1));
}


void RadioStationListView::dragMoveEvent(QDragMoveEvent *event)
{
    QTreeView::dragMoveEvent(event);
    if (event->isAccepted() && event->source() == this) {
        event->ignore();
    }
}


void RadioStationListView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTreeView::currentChanged(current, previous);

    emit sigCurrentStationChanged(current.row());
}

