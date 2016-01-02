/***************************************************************************
                          nosizefontrequester.cpp  -  description
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

#ifndef STYLEDDELEGATE_LIRC_H
#define STYLEDDELEGATE_LIRC_H

#include <QStyledItemDelegate>

class StyledItemDelegateLirc : public QStyledItemDelegate
{
Q_OBJECT
public:
    StyledItemDelegateLirc(QObject *parent = 0);
    ~StyledItemDelegateLirc();

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void stopEditing();

signals:
    void startedRenaming(const QModelIndex &index) const;

private slots:
    void slotEditorClosed(QWidget *editor, QAbstractItemDelegate::EndEditHint hint);

private:
    mutable QWidget *m_lastEditor;
};

#endif
