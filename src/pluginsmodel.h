/***************************************************************************
                          pluginsmodel.h  -  description
                             -------------------
    copyright            : (C) 2016 by Pino Toscano
    email                : toscano.pino@tiscali.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRADIO_PLUGINSMODEL_H
#define KRADIO_PLUGINSMODEL_H

#include <QAbstractItemModel>
#include <QMap>

#include "instancemanager.h"

class PluginBase;

enum {
    ClassNameRole = Qt::UserRole + 1,
    InstanceNameRole
};

class PluginsModel : public QAbstractItemModel
{
Q_OBJECT
public:
    PluginsModel(QObject *parent = 0);
    ~PluginsModel();

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    void setPluginClasses(const QMap<QString, PluginClassInfo> &classes);
    QModelIndex addPlugin(PluginBase *plugin);
    void removePlugin(PluginBase *plugin);
    void notifyPluginRenamed(PluginBase *plugin);

private:
    struct Private;
    Private *d;
};

#endif
