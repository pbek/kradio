/***************************************************************************
                          radioview-configuration.cpp  -  description
                             -------------------
    begin                : Fr Aug 15 2003
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

#include "radioview-configuration.h"

RadioViewConfiguration::RadioViewConfiguration(QWidget *parent)
	: QTabWidget (parent),
      m_dirty(true)
{
    checkTabBar();
}


RadioViewConfiguration::~RadioViewConfiguration()
{
}


void RadioViewConfiguration::connectElementTab(QWidget *page)
{
    QObject::connect(this,  SIGNAL(sigOK()),     page, SLOT(slotOK()));
    QObject::connect(this,  SIGNAL(sigCancel()), page, SLOT(slotCancel()));
    QObject::connect(page,  SIGNAL(sigDirty()),  this, SLOT(slotSetDirty()));
    checkTabBar();
} // connectElementTab


int RadioViewConfiguration::addElementTab    (QWidget *page, const QString &label)
{
    int r = QTabWidget::addTab(page, label);
    connectElementTab(page);
    return r;
}


int RadioViewConfiguration::addElementTab    (QWidget *page, const QIcon &icon, const QString &label)
{
    int r = QTabWidget::addTab(page, icon, label);
    connectElementTab(page);
    return r;
}


int RadioViewConfiguration::insertElementTab (int index, QWidget *page, const QString &label)
{
    int r = QTabWidget::insertTab(index, page, label);
    connectElementTab(page);
    return r;
}


int RadioViewConfiguration::insertElementTab (int index, QWidget *page, const QIcon &icon, const QString &label)
{
    int r = QTabWidget::insertTab(index, page, icon, label);
    connectElementTab(page);
    return r;
}


void RadioViewConfiguration::removeElementTab(int index)
{
    QWidget *w = widget(index);
    QObject::disconnect(this,  SIGNAL(sigOK()),     w,    SLOT(slotOK()));
    QObject::disconnect(this,  SIGNAL(sigCancel()), w,    SLOT(slotCancel()));
    QObject::disconnect(w,     SIGNAL(sigDirty()),  this, SLOT(slotSetDirty()));
    QTabWidget::removeTab(index);
    checkTabBar();
}



void RadioViewConfiguration::slotOK()
{
    if (m_dirty) {
        Q_EMIT sigOK();
        m_dirty = false;
    }
}

void RadioViewConfiguration::slotCancel()
{
    if (m_dirty) {
        Q_EMIT sigCancel();
        m_dirty = false;
    }
}

void RadioViewConfiguration::slotSetDirty()
{
    m_dirty = true;
}


void RadioViewConfiguration::checkTabBar()
{
    const bool onePage = count() < 2;
    setTabBarAutoHide(true);
    setDocumentMode(onePage);
}


