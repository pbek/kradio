/***************************************************************************
                          plugin_configuration_dialog.h  -  description
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
#ifndef KRADIO_PLUGIN_CONFIGURATION_DIALOG
#define KRADIO_PLUGIN_CONFIGURATION_DIALOG

#include <kpagedialog.h>
#include "widgetpluginbase.h"


#ifdef KRADIO_ENABLE_FIXMES
    #warning "FIXME: should we switch to KConfigDialog????"
#endif
class PluginConfigurationDialog : public KPageDialog,
                                  public WidgetPluginBase
{

Q_OBJECT

public:
    PluginConfigurationDialog(
        const QString         &instanceID,
        const QString         &caption,
        QWidget               *parent    = 0,
        const QString         &name      = QString()
    );

    // PluginBase

    virtual QString pluginClassName() const { return QString::fromLatin1("PluginConfigurationDialog"); }

    virtual void   saveState    (      KConfigGroup &) const;
    virtual void   restoreState (const KConfigGroup &);
    virtual void   restoreState (const KConfigGroup &c, bool b) { WidgetPluginBase::restoreState(c,b); }



protected :

    // WidgetPluginBase

public slots:
    virtual void toggleShown() { WidgetPluginBase::pToggleShown(); }
    virtual void cancel() { reject(); }

    // QWidget overrides
public:
    virtual void setVisible(bool v);

protected:
    virtual void showEvent(QShowEvent *);
    virtual void hideEvent(QHideEvent *);

//     virtual       QWidget *getWidget()         { return this; }
//     virtual const QWidget *getWidget() const   { return this; }

    QString m_Caption;
};


#endif
