/***************************************************************************
                          plugins.h  -  description
                             -------------------
    begin                : Mon M�r 10 2003
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

class PluginManager;
class KDialogBase;
class QWidget;
class QFrame;
class KConfig;

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
	virtual void createConfigurationPage () = 0;	
	virtual void createAboutPage () = 0;

	// save/restore status, window position, etc...

	virtual void   saveState (KConfig *) const = 0;
	virtual void   restoreState (KConfig *) = 0;

	//
	
	virtual void noticeWidgetPluginShown(WidgetPluginBase *, bool /*shown*/) {}
	
protected :
	QString            m_name;
    PluginManager     *m_manager;
};



class WidgetPluginBase : public PluginBase
{
public :
	WidgetPluginBase(const QString &name) : PluginBase(name) {}

	virtual void     show () = 0;
	virtual void     show (bool show) = 0;
	virtual void     hide () = 0;

	virtual QWidget *getWidget();

protected:
	virtual void showEvent(QShowEvent *) = 0;
	virtual void hideEvent(QHideEvent *) = 0;

	virtual void notifyManager(bool shown);
};



#endif
