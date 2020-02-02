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
#include <QLayout>

RadioViewConfiguration::RadioViewConfiguration(QWidget *parent)
	: PluginConfigPageBase(parent),
      m_dirty(true)
{
    m_tabWidget = new QTabWidget(this);
    setLayout(new QHBoxLayout());
    layout()->setMargin(0);
    layout()->addWidget(m_tabWidget);
    checkTabBar();
} // CTOR


RadioViewConfiguration::~RadioViewConfiguration()
{
} // DTOR


void RadioViewConfiguration::connectElementTab(PluginConfigPageBase *page)
{
    QObject::connect(this,  &RadioViewConfiguration::sigOK,     page, &PluginConfigPageBase  ::slotOK);
    QObject::connect(this,  &RadioViewConfiguration::sigCancel, page, &PluginConfigPageBase  ::slotCancel);
    QObject::connect(page,  &PluginConfigPageBase  ::sigDirty,  this, &RadioViewConfiguration::slotSetDirty);
    checkTabBar();
} // connectElementTab


int RadioViewConfiguration::addElementTab    (PluginConfigPageBase *page, const QString &label)
{
    int r = m_tabWidget->addTab(page, label);
    connectElementTab(page);
    return r;
} // addElementTab


int RadioViewConfiguration::addElementTab    (PluginConfigPageBase *page, const QIcon &icon, const QString &label)
{
    int r = m_tabWidget->addTab(page, icon, label);
    connectElementTab(page);
    return r;
} // addElementTab


int RadioViewConfiguration::insertElementTab (int index, PluginConfigPageBase *page, const QString &label)
{
    int r = m_tabWidget->insertTab(index, page, label);
    connectElementTab(page);
    return r;
} // insertElementTab


int RadioViewConfiguration::insertElementTab (int index, PluginConfigPageBase *page, const QIcon &icon, const QString &label)
{
    int r = m_tabWidget->insertTab(index, page, icon, label);
    connectElementTab(page);
    return r;
} // insertElementTab


void RadioViewConfiguration::removeElementTab(int index)
{
    PluginConfigPageBase *page = static_cast<PluginConfigPageBase*>(m_tabWidget->widget(index));
    QObject::disconnect(this,  &RadioViewConfiguration::sigOK,     page, &PluginConfigPageBase  ::slotOK);
    QObject::disconnect(this,  &RadioViewConfiguration::sigCancel, page, &PluginConfigPageBase  ::slotCancel);
    QObject::disconnect(page,  &PluginConfigPageBase  ::sigDirty,  this, &RadioViewConfiguration::slotSetDirty);
    m_tabWidget->removeTab(index);
    checkTabBar();
} // removeElementTab



void RadioViewConfiguration::slotOK()
{
    if (m_dirty) {
        Q_EMIT sigOK();
        m_dirty = false;
    }
} // slotOK


void RadioViewConfiguration::slotCancel()
{
    if (m_dirty) {
        Q_EMIT sigCancel();
        m_dirty = false;
    }
} // slotCancel



void RadioViewConfiguration::slotSetDirty()
{
    m_dirty = true;
} // slotSetDirty


void RadioViewConfiguration::checkTabBar()
{
    const bool onePage = m_tabWidget->count() < 2;
    m_tabWidget->setTabBarAutoHide(true);
    m_tabWidget->setDocumentMode(onePage);
} // checkTabBar


