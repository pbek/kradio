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

#ifndef _KRADIO_RADIOSTATION_LISTVIEW_H_
#define _KRADIO_RADIOSTATION_LISTVIEW_H_

#include <QAbstractItemModel>
#include <QTreeView>

#include <kconfiggroup.h>

#include "kradio-def.h"

class RadioStation;
class StationList;

class KRADIO5_EXPORT RadioStationModel : public QAbstractItemModel
{
Q_OBJECT
public:
    enum { StationIdRole = Qt::UserRole };

    struct RS
    {
        int nr;
        QString id;
        QString name;
        QString description;
        QPixmap icon;
    };

    RadioStationModel(QObject *parent=0);
    virtual ~RadioStationModel();

    virtual int           columnCount(const QModelIndex &parent = QModelIndex())     const override;
    virtual QVariant      data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual bool          dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    virtual QStringList mimeTypes() const override;
    virtual QModelIndex parent(const QModelIndex &index) const override;
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual Qt::DropActions supportedDropActions() const override;

    void setStation(int idx, const RadioStation &, int nr);
    void setStations(const StationList &);
    void insertStationByNr(const RadioStation &, int nr);

private:
    void setRS(RS &rs, const RadioStation &station, int nr);
    int indexForNr(int nr) const;

    QVector<RS> m_stations;
};

class KRADIO5_EXPORT RadioStationListView : public QTreeView
{
Q_OBJECT
public:
    RadioStationListView(QWidget *parent=0, const char *name=0);
    virtual ~RadioStationListView();

    void setStation(int idx, const RadioStation &, int nr = -1);
    void appendStation(const RadioStation &, int nr = -1);
    void setStations(const StationList &);
    void insertStationByNr(const RadioStation &, int nr);

    void removeStation(int idx);

    void setCurrentStation(int idx);
    int  currentStationIndex() const;
    QString currentStationID() const;

    int childCount() const;

    void clear();

    void   saveState    (KConfigGroup &) const;
    void   restoreState (const KConfigGroup &);


protected:

    virtual void dragMoveEvent(QDragMoveEvent *event) override;
    virtual void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;

protected slots:

    void slotStationActivation(const QModelIndex &);

signals:
    void sigStationActivated(int idx);
    void sigCurrentStationChanged(int idx);

protected:

    RadioStationModel *m_model;

};

#endif
