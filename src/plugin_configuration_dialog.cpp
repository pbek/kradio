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

PluginConfigurationDialog::PluginConfigurationDialog(
        const QString         &instanceID,
        KPageDialog::FaceType  dialogFace,
        const QString         &caption,
        KDialog::ButtonCodes   buttonMask,
        KDialog::ButtonCode    defaultButton,
        QWidget               *parent,
        const QString         &name,
        bool                   modal,
        bool                   separator
)
: KPageDialog(),
  WidgetPluginBase (this, instanceID, name, i18n("Configuration Dialog")),
  m_Caption(caption)
{
    KPageDialog::setFaceType        (dialogFace);
    KPageDialog::setCaption         (m_Caption);
    KPageDialog::setParent          (parent);
    KPageDialog::setObjectName      (name);
    KPageDialog::setModal           (modal);
    KPageDialog::setButtons         (buttonMask);
    KPageDialog::setDefaultButton   (defaultButton);
    KPageDialog::showButtonSeparator(separator);
}


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


// AboutPageInfo  PluginConfigurationDialog::createAboutPage()
// {
//     return AboutPageInfo();
// }


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
    KPageDialog::setCaption(m_Caption);
    WidgetPluginBase::pShowEvent(e);
}


void PluginConfigurationDialog::hideEvent(QHideEvent *e)
{
    KPageDialog::hideEvent(e);
    WidgetPluginBase::pHideEvent(e);
}

void PluginConfigurationDialog::noticePluginRenamed(PluginBase *p, const QString &name)
{
    WidgetPluginBase::noticePluginRenamed(p, name);
    updateGeometry();
/*    QLayout *l = layout();
    if (l) {
        l->invalidate();
    }*/
}

#include "plugin_configuration_dialog.moc"
