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

	// creates QFrames with configuration or about page.
	// they will be deleted automatically when this plugin
	// is deleted.
	QFrame *createConfigurationPage (KDialogBase *dlg);	
	QFrame *createAboutPage (QWidget *parent);

	virtual void   saveState (KConfig *) const = 0;
	virtual void   restoreState (KConfig *) = 0;

protected :

	virtual QFrame *internal_createConfigurationPage(KDialogBase *dlg) = 0;
	virtual QFrame *internal_createAboutPage(QWidget *parent) = 0;

	void registerGuiElement   (QObject *o);
	void unregisterGuiElement (QObject *o);

protected :
	QString            m_name;
    PluginManager     *m_manager;
    QPtrList<QObject>  m_guiElements;

    
    friend class WidgetDestroyNotifier;
    
    WidgetDestroyNotifier *m_destroyNotifier;    
};




class WidgetDestroyNotifier : public QObject
{
Q_OBJECT
public:
	WidgetDestroyNotifier(PluginBase *parent);
	virtual ~WidgetDestroyNotifier();

public slots:
	virtual void noticeDestroy(QObject *o);

protected:
	PluginBase *m_parent;
};


#endif
