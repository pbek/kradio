/***************************************************************************
                       oss-sound-configuration.h  -  description
                             -------------------
    begin                : Thu Sep 30 2004
    copyright            : (C) 2004 by Martin Witte
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

#ifndef KRADIO_OSS_SOUND_CONFIGURATION_H
#define KRADIO_OSS_SOUND_CONFIGURATION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "oss-sound-configuration-ui.h"
#include "oss-sound.h"

class OSSSoundConfiguration : public OSSSoundConfigurationUI
{
Q_OBJECT
public :
    OSSSoundConfiguration (QWidget *parent, OSSSoundDevice *);
    ~OSSSoundConfiguration ();

protected slots:

    void slotOK();
    void slotCancel();
    void slotSetDirty();

    void slotUpdateConfig();

protected:

    OSSSoundDevice *m_SoundDevice;

    bool m_dirty;
    bool m_ignore_gui_updates;
};

#endif
