/***************************************************************************
                          plugin_configuration_dialog.cpp  -  description
                             -------------------
    begin                : Sam Jun 21 2003
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

#include "plugin_configuration_dialog.h"

PluginConfigurationDialog::PluginConfigurationDialog(
    int dialogFace, const QString &caption,
    int buttonMask, ButtonCode defaultButton,
    QWidget *parent, const char *name,
    bool modal, bool separator)
: KDialogBase(dialogFace, caption, buttonMask, defaultButton,
              parent, name, modal, separator),
  WidgetPluginBase (name)
{
}


// PluginBase

void   PluginConfigurationDialog::saveState (KConfig *) const
{
}

void   PluginConfigurationDialog::restoreState (KConfig *)
{
}


void PluginConfigurationDialog::createConfigurationPage()
{
}


void PluginConfigurationDialog::createAboutPage()
{
}


// WidgetPluginBase

void PluginConfigurationDialog::PluginConfigurationDialog::show()
{
	KDialogBase::show();
}


void PluginConfigurationDialog::PluginConfigurationDialog::show(bool on)
{
	on ? show() : hide();
}


void PluginConfigurationDialog::PluginConfigurationDialog::hide()
{
	KDialogBase::hide();
}


// QWidget overrides

void PluginConfigurationDialog::showEvent(QShowEvent *)
{
	notifyManager (true);
}


void PluginConfigurationDialog::hideEvent(QHideEvent *)
{
	notifyManager (false);
}
