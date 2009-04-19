/***************************************************************************
                       lirc-configuration.h  -  description
                             -------------------
    begin                : Sat May 21 2005
    copyright            : (C) 2005 by Martin Witte
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

#ifndef KRADIO_LIRC_CONFIGURATION_H
#define KRADIO_LIRC_CONFIGURATION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ui_lirc-configuration-ui.h"
#include "lircsupport.h"
#include "listviewitem_lirc.h"

class LIRCConfiguration : public QWidget,
                          public Ui_LIRCConfigurationUI
{
Q_OBJECT
public :
    LIRCConfiguration (QWidget *parent, LircSupport *);
    ~LIRCConfiguration ();

protected slots:

    void slotOK();
    void slotCancel();
    void slotSetDirty();

    void slotUpdateConfig();
    void slotRawLIRCSignal(const QString &val, int repeat_counter, bool &consumed);

    void slotRenamingStarted(ListViewItemLirc *, int);
    void slotRenamingStopped(ListViewItemLirc *, int);

    void readLIRCConfigurationFile();

protected:


    void addKey(const QString &descr, const QString &key, const QString &alt_key);

    LircSupport                 *m_LIRC;

    QMap<int, LIRC_Actions>      m_order;
    QMap<LIRC_Actions, QString>  m_descriptions;

    bool                         m_dirty;
    bool                         m_ignore_gui_updates;
};

#endif
