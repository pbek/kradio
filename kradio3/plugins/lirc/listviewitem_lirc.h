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

#ifndef LISTVIEWITEM_LIRC_H
#define LISTVIEWITEM_LIRC_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <klistview.h>

class ListViewItemLirc : public QObject, public KListViewItem
{
Q_OBJECT
public:
    ListViewItemLirc(QListView *parent, QListViewItem *after);
    ~ListViewItemLirc();

    bool isRenamingInProcess() const { return m_renamingInProcess >= 0; }
    int  getRenamingColumn()   const { return m_renamingInProcess; }

    virtual void startRename(int col);
    virtual void okRename(int col);
    virtual void cancelRename(int col);

signals:

    void sigRenamingStarted(ListViewItemLirc *sender, int column);
    void sigRenamingStopped(ListViewItemLirc *sender, int column);

protected:

    int m_renamingInProcess;
};

#endif
