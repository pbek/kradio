/***************************************************************************
                          plugin_configuration_dialog.h  -  description
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
#ifndef KRADIO_PLUGIN_CONFIGURATION_DIALOG
#define KRADIO_PLUGIN_CONFIGURATION_DIALOG

#include <kdialogbase.h>
#include "widgetplugins.h"

class PluginConfigurationDialog : public KDialogBase,
                                  public WidgetPluginBase
{
Q_OBJECT
public:
    PluginConfigurationDialog(
                 int dialogFace, const QString &caption,
                 int buttonMask, ButtonCode defaultButton,
                 QWidget *parent=0, const char *name=0,
                 bool modal=true, bool separator=false);

    // PluginBase

	virtual void   saveState (KConfig *) const;
	virtual void   restoreState (KConfig *);

protected :

	virtual ConfigPageInfo  createConfigurationPage();
	virtual AboutPageInfo   createAboutPage();

	// WidgetPluginBase

public slots:
	        void toggleShown() { WidgetPluginBase::pToggleShown(); }
    virtual void show();
    virtual void hide();
    virtual void cancel() { slotCancel(); }

    // QWidget overrides

protected:
	virtual void showEvent(QShowEvent *);
	virtual void hideEvent(QHideEvent *);

	virtual       QWidget *getWidget()         { return this; }
	virtual const QWidget *getWidget() const   { return this; }
};


#endif
