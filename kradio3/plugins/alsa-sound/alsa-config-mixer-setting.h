/***************************************************************************
                          alsa-config-mixer-setting.h  -  description
                             -------------------
    begin                : Mon Aug 15 2005
    copyright            : (C) 2005 by Martin Witte
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

#ifndef __KRADIO_ALSA_CONFIG_MIXER_SETTING_H
#define __KRADIO_ALSA_CONFIG_MIXER_SETTING_H

#include <qstring.h>

class KConfig;

class AlsaConfigMixerSetting
{
public:
    AlsaConfigMixerSetting();
    AlsaConfigMixerSetting(KConfig *c, const QString &prefix);
    AlsaConfigMixerSetting(int card, const QString &name, bool use, bool active, float volume);
    ~AlsaConfigMixerSetting();

    QString getIDString() const { return getIDString(m_card, m_name); }
    static QString getIDString(int card, const QString &m_name);

    void saveState(KConfig *c, const QString &prefix) const;

    int     m_card;
    QString m_name;
    bool    m_use;
    bool    m_active;
    float   m_volume;
};

#endif
