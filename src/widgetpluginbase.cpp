/***************************************************************************
                          widgetplugins.cpp  -  description
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

#include "widgetpluginbase.h"
#include "pluginmanager.h"

#include <KWindowSystem>
#include <KWindowInfo>
#include <QWidget>
#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <QtGui/QIcon>
#include <QtGui/QWindow>

WidgetPluginBase::WidgetPluginBase(QWidget *myself, const QString &instanceID, const QString &name, const QString &description)
  : PluginBase(instanceID, name, description),
    m_myself(myself),
    m_HideShowAction(myself),
    m_restoreShow(false),
    m_geoRestoreFlag(false),
    m_ignoreHideShow(false),
    m_geoCacheValid(false)
{
    auto     widget = getWidget();
    widget->setAttribute(Qt::WA_NativeWindow, true);
    auto     window = widget->window();
    auto     handle = window->windowHandle();
    KWindowSystem::setMainWindow(handle, 0);
} // CTOR


void WidgetPluginBase::notifyManager(bool shown)
{
    if (m_manager)
        m_manager->noticeWidgetPluginShown(this, shown);
}


bool WidgetPluginBase::isReallyVisible(const QWidget *_w, bool ignore_mapping_state) const
{
    const QWidget *w = _w ? _w : getWidget();
    if (!w) return false;
    KWindowInfo i(w->winId(), NET::WMDesktop | NET::WMState);
    bool mappingVisible   = ignore_mapping_state || (i.mappingState() == NET::Visible);
    bool widgetVisible    = w->isVisible();
    bool onAllDesktops    = i.onAllDesktops();
    bool onCurrentDesktop = i.isOnCurrentDesktop();
    return mappingVisible && widgetVisible && (onAllDesktops || onCurrentDesktop);
}


bool WidgetPluginBase::isAnywhereVisible(const QWidget *_w) const
{
    const QWidget *w = _w ? _w : getWidget();
    if (!w) return false;
    return w->isVisible();
}


void WidgetPluginBase::pSetVisible(bool v)
{
    QWidget *w = getWidget();
    if (!w) return;
    if (v && (!isReallyVisible(w) || m_geoRestoreFlag)) {
        pShow();
    } else if (!v && !w->isHidden()) {
        pHide();
    }
}


void WidgetPluginBase::pToggleShown()
{
    QWidget *w = getWidget();
    if (!w) return;
    if (!isReallyVisible(w) || m_geoRestoreFlag) {
        pShow();
        w->show();
        w->raise();
    } else {
        w->hide();
    }
}

// this function is not a hook, it really shows the widget
void WidgetPluginBase::showOnOrgDesktop()
{
    QWidget *w = getWidget();
    if (!w) return;
    WId  id = w->winId();

//     logDebug(QString("%1::pShowOnOrgDesktop: window managed1: %2").arg(name()).arg(KWindowSystem::hasWId(id)));
//     logDebug(QString("%1::pShowOnOrgDesktop: all: %2, desktop: %3, visible:%4, anywherevisible:%5, cachevalid: %6").arg(name()).arg(m_saveSticky).arg(m_saveDesktop).arg(isReallyVisible()).arg(isAnywhereVisible()).arg(m_geoCacheValid));
    if (m_geoCacheValid && (!isReallyVisible() || m_geoRestoreFlag) ) {

//         logDebug(QString("%1::pShowOnOrgDesktop: window managed2: %2").arg(name()).arg(KWindowSystem::hasWId(id)));

        KWindowSystem::setOnAllDesktops(id, m_saveSticky);
        if (!m_saveSticky) {
            KWindowSystem::setOnDesktop(id, m_saveDesktop);
            // currently very strange effects happen when we try to restore on another than
            // the current desktop... let's keep this so far as a known problem
//             KWindowSystem::setOnDesktop(id, KWindowSystem::currentDesktop());
        }

        w->resize(m_saveGeometry.size());
        w->move(m_saveGeometry.topLeft());

        if (m_saveMinimized) {
            w->setWindowState(Qt::WindowMinimized);
        } else if (m_saveMaximized) {
            w->setWindowState(Qt::WindowMaximized);
        } else {
            w->setWindowState(Qt::WindowNoState);
        }

    }


//         logDebug(QString("%1::pShowOnOrgDesktop: window managed3: %2").arg(name()).arg(KWindowSystem::hasWId(id)));

    bool old_flag    = m_ignoreHideShow;
    m_ignoreHideShow = true;
    w->show();
    KWindowSystem::unminimizeWindow(id);
    m_ignoreHideShow = old_flag;
//         logDebug(QString("%1::pShowOnOrgDesktop: window managed4: %2").arg(name()).arg(KWindowSystem::hasWId(id)));
}

// this funktion is more or less only a hook
// which adjusts some parameters when the widget is been shown
void WidgetPluginBase::pShow()
{
    if (m_ignoreHideShow) // means we are in showOnOrgDesktop and this is called recursively
        return;

    QWidget *w = getWidget();
    if (!w) return;
    WId  id = w->winId();
    bool use_cache = m_geoCacheValid && (!isReallyVisible() || m_geoRestoreFlag);

    // this is a normal show hook. thus if the widget is not on the current
    // desktop, we need to get it here.
    KWindowInfo i(id, NET::WMDesktop | NET::WMState);
    if (!i.onAllDesktops()) {
        KWindowSystem::setOnDesktop(id, KWindowSystem::currentDesktop());
    }

//     logDebug(QString("%1::pShow: window managed: %2").arg(name()).arg(KWindowSystem::hasWId(id)));
//     logDebug(QString("%1::pShow: all: %2, desktop: %3, visible:%4, anywherevisible:%5, cachevalid: %6").arg(name()).arg(m_saveSticky).arg(m_saveDesktop).arg(isReallyVisible()).arg(isAnywhereVisible()).arg(m_geoCacheValid));
    if (use_cache) {
        KWindowSystem::setOnAllDesktops(id, m_saveSticky);
        if (!m_saveSticky) {
            KWindowSystem::setOnDesktop(id, KWindowSystem::currentDesktop());
        }
        w->resize(m_saveGeometry.size());
        w->move(m_saveGeometry.topLeft());
    }
    KWindowSystem::unminimizeWindow(id);
    updateHideShowAction(true);
}


void WidgetPluginBase::pHide()
{
//     logDebug(QString("%1::pHide:  window managed: %2").arg(name()).arg(KWindowSystem::hasWId(getWidget()->winId())));
//     logDebug(QString("%1::pHide1: all: %2, desktop: %3, visible:%4, anywherevisible:%5, cachevalid: %6").arg(name()).arg(m_saveSticky).arg(m_saveDesktop).arg(isReallyVisible()).arg(isAnywhereVisible()).arg(m_geoCacheValid));
    getKWinState();
//     logDebug(QString("%1::pHide2: all: %2, desktop: %3, visible:%4, anywherevisible:%5, cachevalid: %6").arg(name()).arg(m_saveSticky).arg(m_saveDesktop).arg(isReallyVisible()).arg(isAnywhereVisible()).arg(m_geoCacheValid));
    updateHideShowAction(false);
}


void WidgetPluginBase::pShowEvent(QShowEvent *)
{
    updateHideShowAction(true);
}


void WidgetPluginBase::pHideEvent(QHideEvent *)
{
    updateHideShowAction(false);
}


void WidgetPluginBase::getKWinState(const QWidget *_w) const
{
    if (m_ignoreHideShow) return;

    const QWidget *w = _w ? _w : getWidget();
    if (!w) return;
    if (w->isVisible()) {
        KWindowInfo i(w->winId(), NET::WMDesktop | NET::WMState);
        m_saveMinimized    = i.isMinimized();
        m_saveMaximized    = w->isMaximized();
        m_saveSticky       = i.onAllDesktops();
        m_saveDesktop      = i.desktop();
        m_saveGeometry     = QRect(w->pos(), w->size());
        m_geoCacheValid    = true;
    }
}


void   WidgetPluginBase::saveState (KConfigGroup &config) const
{
    PluginBase::saveState(config);

    const QWidget *w = getWidget();
    getKWinState(w);

    config.writeEntry("hidden",   w ? w->isHidden() : false);
    config.writeEntry("minimized",  m_saveMinimized);
    config.writeEntry("maximized",  m_saveMaximized);
    config.writeEntry("sticky",     m_saveSticky);
    config.writeEntry("desktop",    m_saveDesktop);
    config.writeEntry("geometry",   m_saveGeometry);
    config.writeEntry("geoCacheValid", m_geoCacheValid);
}


void   WidgetPluginBase::restoreState (const KConfigGroup &config, bool showByDefault)
{
    PluginBase::restoreState(config);

    m_geoCacheValid = config.readEntry("geoCacheValid", false);
    m_saveDesktop   = config.readEntry("desktop",       KWindowSystem::currentDesktop());
    m_saveSticky    = config.readEntry("sticky",        false);
    m_saveMaximized = config.readEntry("maximized",     false);
    m_saveMinimized = config.readEntry("minimized",     false);
    m_saveGeometry  = config.readEntry("geometry",      QRect());

    m_restoreShow  = !config.readEntry("hidden",        !showByDefault);
}


void   WidgetPluginBase::restoreState (const KConfigGroup &config)
{
    restoreState(config, true);
}


void WidgetPluginBase::startPlugin()
{
    PluginBase::startPlugin();

    QObject::connect(&m_HideShowAction, SIGNAL(triggered()), m_myself, SLOT(toggleShown()));

    QWidget *w = getWidget();
    if (w) {
        KWindowSystem::setMainWindow(w->window()->windowHandle(), 0);
        m_geoRestoreFlag = true;
        if (!m_restoreShow) {
            m_ignoreHideShow = true;
            w->hide();
            m_ignoreHideShow = false;
            pHideEvent(NULL); // update menu items and notify manager
        } else {
            showOnOrgDesktop();
        }
        m_geoRestoreFlag = false;
        updateHideShowAction(isReallyVisible(getWidget(), true));
    }
}



void WidgetPluginBase::updateHideShowAction(bool show)
{
    QString menuitem;
    if (PluginManager::pluginHasDefaultName(this)) {
        menuitem = description();
    } else {
        menuitem = QString::fromLatin1("%1 (%2)").arg(description(), name());
    }
    if (!show) {
        m_HideShowAction.setText(i18n("Show %1", menuitem));
        m_HideShowAction.setIcon(QIcon::fromTheme("kradio5_show"));
    }
    else {
        m_HideShowAction.setText(i18n("Hide %1", menuitem));
        m_HideShowAction.setIcon(QIcon::fromTheme("kradio5_hide"));
    }
    notifyManager (show);
}
