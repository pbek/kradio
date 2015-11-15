/***************************************************************************
                          docking-configuration.h  -  description
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

#ifndef KRADIO_DOCKING_CONFIGURATION_H
#define KRADIO_DOCKING_CONFIGURATION_H

#include <QtCore/QMap>

#include "stationselector.h"
#include "docking.h"

class QComboBox;
class QLabel;

class DockingConfiguration : public StationSelector
{
Q_OBJECT
public :
    DockingConfiguration (RadioDocking *docking, QWidget *parent);
    ~DockingConfiguration ();

protected slots:

    void slotOK();
    void slotCancel();
    void slotSetDirty();

    void slotClickActionChanged      (Qt::MouseButton btn, SystrayClickAction action);
    void slotDoubleClickActionChanged(Qt::MouseButton btn, SystrayClickAction action);
    void slotWheelActionChanged      (SystrayWheelAction action);
    void languageChange();

protected:
    RadioDocking                       *m_docking;

    QMap<Qt::MouseButton, QComboBox*>   m_comboClickMode;
    QMap<Qt::MouseButton, QComboBox*>   m_comboDoubleClickMode;
    QComboBox                          *m_comboWheelMode;

    QLabel                             *m_labelClickModeCaption;
    QLabel                             *m_labelDoubleClickModeCaption;
    QMap<Qt::MouseButton, QLabel*>      m_labelClickMode;
    QMap<Qt::MouseButton, QLabel*>      m_labelDoubleClickMode;
    QLabel                             *m_labelWheelMode;

    bool                                m_disableGUIUpdates;

};

#endif
