/***************************************************************************
                       listviewitem_lirc.cpp  -  description
                             -------------------
    begin                : Sun Aug 14 2005
    copyright            : (C) 2005 by Martin Witte
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

class QColorGroup;

#include "listviewitem_lirc.h"

ListViewItemLirc::ListViewItemLirc(Q3ListView *parent, Q3ListViewItem *after)
    : Q3ListViewItem(parent, after),
      m_renamingInProcess(-1)
{
}

ListViewItemLirc::~ListViewItemLirc()
{
}

void ListViewItemLirc::startRename(int col)
{
    Q3ListViewItem::startRename(col);
    m_renamingInProcess = col;
    emit sigRenamingStarted(this, col);
}

void ListViewItemLirc::okRename(int col)
{
    Q3ListViewItem::okRename(col);
    m_renamingInProcess = -1;
    emit sigRenamingStopped(this, col);
}

void ListViewItemLirc::cancelRename(int col)
{
    Q3ListViewItem::cancelRename(col);
    m_renamingInProcess = -1;
    emit sigRenamingStopped(this, col);
}

#include "listviewitem_lirc.moc"
