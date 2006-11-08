/***************************************************************************
                          widgetplugins.cpp  -  description
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

#include "widgetplugins.h"
#include "pluginmanager.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kwin.h>
#include <qwidget.h>
#include <kconfig.h>
#include <klocale.h>

WidgetPluginBase::WidgetPluginBase(const QString &name, const QString &description)
  : PluginBase(name, description),
    m_geoCacheValid(false),
    m_geoRestoreFlag(false),
    m_restoreShow(false)
{
}

/*
QWidget *WidgetPluginBase::getWidget()
{
    return dynamic_cast<QWidget*>(this);
}


const QWidget *WidgetPluginBase::getWidget() const
{
    return dynamic_cast<const QWidget*>(this);
}
*/

void WidgetPluginBase::notifyManager(bool shown)
{
    if (m_manager)
        m_manager->noticeWidgetPluginShown(this, shown);
}


bool WidgetPluginBase::isReallyVisible(const QWidget *_w) const
{
    const QWidget *w = _w ? _w : getWidget();
    if (!w) return false;
    KWin::WindowInfo i = KWin::WindowInfo(w->winId(), 0, 0);
    return (i.mappingState() == NET::Visible)
            && w->isVisible()
            && (i.onAllDesktops() || i.isOnCurrentDesktop());
}


bool WidgetPluginBase::isAnywhereVisible(const QWidget *_w) const
{
    const QWidget *w = _w ? _w : getWidget();
    if (!w) return false;
    return w->isVisible();
}


void WidgetPluginBase::pShow(bool on)
{
    QWidget *w = getWidget();
    if (!w) return;
    if (on && !isReallyVisible(w))
        w->show();
    else if (!on && !w->isHidden())
        w->hide();
}


void WidgetPluginBase::pToggleShown()
{
    QWidget *w = getWidget();
    if (!w) return;
    if (!isReallyVisible(w))
        w->show();
    else
        w->hide();
}


void WidgetPluginBase::pShowOnOrgDesktop()
{
    logDebug(QString("%1::pShowOnOrgDesktop: all: %2, desktop: %3, visible:%4, anywherevisible:%5, cachevalid: %6").arg(name()).arg(m_saveSticky).arg(m_saveDesktop).arg(isReallyVisible()).arg(isAnywhereVisible()).arg(m_geoCacheValid));
    if (m_geoCacheValid && (!isReallyVisible() || m_geoRestoreFlag) ) {
        QWidget *w = getWidget();
        if (!w) return;
        WId  id = w->winId();

        KWin::setOnAllDesktops(id, m_saveSticky);
        if (!m_saveSticky) {
            KWin::setOnDesktop(id, m_saveDesktop);
        }

        w->resize(m_saveGeometry.size());
        w->move(m_saveGeometry.topLeft());

        KWin::deIconifyWindow(id);

        KWin::setMainWindow(getWidget(), 0);
        //w->show();
    }
}

void WidgetPluginBase::pShow()
{
    logDebug(QString("%1::pShow: all: %2, desktop: %3, visible:%4, anywherevisible:%5, cachevalid: %6").arg(name()).arg(m_saveSticky).arg(m_saveDesktop).arg(isReallyVisible()).arg(isAnywhereVisible()).arg(m_geoCacheValid));
    if (m_geoCacheValid && (!isReallyVisible() || m_geoRestoreFlag) ) {
        QWidget *w = getWidget();
        if (!w) return;
        WId  id = w->winId();

        KWin::setOnAllDesktops(id, m_saveSticky);
        if (!m_saveSticky)
            KWin::setOnDesktop(id, KWin::currentDesktop());

        w->resize(m_saveGeometry.size());
        w->move(m_saveGeometry.topLeft());
        KWin::setMainWindow(getWidget(), 0);
    }
}


void WidgetPluginBase::pHide()
{
    logDebug(QString("%1::pHide1: all: %2, desktop: %3, visible:%4, anywherevisible:%5, cachevalid: %6").arg(name()).arg(m_saveSticky).arg(m_saveDesktop).arg(isReallyVisible()).arg(isAnywhereVisible()).arg(m_geoCacheValid));
    getKWinState();
    logDebug(QString("%1::pHide2: all: %2, desktop: %3, visible:%4, anywherevisible:%5, cachevalid: %6").arg(name()).arg(m_saveSticky).arg(m_saveDesktop).arg(isReallyVisible()).arg(isAnywhereVisible()).arg(m_geoCacheValid));
}


void WidgetPluginBase::pShowEvent(QShowEvent *)
{
    notifyManager (true);
}


void WidgetPluginBase::pHideEvent(QHideEvent *)
{
    notifyManager (false);
}


void WidgetPluginBase::getKWinState(const QWidget *_w) const
{
    if (m_geoRestoreFlag) return;

    const QWidget *w = _w ? _w : getWidget();
    if (!w) return;
    if (w->isVisible()) {
        KWin::WindowInfo i = KWin::WindowInfo(w->winId(), 0, 0);
        m_saveSticky       = i.onAllDesktops();
        m_saveDesktop      = i.desktop();
        m_saveGeometry     = QRect(w->pos(), w->size());
        m_geoCacheValid    = true;
    }
}


void   WidgetPluginBase::saveState (KConfig *config) const
{
    const QWidget *w = getWidget();
    getKWinState(w);

    config->writeEntry("hidden",   w ? w->isHidden() : false);
    config->writeEntry("sticky",   m_saveSticky);
    config->writeEntry("desktop",  m_saveDesktop);
    config->writeEntry("geometry", m_saveGeometry);
    config->writeEntry("geoCacheValid", m_geoCacheValid);
}


void   WidgetPluginBase::restoreState (KConfig *config, bool showByDefault)
{
    m_geoCacheValid= config->readBoolEntry("geoCacheValid", false);
    m_saveDesktop  = config->readNumEntry ("desktop", 1);
    m_saveSticky   = config->readBoolEntry("sticky",  false);
    m_saveGeometry = config->readRectEntry("geometry");

    m_restoreShow  = !config->readBoolEntry("hidden", !showByDefault);
}


void   WidgetPluginBase::restoreState (KConfig *config)
{
    restoreState(config, true);
}


void   WidgetPluginBase::startPlugin()
{
    PluginBase::startPlugin();

    QWidget *w = getWidget();
    if (w) {
        m_geoRestoreFlag = true;
        if (!m_restoreShow) w->hide();
        else                w->show();
        m_geoRestoreFlag = false;
    }
}

