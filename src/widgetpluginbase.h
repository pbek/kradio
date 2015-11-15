/***************************************************************************
                          widgetplugins.h  -  description
                             -------------------
    begin                : Mi Aug 27 2003
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

#ifndef KRADIO_WIDGETPLUGINS_INTERFACES_H
#define KRADIO_WIDGETPLUGINS_INTERFACES_H

#include <QtGui/QAction>

#include "pluginbase.h"


class QWidget;
class KConfigGroup;

class KDE_EXPORT WidgetPluginBase : public PluginBase
{
public :
    WidgetPluginBase(QWidget *myself, const QString &instanceID, const QString &name, const QString &description);

    virtual void     saveState    (      KConfigGroup &) const;
    virtual void     restoreState (const KConfigGroup &);
    virtual void     restoreState (const KConfigGroup &, bool showByDefault);
    virtual void     startPlugin();

    virtual       QWidget *getWidget()       { return m_myself; }
    virtual const QWidget *getWidget() const { return m_myself; }

    virtual bool           isReallyVisible(const QWidget *w = NULL, bool ignore_mapping_state = false) const;
    virtual bool           isAnywhereVisible(const QWidget *w = NULL) const;

    virtual QAction       *getHideShowAction() { return &m_HideShowAction; }
    virtual void           updateHideShowAction(bool show);

    virtual void           showOnOrgDesktop();

protected:
    virtual void toggleShown()    = 0; // must be implemented as protected SLOT in the widget plugin!
    virtual void setVisible(bool) = 0; // this QWidget method must be intercepted

    virtual void pShow ();
    virtual void pHide ();
    virtual void pSetVisible(bool v);
    virtual void pToggleShown ();

    virtual void showEvent(QShowEvent *) = 0;
    virtual void pShowEvent(QShowEvent *);
    virtual void hideEvent(QHideEvent *) = 0;
    virtual void pHideEvent(QHideEvent *);

    virtual void notifyManager(bool shown);

    virtual void getKWinState(const QWidget *w = NULL) const;


protected:
    QWidget            *m_myself;
    QAction             m_HideShowAction;
    bool                m_restoreShow;
    bool                m_geoRestoreFlag;
    bool                m_ignoreHideShow;

    // temporary data
    mutable bool        m_geoCacheValid;
    mutable bool        m_saveMinimized;
    mutable bool        m_saveMaximized;
    mutable bool        m_saveSticky;
    mutable int         m_saveDesktop;
    mutable QRect       m_saveGeometry;


};



#endif
