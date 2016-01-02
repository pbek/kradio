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

#include "styleditemdelegate_lirc.h"

StyledItemDelegateLirc::StyledItemDelegateLirc(QObject *parent)
    : QStyledItemDelegate(parent)
    , m_lastEditor(0)
{
    connect(this, SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),
            this, SLOT(slotEditorClosed(QWidget*,QAbstractItemDelegate::EndEditHint)));
}

StyledItemDelegateLirc::~StyledItemDelegateLirc()
{
}

QWidget *StyledItemDelegateLirc::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QWidget *w = QStyledItemDelegate::createEditor(parent, option, index);
    if (w) {
        m_lastEditor = w;
        emit startedRenaming(index);
    }
    return w;
}

void StyledItemDelegateLirc::stopEditing()
{
    if (!m_lastEditor) {
        return;
    }

    emit closeEditor(m_lastEditor);
}

void StyledItemDelegateLirc::slotEditorClosed(QWidget *editor, QAbstractItemDelegate::EndEditHint)
{
    if (editor == m_lastEditor) {
        m_lastEditor = NULL;
    }
}
