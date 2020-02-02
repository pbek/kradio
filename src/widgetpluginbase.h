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

#include <QAction>

#include "pluginbase.h"


class QWidget;
class KConfigGroup;
class WidgetPluginBase;


class KRADIO5_EXPORT widgetPluginHelper_t : public QObject
{
Q_OBJECT
protected:
    WidgetPluginBase  *m_parent;
    
public:
    widgetPluginHelper_t(WidgetPluginBase *parent)
        : m_parent(parent)
    {} // CTOR
    
public Q_SLOTS:
    void slotToggleShown();
    
}; // widgetPluginHelper_t


class KRADIO5_EXPORT WidgetPluginBase : public PluginBase
{
protected:
    
public :
    WidgetPluginBase(QWidget *myself, const QString &instanceID, const QString &name, const QString &description);

    virtual void     saveState    (      KConfigGroup &) const override;
    virtual void     restoreState (const KConfigGroup &)       override;
    virtual void     restoreState (const KConfigGroup &, bool showByDefault);
    virtual void     startPlugin() override;

    virtual       QWidget *getWidget()       { return m_myself; }
    virtual const QWidget *getWidget() const { return m_myself; }

    virtual bool           isReallyVisible(const QWidget *w = NULL, bool ignore_mapping_state = false) const;
    virtual bool           isAnywhereVisible(const QWidget *w = NULL) const;

    virtual QAction       *getHideShowAction() { return &m_HideShowAction; }
    virtual void           updateHideShowAction(bool show);

    virtual void           showOnOrgDesktop();

protected:
    friend  class widgetPluginHelper_t;
    
    virtual void toggleShown();
    virtual void setVisible(bool) = 0; // this QWidget method must be intercepted

    virtual void pShow ();
    virtual void pHide ();
    virtual void pSetVisible(bool v);

    virtual void showEvent(QShowEvent *) = 0;
    virtual void pShowEvent(QShowEvent *);
    virtual void hideEvent(QHideEvent *) = 0;
    virtual void pHideEvent(QHideEvent *);

    virtual void notifyManager(bool shown);

    virtual void getKWinState(const QWidget *w = NULL) const;


protected:
    widgetPluginHelper_t  m_toggleHelper;
    QWidget              *m_myself;
    QAction               m_HideShowAction;
    bool                  m_restoreShow;
    bool                  m_geoRestoreFlag;
    bool                  m_ignoreHideShow;

    // temporary data
    mutable bool          m_geoCacheValid;
    mutable bool          m_saveMinimized;
    mutable bool          m_saveMaximized;
    mutable bool          m_saveSticky;
    mutable int           m_saveDesktop;
    mutable QRect         m_saveGeometry;


}; // WidgetPluginBase



#endif
