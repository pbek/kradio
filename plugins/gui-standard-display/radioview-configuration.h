/***************************************************************************
                          radioview-configuration.h  -  description
                             -------------------
    begin                : Fr Aug 15 2003
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

#ifndef KRADIO_RADIOVIEW_CONFIGURATION_H
#define KRADIO_RADIOVIEW_CONFIGURATION_H

#include <ktabwidget.h>

class RadioViewConfiguration : public KTabWidget
{
Q_OBJECT
public :
    RadioViewConfiguration(QWidget *parent = NULL);
    ~RadioViewConfiguration();

    int  addElementTab    (QWidget *page, const QString &label);
    int  addElementTab    (QWidget *page, const QIcon &icon, const QString &label);
    int  insertElementTab (int index, QWidget *page, const QString &label);
    int  insertElementTab (int index, QWidget *page, const QIcon &icon, const QString &label);
    void removeElementTab (int index);

public slots:

    void slotOK();
    void slotCancel();
    void slotSetDirty();

signals:

    void sigOK();
    void sigCancel();

protected:
    bool m_dirty;
};



#endif
