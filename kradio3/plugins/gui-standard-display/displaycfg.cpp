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
#include <kfontdialog.h>

#include <qlayout.h>
#include <klocale.h>
#include <qlabel.h>
#include <qbuttongroup.h>

DisplayConfiguration::DisplayConfiguration(QWidget *parent)
    : QWidget (parent),
      m_dirty(true),
      m_ignore_gui_updates(false)
{
    QGroupBox *bg = new QGroupBox(i18n("Display Colors"), this);
    bg->setColumnLayout(0, Qt::Vertical );
    bg->layout()->setSpacing( 8 );
    bg->layout()->setMargin( 12 );
    QGridLayout *gl = new QGridLayout (bg->layout());

    m_btnActive   = new KColorButton(queryDisplayActiveColor(), bg);
    m_btnInactive = new KColorButton(queryDisplayInactiveColor(), bg);
    m_btnBkgnd    = new KColorButton(queryDisplayBkgndColor(), bg);

    connect(m_btnActive,   SIGNAL(changed(const QColor &)), this, SLOT(slotSetDirty()));
    connect(m_btnInactive, SIGNAL(changed(const QColor &)), this, SLOT(slotSetDirty()));
    connect(m_btnBkgnd,    SIGNAL(changed(const QColor &)), this, SLOT(slotSetDirty()));

    QLabel *l1  = new QLabel(i18n("Active Text"), bg);
    QLabel *l2  = new QLabel(i18n("Inactive Text"), bg);
    QLabel *l3  = new QLabel(i18n("Background Color"), bg);

    l1->setAlignment(QLabel::AlignCenter);
    l2->setAlignment(QLabel::AlignCenter);
    l3->setAlignment(QLabel::AlignCenter);

    l1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    l2->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    l3->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    m_btnActive  ->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    m_btnInactive->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    m_btnBkgnd   ->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    m_btnActive  ->setMinimumSize(QSize(40, 40));
    m_btnInactive->setMinimumSize(QSize(40, 40));
    m_btnBkgnd   ->setMinimumSize(QSize(40, 40));

    gl->addWidget (l1,                   0, 0, Qt::AlignCenter);
    gl->addWidget (l2,                   0, 1, Qt::AlignCenter);
    gl->addWidget (l3,                   0, 2, Qt::AlignCenter);
    gl->addWidget (m_btnActive,          1, 0);
    gl->addWidget (m_btnInactive,        1, 1);
    gl->addWidget (m_btnBkgnd,           1, 2);

    m_fontChooser = new KFontChooser(this, NULL, false, QStringList(), true, 4);
    m_fontChooser->setFont(queryDisplayFont());
    m_fontChooser->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    QVBoxLayout  *l = new QVBoxLayout(this, 10);
    l->addWidget(bg);
    l->addWidget(m_fontChooser);

    connect(m_btnActive,   SIGNAL(changed(const QColor &)),     this, SLOT(slotSetDirty()));
    connect(m_btnInactive, SIGNAL(changed(const QColor &)),     this, SLOT(slotSetDirty()));
    connect(m_btnBkgnd,    SIGNAL(changed(const QColor &)),     this, SLOT(slotSetDirty()));
    connect(m_fontChooser, SIGNAL(fontSelected(const QFont &)), this, SLOT(slotSetDirty()));

}


DisplayConfiguration::~DisplayConfiguration()
{
}


bool DisplayConfiguration::noticeDisplayColorsChanged(const QColor &activeColor, const QColor &inactiveColor, const QColor &bkgnd)
{
    m_ignore_gui_updates = true;
    m_btnActive->setColor(activeColor);
    m_btnInactive->setColor(inactiveColor);
    m_btnBkgnd->setColor(bkgnd);
    m_ignore_gui_updates = false;
    return true;
}


bool DisplayConfiguration::noticeDisplayFontChanged(const QFont &f)
{
    m_ignore_gui_updates = true;
    m_fontChooser->setFont(f);
    m_ignore_gui_updates = false;
    return true;
}


void DisplayConfiguration::slotOK()
{
    if (m_dirty) {
        sendDisplayColors(m_btnActive->color(), m_btnInactive->color(), m_btnBkgnd->color());
        sendDisplayFont(m_fontChooser->font());
        m_dirty = false;
    }
}

void DisplayConfiguration::slotCancel()
{
    if (m_dirty) {
        m_ignore_gui_updates = true;
        m_btnActive  ->setColor(queryDisplayActiveColor());
        m_btnInactive->setColor(queryDisplayInactiveColor());
        m_btnBkgnd   ->setColor(queryDisplayBkgndColor());
        m_fontChooser->setFont(queryDisplayFont());
        m_dirty = false;
        m_ignore_gui_updates = false;
    }
}

void DisplayConfiguration::slotSetDirty()
{
    if (!m_dirty && !m_ignore_gui_updates) {
        m_dirty = true;
        emit sigDirty();
    }
}


#include "displaycfg.moc"
