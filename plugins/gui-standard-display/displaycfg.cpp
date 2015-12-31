/***************************************************************************
                          displaycfg.cpp  -  description
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

#include "displaycfg.h"

#include <kcolorbutton.h>
#include <kfontdialog.h>
#include <klocalizedstring.h>

#include <QLayout>
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>

DisplayConfiguration::DisplayConfiguration(QWidget *parent)
    : QWidget (parent),
      m_dirty(true),
      m_ignore_gui_updates(false)
{
    QGridLayout *gl = new QGridLayout (this);

    m_btnActive   = new KColorButton(queryDisplayActiveColor(),   this);
    m_btnInactive = new KColorButton(queryDisplayInactiveColor(), this);
    m_btnBkgnd    = new KColorButton(queryDisplayBkgndColor(),    this);

    QLabel *l1  = new QLabel(i18n("Active text:"),      this);
    QLabel *l2  = new QLabel(i18n("Inactive text:"),    this);
    QLabel *l3  = new QLabel(i18n("Background color:"), this);

    m_fontChooser = new KFontChooser(this, KFontChooser::DisplayFrame, QStringList(), 4);
    m_fontChooser->enableColumn(KFontChooser::SizeList, false);
    m_fontChooser->setFont(queryDisplayFont());
    m_fontChooser->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    gl->addWidget (l1,                   0, 0);
    gl->addWidget (m_btnActive,          0, 1);
    gl->addWidget (l2,                   1, 0);
    gl->addWidget (m_btnInactive,        1, 1);
    gl->addWidget (l3,                   2, 0);
    gl->addWidget (m_btnBkgnd,           2, 1);
    gl->addWidget (m_fontChooser,        3, 0, /*rowspan*/1, /*colspan*/ 2);

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
