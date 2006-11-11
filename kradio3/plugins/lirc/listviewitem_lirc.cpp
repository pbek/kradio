/***************************************************************************
                       listviewitem_lirc.cpp  -  description
                             -------------------
    begin                : Sun Aug 14 2005
    copyright            : (C) 2005 by Martin Witte
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

#include "listviewitem_lirc.h"

ListViewItemLirc::ListViewItemLirc(QListView *parent, QListViewItem *after)
    : KListViewItem(parent, after),
      m_renamingInProcess(-1)
{
}

ListViewItemLirc::~ListViewItemLirc()
{
}

void ListViewItemLirc::startRename(int col)
{
    KListViewItem::startRename(col);
    m_renamingInProcess = col;
    emit sigRenamingStarted(this, col);
}

void ListViewItemLirc::okRename(int col)
{
    KListViewItem::okRename(col);
    m_renamingInProcess = -1;
    emit sigRenamingStopped(this, col);
}

void ListViewItemLirc::cancelRename(int col)
{
    KListViewItem::cancelRename(col);
    m_renamingInProcess = -1;
    emit sigRenamingStopped(this, col);
}

#include "listviewitem_lirc.moc"
