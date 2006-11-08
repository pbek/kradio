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

    virtual void     saveState    (KConfig *) const;
    virtual void     restoreState (KConfig *);
    virtual void     restoreState (KConfig *, bool showByDefault);
    virtual void     startPlugin();

    virtual       QWidget *getWidget() = 0;
    virtual const QWidget *getWidget() const = 0;

    virtual bool           isReallyVisible(const QWidget *w = NULL) const;
    virtual bool           isAnywhereVisible(const QWidget *w = NULL) const;
    virtual void           showOnOrgDesktop () = 0;

protected:
    virtual void pShowOnOrgDesktop ();
    virtual void pShow ();
    virtual void pShow (bool show);
    virtual void pHide ();
    virtual void pToggleShown ();

    virtual void pShowEvent(QShowEvent *);
    virtual void pHideEvent(QHideEvent *);

    virtual void notifyManager(bool shown);

    virtual void getKWinState(const QWidget *w = NULL) const;

protected:
    // temporary data
    mutable bool        m_geoCacheValid;
    mutable bool        m_saveSticky;
    mutable int         m_saveDesktop;
    mutable QRect       m_saveGeometry;

    bool                m_geoRestoreFlag;
    bool                m_restoreShow;
};



#endif
