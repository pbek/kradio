/***************************************************************************
                          widgetplugins.h  -  description
                             -------------------
    begin                : Mi Aug 27 2003
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

#ifndef KRADIO_WIDGETPLUGINS_INTERFACES_H
#define KRADIO_WIDGETPLUGINS_INTERFACES_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "plugins.h"

class QWidget;

class WidgetPluginBase : public PluginBase
{
public :
	WidgetPluginBase(const QString &name, const QString &description);

	virtual void     show ();
	virtual void     hide ();
	virtual void     toggleShown ();
	virtual void     show (bool show);
	virtual bool     isReallyVisible(const QWidget *w = NULL) const;

	virtual void     saveState    (KConfig *) const;
	virtual void     restoreState (KConfig *);

	virtual       QWidget *getWidget();
	virtual const QWidget *getWidget() const;

protected:
	virtual void showEvent(QShowEvent *);
	virtual void hideEvent(QHideEvent *);

	virtual void notifyManager(bool shown);

	virtual void getKWinState(const QWidget *w = NULL) const;

protected:
    // temporary data
    mutable bool        m_geoCacheValid;
    mutable bool		m_saveSticky;
    mutable int		    m_saveDesktop;
    mutable QRect		m_saveGeometry;

};



#endif
