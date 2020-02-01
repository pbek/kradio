/***************************************************************************
                          plugin_configuration_dialog.cpp  -  description
                             -------------------
    begin                : Sam Jun 21 2003
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

#include "plugin_configuration_dialog.h"
#include <kconfig.h>
#include <klocalizedstring.h>
#include <QLayout>
#include <QPushButton>

PluginConfigurationDialog::PluginConfigurationDialog(
        const QString         &instanceID,
        const QString         &caption,
        QWidget               *parent,
        const QString         &name
)
: KPageDialog(parent),
  WidgetPluginBase (this, instanceID, name, i18n("Configuration Dialog")),
  m_Caption(caption)
{
    KPageDialog::setFaceType        (KPageDialog::List);
    KPageDialog::setWindowTitle     (m_Caption);
    KPageDialog::setObjectName      (name);
    KPageDialog::setModal           (false);
    KPageDialog::setStandardButtons (QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply);
    
    QObject::connect(this->buttonBox()->button(QDialogButtonBox::Ok),     &QPushButton::clicked,  this, &PluginConfigurationDialog::sigAccept);
    QObject::connect(this->buttonBox()->button(QDialogButtonBox::Apply),  &QPushButton::clicked,  this, &PluginConfigurationDialog::sigAccept);
    QObject::connect(this->buttonBox()->button(QDialogButtonBox::Cancel), &QPushButton::clicked,  this, &PluginConfigurationDialog::sigCancel);
    QObject::connect(this,                                                &QDialog    ::accepted, this, &PluginConfigurationDialog::sigAccept);
    QObject::connect(this,                                                &QDialog    ::rejected, this, &PluginConfigurationDialog::sigCancel);
} // CTOR


// PluginBase

void   PluginConfigurationDialog::saveState (KConfigGroup &c) const
{
//    KConfigGroup selfcfg = c->group("config-dialog-" + WidgetPluginBase::name());
//    WidgetPluginBase::saveState(selfcfg);
    WidgetPluginBase::saveState(c);
}

void   PluginConfigurationDialog::restoreState (const KConfigGroup &c)
{
//    KConfigGroup selfcfg = c->group("config-dialog-" + WidgetPluginBase::name());
//    WidgetPluginBase::restoreState(selfcfg, true);
    WidgetPluginBase::restoreState(c, true);
}


// WidgetPluginBase

void PluginConfigurationDialog::setVisible(bool v)
{
    pSetVisible(v);
    KPageDialog::setVisible(v);
}


// QWidget overrides

void PluginConfigurationDialog::showEvent(QShowEvent *e)
{
    KPageDialog::showEvent(e);
    KPageDialog::setWindowTitle(m_Caption);
    WidgetPluginBase::pShowEvent(e);
}


void PluginConfigurationDialog::hideEvent(QHideEvent *e)
{
    KPageDialog::hideEvent(e);
    WidgetPluginBase::pHideEvent(e);
}
