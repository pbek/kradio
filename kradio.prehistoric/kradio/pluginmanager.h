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
#include "plugins.h"

class PluginBase;
class WidgetPluginBase;
class PluginConfigurationDialog;
class QWidget;
class KConfig;
class QFrame;
class KAboutDialog;
class KDialogBase;

struct ConfigPageInfo;

class PluginManager
{
public :
	         PluginManager(const QString &configDialogTitle, const QString &aboutTitle);
	virtual ~PluginManager();


	// managing plugins

	const PluginList     &plugins() const { return m_plugins; }

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

	virtual PluginConfigurationDialog *getConfigDialog();
	virtual KDialogBase               *getAboutDialog();

    virtual void         noticeWidgetPluginShown(WidgetPluginBase *p, bool shown);
	
protected :
	virtual void         createConfigDialog(const QString &title = QString::null);
	virtual void         createAboutDialog (const QString &title = QString::null);

    virtual void         addConfigurationPage (PluginBase *forWhom,
											   const ConfigPageInfo	&info);
    virtual void         addAboutPage         (PluginBase *forWhom,
											   const AboutPageInfo	&info);

	// PluginManager's data & types ;)
protected:
    typedef QPtrDict<QFrame>               QFrameDict;
    typedef QPtrDictIterator<QFrame>       QFrameDictIterator;
    typedef QPtrDict<QWidget>              QWidgetDict;
    typedef QPtrDictIterator<QWidget>      QWidgetDictIterator;

    PluginList   m_plugins;
    
    QFrameDict   m_configPageFrames;
    QWidgetDict  m_configPages;
    
    QFrameDict   m_aboutPageFrames;
    QWidgetDict  m_aboutPages;
    
    PluginConfigurationDialog *m_configDialog;
    KDialogBase               *m_aboutDialog;
    QString                    m_configDialogTitle;
    QString                    m_aboutDialogTitle;
};




#endif
