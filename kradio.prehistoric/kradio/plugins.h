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

#include "interfaces.h"
#include <qstring.h>
#include <qobject.h>

class PluginManager;
class KDialogBase;
class QWidget;
class QFrame;
class KConfig;

/////////////////////////////////////////////////////////////////////////////


class PluginBase : public QObject, virtual public Interface
{
friend class PluginManager;
public :
	         PluginBase();
	virtual ~PluginBase();


	// Only the plugin itself knows what interfaces it implements. Thus it has
	// to call the apropriate InterfaceBase::establishConnection methods

	virtual void connect    (PluginBase *p) = 0;
	virtual void disconnect (PluginBase *p) = 0;

	// interaction with pluginmanager
protected:
	virtual bool setManager (PluginManager *);
	virtual void unsetManager ();
	virtual bool isManagerSet () const;

	////////////////////////////////////

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

protected slots:	
	void registerGuiElement   (QObject *o);
	void unregisterGuiElement (QObject *o);

protected :	
    PluginManager     *m_manager;
    QPtrList<QObject>  m_guiElements;
};



#endif
