/***************************************************************************
                          displaycfg.h  -  description
                             -------------------
    begin                : Fr Aug 15 2003
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

#ifndef KRADIO_DISPLAYCFG_H
#define KRADIO_DISPLAYCFG_H

#include "displaycfg_interfaces.h"
#include "pluginbase_config_page.h"

class KColorButton;
class NoSizeFontRequester;

class DisplayConfiguration : public PluginConfigPageBase,
                             public IDisplayCfgClient
{
Q_OBJECT
public:
    DisplayConfiguration(QWidget *parent);
    ~DisplayConfiguration();

// Interface

    bool connectI (Interface *i)    override { return IDisplayCfgClient::connectI(i); }
    bool disconnectI (Interface *i) override { return IDisplayCfgClient::disconnectI(i); }

// IDisplayCfgClient

RECEIVERS:
    bool noticeDisplayColorsChanged(const QColor &activeColor, const QColor &inactiveColor, const QColor &bkgnd) override;
    bool noticeDisplayFontChanged(const QFont &f) override;


public slots:

    virtual void slotOK()     override;
    virtual void slotCancel() override;
    void slotSetDirty();

protected:
    KColorButton *m_btnActive;
    KColorButton *m_btnInactive;
    KColorButton *m_btnBkgnd;
    NoSizeFontRequester *m_fontChooser;

    bool          m_dirty;
    bool          m_ignore_gui_updates;
};


#endif
