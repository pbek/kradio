/***************************************************************************
                          displaycfg.cpp  -  description
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

#include "displaycfg.h"
#include <kcolorbutton.h>
#include <kcolordialog.h>
#include <qlayout.h>
#include <klocale.h>
#include <qlabel.h>

DisplayConfiguration::DisplayConfiguration(QWidget *parent)
	: QWidget (parent)
{
	QGridLayout *l = new QGridLayout (this, 2, 2, 10);

	m_btnActive = new KColorButton(queryActiveColor(), this);
	m_btnBkgnd  = new KColorButton(queryBkgndColor(),  this);
	QLabel *l1  = new QLabel(i18n("active text"), this);
	QLabel *l2  = new QLabel(i18n("background color"), this);

	l1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
	l2->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
	m_btnActive->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	m_btnBkgnd ->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

	m_btnActive->setMinimumSize(QSize(80, 40));
	m_btnBkgnd ->setMinimumSize(QSize(80, 40));
	
	l->addWidget (l1,                   0, 0, Qt::AlignLeft);
	l->addWidget (m_btnActive,          0, 1);
	l->addWidget (l2,                   1, 0, Qt::AlignLeft);
	l->addWidget (m_btnBkgnd,           1, 1);

}


DisplayConfiguration::~DisplayConfiguration()
{
}


bool DisplayConfiguration::noticeColorsChanged(const QColor &activeColor, const QColor &bkgnd)
{
	m_btnActive->setColor(activeColor);
	m_btnBkgnd->setColor(bkgnd);
	return true;
}


void DisplayConfiguration::slotOK()
{
	sendColors(m_btnActive->color(), m_btnBkgnd->color());
}

void DisplayConfiguration::slotCancel()
{
	m_btnActive->setColor(queryActiveColor());
	m_btnBkgnd->setColor(queryBkgndColor());
}
