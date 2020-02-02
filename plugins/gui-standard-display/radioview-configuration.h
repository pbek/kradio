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

#include <QtWidgets/QTabWidget>
#include "pluginbase_config_page.h"

class RadioViewConfiguration : public PluginConfigPageBase
{
Q_OBJECT
protected:
    
    QTabWidget   * m_tabWidget;
    bool           m_dirty;
    
public :
    RadioViewConfiguration(QWidget *parent = NULL);
    virtual ~RadioViewConfiguration();

    int  addElementTab    (           PluginConfigPageBase *page,                    const QString &label);
    int  addElementTab    (           PluginConfigPageBase *page, const QIcon &icon, const QString &label);
    int  insertElementTab (int index, PluginConfigPageBase *page,                    const QString &label);
    int  insertElementTab (int index, PluginConfigPageBase *page, const QIcon &icon, const QString &label);
    void removeElementTab (int index);
    
protected:
    
    void connectElementTab(PluginConfigPageBase *page);

public slots:

    virtual void slotOK    () override;
    virtual void slotCancel() override;
    void         slotSetDirty();

signals:

    void sigOK();
    void sigCancel();

protected:
    void checkTabBar();

}; // RadioViewConfiguration




#endif
