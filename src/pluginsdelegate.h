/***************************************************************************
                          pluginsdelegate.h  -  description
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

#ifndef KRADIO_PLUGINSDELEGATE_H
#define KRADIO_PLUGINSDELEGATE_H

#include <QStyledItemDelegate>

class PluginsDelegate : public QStyledItemDelegate
{
Q_OBJECT
public:
    PluginsDelegate(QObject *parent = 0);
    ~PluginsDelegate();

    virtual void  paint   (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif
