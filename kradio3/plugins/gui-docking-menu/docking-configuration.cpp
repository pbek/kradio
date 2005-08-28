/***************************************************************************
                          docking-configuration.cpp  -  description
                             -------------------
    begin                : Son Aug 3 2003
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

#include "docking-configuration.h"

#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qframe.h>

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

    StationSelectorUILayout->expand(2,0);
    StationSelectorUILayout->addMultiCellLayout(layout2, 2, 2, 0, 2);
    StationSelectorUILayout->addMultiCellLayout(layout,  3, 3, 0, 2);

    languageChange();
    slotCancel();
}


DockingConfiguration::~DockingConfiguration ()
{
}


void DockingConfiguration::languageChange()
{
    StationSelector::languageChange();
    m_labelClickMode->setText( i18n( "Left Mouse Click on Tray" ) );

    m_comboClickMode->clear();
    m_comboClickMode->insertItem(i18n("Show/Hide all GUI Elements"));
    m_comboClickMode->insertItem(i18n("Power On/Off"));
}

void DockingConfiguration::slotOK()
{
    StationSelector::slotOK();
    bool old = m_disableGUIUpdates;
    m_disableGUIUpdates = true;
    if (m_docking)
        m_docking->setLeftClickAction((LeftClickAction)m_comboClickMode->currentItem());
    m_disableGUIUpdates = old;
}

void DockingConfiguration::slotCancel()
{
    StationSelector::slotCancel();
    if (m_docking)
        m_comboClickMode->setCurrentItem(m_docking->getLeftClickAction());
}

void DockingConfiguration::slotLeftClickActionChanged(LeftClickAction action)
{
    if (!m_disableGUIUpdates) {
        if (m_docking)
            m_comboClickMode->setCurrentItem(action);
    }
}

#include "docking-configuration.moc"
