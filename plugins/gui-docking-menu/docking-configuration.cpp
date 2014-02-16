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
    QGridLayout *layoutClicks  = new QGridLayout();
    QHBoxLayout *layoutWheel   = new QHBoxLayout();

    m_labelClickModeCaption                 = new QLabel(this);
    m_labelClickMode      [Qt::LeftButton]  = new QLabel(this);
    m_labelClickMode      [Qt::RightButton] = new QLabel(this);
    m_labelClickMode      [Qt::MidButton]   = new QLabel(this);
    m_labelClickMode      [Qt::XButton1]    = new QLabel(this);
    m_labelClickMode      [Qt::XButton2]    = new QLabel(this);
    m_labelDoubleClickModeCaption           = new QLabel(this);
    m_labelDoubleClickMode[Qt::LeftButton]  = new QLabel(this);
    m_labelDoubleClickMode[Qt::RightButton] = new QLabel(this);
    m_labelDoubleClickMode[Qt::MidButton]   = new QLabel(this);
    m_labelDoubleClickMode[Qt::XButton1]    = new QLabel(this);
    m_labelDoubleClickMode[Qt::XButton2]    = new QLabel(this);
    m_labelWheelMode                        = new QLabel(this);

    m_comboClickMode      [Qt::LeftButton]  = new QComboBox(this);
    m_comboClickMode      [Qt::RightButton] = new QComboBox(this);
    m_comboClickMode      [Qt::MidButton]   = new QComboBox(this);
    m_comboClickMode      [Qt::XButton1]    = new QComboBox(this);
    m_comboClickMode      [Qt::XButton2]    = new QComboBox(this);
    m_comboDoubleClickMode[Qt::LeftButton]  = new QComboBox(this);
    m_comboDoubleClickMode[Qt::RightButton] = new QComboBox(this);
    m_comboDoubleClickMode[Qt::MidButton]   = new QComboBox(this);
    m_comboDoubleClickMode[Qt::XButton1]    = new QComboBox(this);
    m_comboDoubleClickMode[Qt::XButton2]    = new QComboBox(this);
    m_comboWheelMode                        = new QComboBox(this);


    layoutClicks->addWidget(m_labelClickModeCaption,                 /*row*/ 0, /*col*/ 0, /*rowspan*/ 1, /*colspan*/ 6);
    layoutClicks->addWidget(m_labelClickMode      [Qt::LeftButton],  /*row*/ 1, /*col*/ 0);
    layoutClicks->addWidget(m_comboClickMode      [Qt::LeftButton],  /*row*/ 1, /*col*/ 1);
    layoutClicks->addWidget(m_labelClickMode      [Qt::MidButton],   /*row*/ 1, /*col*/ 2);
    layoutClicks->addWidget(m_comboClickMode      [Qt::MidButton],   /*row*/ 1, /*col*/ 3);
    layoutClicks->addWidget(m_labelClickMode      [Qt::RightButton], /*row*/ 1, /*col*/ 4);
    layoutClicks->addWidget(m_comboClickMode      [Qt::RightButton], /*row*/ 1, /*col*/ 5);
    layoutClicks->addWidget(m_labelClickMode      [Qt::XButton1],    /*row*/ 2, /*col*/ 0);
    layoutClicks->addWidget(m_comboClickMode      [Qt::XButton1],    /*row*/ 2, /*col*/ 1);
    layoutClicks->addWidget(m_labelClickMode      [Qt::XButton2],    /*row*/ 2, /*col*/ 2);
    layoutClicks->addWidget(m_comboClickMode      [Qt::XButton2],    /*row*/ 2, /*col*/ 3);

    layoutClicks->addWidget(m_labelDoubleClickModeCaption,           /*row*/ 3, /*col*/ 0, /*rowspan*/ 1, /*colspan*/ 6);
    layoutClicks->addWidget(m_labelDoubleClickMode[Qt::LeftButton],  /*row*/ 4, /*col*/ 0);
    layoutClicks->addWidget(m_comboDoubleClickMode[Qt::LeftButton],  /*row*/ 4, /*col*/ 1);
    layoutClicks->addWidget(m_labelDoubleClickMode[Qt::MidButton],   /*row*/ 4, /*col*/ 2);
    layoutClicks->addWidget(m_comboDoubleClickMode[Qt::MidButton],   /*row*/ 4, /*col*/ 3);
    layoutClicks->addWidget(m_labelDoubleClickMode[Qt::RightButton], /*row*/ 4, /*col*/ 4);
    layoutClicks->addWidget(m_comboDoubleClickMode[Qt::RightButton], /*row*/ 4, /*col*/ 5);
    layoutClicks->addWidget(m_labelDoubleClickMode[Qt::XButton1],    /*row*/ 5, /*col*/ 0);
    layoutClicks->addWidget(m_comboDoubleClickMode[Qt::XButton1],    /*row*/ 5, /*col*/ 1);
    layoutClicks->addWidget(m_labelDoubleClickMode[Qt::XButton2],    /*row*/ 5, /*col*/ 2);
    layoutClicks->addWidget(m_comboDoubleClickMode[Qt::XButton2],    /*row*/ 5, /*col*/ 3);

    layoutWheel->addWidget(m_labelWheelMode);
    layoutWheel->addWidget(m_comboWheelMode);
    layoutWheel->addItem  (new QSpacerItem( 20, 2, QSizePolicy::Expanding, QSizePolicy::Minimum));


    QHBoxLayout *layout2 = new QHBoxLayout();
    QFrame     *line     = new QFrame(this);
    line->setFrameShape ( QFrame::HLine );
    line->setFrameShadow( QFrame::Sunken );
    layout2->addWidget(line);

    getGridLayout()->addLayout(layout2,                 /*row*/2, /*col*/0, /*rowspan*/1, /*colspan*/3);
    getGridLayout()->addLayout(layoutClicks,            /*row*/3, /*col*/0, /*rowspan*/1, /*colspan*/3);
    getGridLayout()->addLayout(layoutWheel,             /*row*/4, /*col*/0, /*rowspan*/1, /*colspan*/3);

    foreach (QComboBox *combo, m_comboClickMode) {
        connect(combo,  SIGNAL(activated( int )), this, SLOT(slotSetDirty()));
    }
    foreach (QComboBox *combo, m_comboDoubleClickMode) {
        connect(combo,  SIGNAL(activated( int )), this, SLOT(slotSetDirty()));
    }
    connect(m_comboWheelMode,                        SIGNAL(activated( int )), this, SLOT(slotSetDirty()));

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

    m_labelClickModeCaption                 -> setText(i18n( "Mouse click on tray icon:"        ) );
    m_labelClickMode      [Qt::LeftButton]  -> setText(i18nc( "Mouse button", "Left:"     ) );
    m_labelClickMode      [Qt::RightButton] -> setText(i18nc( "Mouse button", "Right:"    ) );
    m_labelClickMode      [Qt::MidButton]   -> setText(i18nc( "Mouse button", "Middle:"   ) );
    m_labelClickMode      [Qt::XButton1]    -> setText(i18nc( "Mouse button", "XButton1:" ) );
    m_labelClickMode      [Qt::XButton2]    -> setText(i18nc( "Mouse button", "XButton2:" ) );

    m_labelDoubleClickModeCaption           -> setText(i18n( "Mouse double-click on tray icon:" ) );
    m_labelDoubleClickMode[Qt::LeftButton]  -> setText(i18nc( "Mouse button", "Left:"     ) );
    m_labelDoubleClickMode[Qt::RightButton] -> setText(i18nc( "Mouse button", "Right:"    ) );
    m_labelDoubleClickMode[Qt::MidButton]   -> setText(i18nc( "Mouse button", "Middle:"   ) );
    m_labelDoubleClickMode[Qt::XButton1]    -> setText(i18nc( "Mouse button", "XButton1:" ) );
    m_labelDoubleClickMode[Qt::XButton2]    -> setText(i18nc( "Mouse button", "XButton2:" ) );

    m_labelWheelMode                        -> setText(i18n( "Mouse wheel action on tray icon:" ) );

    QList<QComboBox*>  allCombos = m_comboClickMode.values();
    allCombos.append(m_comboDoubleClickMode.values());
    foreach (QComboBox *combo, allCombos) {
        combo->clear();
        combo->addItem(i18n("No Action"),               (int)staNone);
        combo->addItem(i18n("Show/Hide Windows"),       (int)staShowHide);
        combo->addItem(i18n("Power On/Off"),            (int)staPowerOnOff);
        combo->addItem(i18n("Pause/Play"),              (int)staPause);
        combo->addItem(i18n("Recording"),               (int)staRecord);
        combo->addItem(i18n("System Tray Menu"),        (int)staSystrayMenu);
        combo->addItem(i18n("GUI-Plugin Menu"),         (int)staGuiPluginsMenu);
        combo->addItem(i18n("Configuration"),           (int)staConfigDialog);
    }

    m_comboWheelMode->addItem(i18n("No Action"),        (int)swaNone);
    m_comboWheelMode->addItem(i18n("Change Station"),   (int)swaChangeStation);
    m_comboWheelMode->addItem(i18n("Change Volume"),    (int)swaChangeVolume);
    m_comboWheelMode->addItem(i18n("Change Frequency"), (int)swaChangeFrequency);


    // features currently not supported by SystemTrayIcon/StatusNotifier mechanisms
    m_labelClickMode      [Qt::XButton1]    ->setEnabled(false);
    m_labelClickMode      [Qt::XButton2]    ->setEnabled(false);
    m_labelDoubleClickMode[Qt::RightButton] ->setEnabled(false);
    m_labelDoubleClickMode[Qt::MidButton]   ->setEnabled(false);
    m_labelDoubleClickMode[Qt::XButton1]    ->setEnabled(false);
    m_labelDoubleClickMode[Qt::XButton2]    ->setEnabled(false);

    m_comboClickMode      [Qt::XButton1]    ->setEnabled(false);
    m_comboClickMode      [Qt::XButton2]    ->setEnabled(false);
    m_comboDoubleClickMode[Qt::RightButton] ->setEnabled(false);
    m_comboDoubleClickMode[Qt::MidButton]   ->setEnabled(false);
    m_comboDoubleClickMode[Qt::XButton1]    ->setEnabled(false);
    m_comboDoubleClickMode[Qt::XButton2]    ->setEnabled(false);

}


