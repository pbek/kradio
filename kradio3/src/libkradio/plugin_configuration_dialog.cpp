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
#include <kconfig.h>
#include <klocale.h>

PluginConfigurationDialog::PluginConfigurationDialog(
    int dialogFace, const QString &caption,
    int buttonMask, ButtonCode defaultButton,
    QWidget *parent, const char *name,
    bool modal, bool separator)
: KDialogBase(dialogFace, caption, buttonMask, defaultButton,
              parent, name, modal, separator),
  WidgetPluginBase (name, i18n("Configuration Dialog"))
{
}


// PluginBase

void   PluginConfigurationDialog::saveState (KConfig *c) const
{
    c->setGroup(QString("config-dialog-") + WidgetPluginBase::name());
	WidgetPluginBase::saveState(c);
}

void   PluginConfigurationDialog::restoreState (KConfig *c)
{
    c->setGroup(QString("config-dialog-") + WidgetPluginBase::name());
	WidgetPluginBase::restoreState(c, true);
}


ConfigPageInfo PluginConfigurationDialog::createConfigurationPage()
{
	return ConfigPageInfo();
}


AboutPageInfo  PluginConfigurationDialog::createAboutPage()
{
	return AboutPageInfo();
}


// WidgetPluginBase

void PluginConfigurationDialog::show()
{
	WidgetPluginBase::pShow();
	KDialogBase::show();
}


void PluginConfigurationDialog::hide()
{
	WidgetPluginBase::pHide();
	KDialogBase::hide();
}


// QWidget overrides

void PluginConfigurationDialog::showEvent(QShowEvent *e)
{
	KDialogBase::showEvent(e);
	WidgetPluginBase::pShowEvent(e);
}


void PluginConfigurationDialog::hideEvent(QHideEvent *e)
{
	KDialogBase::hideEvent(e);
	WidgetPluginBase::pHideEvent(e);
}



#include "plugin_configuration_dialog.moc"
