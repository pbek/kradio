/***************************************************************************
                          plugins.h  -  description
                             -------------------
    begin                : Mon Mär 10 2003
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

/////////////////////////////////////////////////////////////////////////////

#ifndef KRADIO_PLUGINS_INTERFACES_H
#define KRADIO_PLUGINS_INTERFACES_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "interfaces.h"
#include <qstring.h>
#include <qobject.h>
#include <qptrlist.h>

class PluginManager;
class PluginBase;
class QWidget;
class KConfig;

typedef QPtrList<PluginBase>           PluginList;
typedef QPtrListIterator<PluginBase>   PluginIterator;

/* PluginBase must be inherited from Interface so that a plugin can be used
   in Interface::connect functions.

   PluginBase must not be inherited from QObject, because derived classes may
   be inherited e.g. from QWidget (multiple inheritance is not possible with
   OBjects). But we must be able to receive destroy messages e.g. from
   configuration pages. Thus we need the special callback member
   m_destroyNotifier.

   PluginBase is derived from Interface to provide connection facilities.
   In case of multiple inheritance from interface classes, connect and disconnect
   methods have to be reimplemented in order to call all inherited
   connect/disconnect methods.

*/


class WidgetPluginBase;

struct ConfigPageInfo
{
	ConfigPageInfo () : configPage(NULL) {}
	ConfigPageInfo (QWidget *cp,
	                const QString &in,
	                const QString &ph,
	                const QString &icon)
	  : configPage (cp),
	    itemName(in),
	    pageHeader(ph),
	    iconName(icon)
	{}

	QWidget  *configPage;
	QString   itemName,
			  pageHeader,
			  iconName;
};


class PluginBase : virtual public Interface
{
friend class PluginManager;
public :
	         PluginBase(const QString &name);
	virtual ~PluginBase();

	const QString &name() const { return m_name; }
	      QString &name()       { return m_name; }

	// interaction with pluginmanager
protected:
	virtual bool setManager (PluginManager *);
	virtual void unsetManager ();
	virtual bool isManagerSet () const;

public:

	// these two methods will request a configuration page or
	// plugin page from plugin manager
	// they will be deleted automatically when this plugin
	// is deleted, because we disconnect from pluginmanager
	// and the plugin manager will delete all associated gui elements
	virtual ConfigPageInfo createConfigurationPage () = 0;	
	virtual QWidget *createAboutPage () = 0;

	// save/restore status, window position, etc...

	virtual void   saveState (KConfig *) const = 0;
	virtual void   restoreState (KConfig *) = 0;

	//
	
	virtual void noticeWidgetPluginShown(WidgetPluginBase *, bool /*shown*/) {}
	virtual void noticePluginsChanged(const PluginList &) {}
	
protected :
	QString            m_name;
    PluginManager     *m_manager;
};




#endif