void DockingConfiguration::slotOK()
{
    if (m_dirty) {
        StationSelector::slotOK();
        bool old = m_disableGUIUpdates;
        m_disableGUIUpdates = true;
        if (m_docking) {
            foreach (Qt::MouseButton btn, m_comboClickMode.keys()) {
                m_docking->setClickAction      (btn, (SystrayClickAction)m_comboClickMode      [btn]->itemData(m_comboClickMode      [btn]->currentIndex()).value<int>());
            }
            foreach (Qt::MouseButton btn, m_comboDoubleClickMode.keys()) {
                m_docking->setDoubleClickAction(btn, (SystrayClickAction)m_comboDoubleClickMode[btn]->itemData(m_comboDoubleClickMode[btn]->currentIndex()).value<int>());
            }
            m_docking->setWheelAction((SystrayWheelAction)m_comboWheelMode->itemData(m_comboWheelMode->currentIndex()).value<int>());
        }
        m_disableGUIUpdates = old;
        m_dirty = false;
    }
}


void DockingConfiguration::slotCancel()
{
    if (m_dirty) {
        StationSelector::slotCancel();
        if (m_docking) {
            foreach (Qt::MouseButton btn, m_comboClickMode.keys()) {
                m_comboClickMode[btn]      ->setCurrentIndex(m_comboClickMode      [btn]->findData(QVariant((int)m_docking->getClickAction      (btn))));
            }
            foreach (Qt::MouseButton btn, m_comboDoubleClickMode.keys()) {
                m_comboDoubleClickMode[btn]->setCurrentIndex(m_comboDoubleClickMode[btn]->findData(QVariant((int)m_docking->getDoubleClickAction(btn))));
            }
            m_comboWheelMode->setCurrentIndex(m_comboWheelMode->findData(QVariant((int)m_docking->getWheelAction())));
        }
        m_dirty = false;
    }
}


void DockingConfiguration::slotClickActionChanged(Qt::MouseButton btn, SystrayClickAction action)
{
    if (!m_disableGUIUpdates) {
        if (m_docking) {
            m_comboClickMode[btn]->setCurrentIndex(m_comboClickMode[btn]->findData(QVariant((int)action)));
        }
    }
}

void DockingConfiguration::slotDoubleClickActionChanged(Qt::MouseButton btn, SystrayClickAction action)
{
    if (!m_disableGUIUpdates) {
        if (m_docking) {
            m_comboDoubleClickMode[btn]->setCurrentIndex(m_comboDoubleClickMode[btn]->findData(QVariant((int)action)));
        }
    }
}

void DockingConfiguration::slotWheelActionChanged(SystrayWheelAction action)
{
    if (!m_disableGUIUpdates) {
        if (m_docking) {
            m_comboWheelMode->setCurrentIndex(m_comboWheelMode->findData(QVariant((int)action)));
        }
    }
}

void DockingConfiguration::slotSetDirty()
{
    m_dirty = true;
}


#include "docking-configuration.moc"
