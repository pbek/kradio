/***************************************************************************
                          pluginmanager.h  -  description
                             -------------------
    begin                : Mon Apr 28 2003
    copyright            : (C) 2003 by Martin Witte
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

#ifndef KRADIO_PLUGINMANAGER_INTERFACES_H
#define KRADIO_PLUGINMANAGER_INTERFACES_H

#include <qstring.h>
#include <qptrdict.h>

#include "interfaces.h"

class PluginBase;
class WidgetPluginBase;
class PluginConfigurationDialog;
class QWidget;
class KConfig;

class PluginManager
{
public :
	         PluginManager(const QString &configDialogTitle);
	virtual ~PluginManager();


	// managing plugins

	// after insert, pluginManager is responsible for deletion
	virtual void         insertPlugin(PluginBase *);

	// remove and delete plugin
	virtual void         deletePlugin(PluginBase *);
	
	// remove plugin, afterwards pluginManager is no longer responsible for deletion
	virtual void         removePlugin(PluginBase *);

	// operations on all plugins

	virtual void         saveState    (KConfig *) const;
	virtual void         restoreState (KConfig *);

	// configuration dialog handling

	virtual PluginConfigurationDialog *getConfigDialog() { return m_configDialog; }

    virtual QFrame      *addConfigurationPage (PluginBase *forWhom,
                                               const QString &itemname,
									           const QString &header,
									           const QPixmap &icon);
	virtual void         connectWithConfigDialog(QObject *o);
	
	virtual void         noticeWidgetPluginShown(WidgetPluginBase *p, bool shown);
	
protected :
	virtual void         createConfigDialog(const QString &title = QString());


	// PluginManager's data & types ;)
protected:
    typedef QPtrList<PluginBase>           PluginList;
    typedef QPtrListIterator<PluginBase>   PluginIterator;
    typedef QPtrDict<QFrame>               QFrameDict;
    typedef QPtrDictIterator<QFrame>       QFrameDictIterator;
    

    PluginList   m_plugins;
    QFrameDict   m_configPages;
    
    PluginConfigurationDialog *m_configDialog;
};




#endif
