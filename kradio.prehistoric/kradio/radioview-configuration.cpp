/***************************************************************************
                          radioview-configuration.cpp  -  description
                             -------------------
    begin                : Fr Aug 15 2003
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

#include "radioview-configuration.h"
 
RadioViewConfiguration::RadioViewConfiguration(QWidget *parent)
	: QTabWidget (parent)
{
}

RadioViewConfiguration::~RadioViewConfiguration()
{
}


void RadioViewConfiguration::addTab    (QWidget *child, const QString &label)
{
	QTabWidget::addTab(child, label);
	QObject::connect(this, SIGNAL(sigOK()),     child, SLOT(slotOK()));
	QObject::connect(this, SIGNAL(sigCancel()), child, SLOT(slotCancel()));
}


void RadioViewConfiguration::addTab    (QWidget *child, const QIconSet &iconset, const QString &label)
{
	QTabWidget::addTab(child, iconset, label);
	QObject::connect(this, SIGNAL(sigOK()),     child, SLOT(slotOK()));
	QObject::connect(this, SIGNAL(sigCancel()), child, SLOT(slotCancel()));
}


void RadioViewConfiguration::addTab    (QWidget *child, QTab *tab)
{
	QTabWidget::addTab(child, tab);
	QObject::connect(this, SIGNAL(sigOK()),     child, SLOT(slotOK()));
	QObject::connect(this, SIGNAL(sigCancel()), child, SLOT(slotCancel()));
}


void RadioViewConfiguration::insertTab (QWidget *child, const QString &label, int index)
{
	QTabWidget::insertTab(child, label, index);
	QObject::connect(this, SIGNAL(sigOK()),     child, SLOT(slotOK()));
	QObject::connect(this, SIGNAL(sigCancel()), child, SLOT(slotCancel()));
}


void RadioViewConfiguration::insertTab (QWidget *child, const QIconSet &iconset, const QString &label, int index)
{
	QTabWidget::insertTab(child, iconset, label, index);
	QObject::connect(this, SIGNAL(sigOK()),     child, SLOT(slotOK()));
	QObject::connect(this, SIGNAL(sigCancel()), child, SLOT(slotCancel()));
}


void RadioViewConfiguration::insertTab (QWidget *child, QTab *tab, int index)
{
	QTabWidget::insertTab(child, tab, index);
	QObject::connect(this, SIGNAL(sigOK()),     child, SLOT(slotOK()));
	QObject::connect(this, SIGNAL(sigCancel()), child, SLOT(slotCancel()));
}


void RadioViewConfiguration::removePage(QWidget *w)
{
	QObject::disconnect(this, SIGNAL(sigOK()),     w, SLOT(slotOK()));
	QObject::disconnect(this, SIGNAL(sigCancel()), w, SLOT(slotCancel()));
	QTabWidget::removePage(w);
}


void RadioViewConfiguration::slotOK()
{
	emit sigOK();
}

void RadioViewConfiguration::slotCancel()
{
	emit sigCancel();
}

