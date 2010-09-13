/***************************************************************************
                          docking-configuration.cpp  -  description
                             -------------------
    begin                : Son Aug 3 2003
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

#include "docking-configuration.h"

#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QFrame>

#include <klocale.h>

using namespace std;

DockingConfiguration::DockingConfiguration (RadioDocking *docking, QWidget *parent)
    : StationSelector(parent),
      m_docking(docking),
      m_disableGUIUpdates(false)
{
    QHBoxLayout *layout  = new QHBoxLayout();
    QHBoxLayout *layout2 = new QHBoxLayout();

    m_labelClickMode = new QLabel(this);
    layout->addWidget(m_labelClickMode);

    m_comboClickMode = new QComboBox(this);
    layout->addWidget(m_comboClickMode);

    QSpacerItem *spacer = new QSpacerItem( 20, 2, QSizePolicy::Expanding, QSizePolicy::Minimum);
    layout->addItem(spacer);

    QFrame *line = new QFrame(this);
    line->setFrameShape ( QFrame::HLine );
    line->setFrameShadow( QFrame::Sunken );
    layout2->addWidget(line);

    getGridLayout()->addLayout(layout2, /*row*/2, /*col*/0, /*rowspan*/1, /*colspan*/3);
    getGridLayout()->addLayout(layout,  /*row*/3, /*col*/0, /*rowspan*/1, /*colspan*/3);

    connect(m_comboClickMode, SIGNAL(activated( int )), this, SLOT(slotSetDirty()));

    languageChange();
    m_dirty=true;
    slotCancel();
}


DockingConfiguration::~DockingConfiguration ()
{
}


void DockingConfiguration::languageChange()
{
    StationSelector::languageChange();
    m_labelClickMode->setText(i18n( "Left Mouse Click on Tray" ) );

    m_comboClickMode->clear();
    m_comboClickMode->addItem(i18n("Show/Hide all GUI Elements"));
    m_comboClickMode->addItem(i18n("Power On/Off"));
}

void DockingConfiguration::slotOK()
{
    if (m_dirty) {
        StationSelector::slotOK();
        bool old = m_disableGUIUpdates;
        m_disableGUIUpdates = true;
        if (m_docking)
            m_docking->setLeftClickAction((LeftClickAction)m_comboClickMode->currentIndex());
        m_disableGUIUpdates = old;
        m_dirty = false;
    }
}

void DockingConfiguration::slotCancel()
{
    if (m_dirty) {
        StationSelector::slotCancel();
        if (m_docking)
            m_comboClickMode->setCurrentIndex(m_docking->getLeftClickAction());
        m_dirty = false;
    }
}

void DockingConfiguration::slotLeftClickActionChanged(LeftClickAction action)
{
    if (!m_disableGUIUpdates) {
        if (m_docking)
            m_comboClickMode->setCurrentIndex(action);
    }
}

void DockingConfiguration::slotSetDirty()
{
    m_dirty = true;
}


#include "docking-configuration.moc"
