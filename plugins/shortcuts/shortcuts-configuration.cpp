/***************************************************************************
                          shortcuts-configuration.cpp  -  description
                             -------------------
    begin                : Mon Feb 4 2002
    copyright            : (C) 2002 by Martin Witte / Frank Schwanz
    email                : emw-kradio@nocabal.de / schwanz@fh-brandenburg.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "shortcuts-configuration.h"
#include <QLayout>

ShortcutsConfiguration::ShortcutsConfiguration()
    : PluginConfigPageBase(nullptr)
{
    m_shortCutsEditor = new KShortcutsEditor(this);
    setLayout(new QHBoxLayout());
    layout()->setMargin(0);
    layout()->addWidget(m_shortCutsEditor);
} // CTOR


ShortcutsConfiguration::~ShortcutsConfiguration()
{
}


void ShortcutsConfiguration::slotOK()
{
    m_shortCutsEditor->save();
    m_shortCutsEditor->commit();
} // slotOK


void ShortcutsConfiguration::slotCancel()
{
    m_shortCutsEditor->undoChanges();
} // slotCancel

