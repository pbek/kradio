/***************************************************************************
                          alsa-config-mixer-setting.h  -  description
                             -------------------
    begin                : Mon Aug 15 2005
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

#ifndef __KRADIO_ALSA_CONFIG_MIXER_SETTING_H
#define __KRADIO_ALSA_CONFIG_MIXER_SETTING_H

#include <QtCore/QString>

class KConfigGroup;

class AlsaConfigMixerSetting
{
public:
    AlsaConfigMixerSetting();
    AlsaConfigMixerSetting(const KConfigGroup &c, const QString &prefix);
    AlsaConfigMixerSetting(const QString &mixerName, const QString &name, bool use, bool active, float volume);
    ~AlsaConfigMixerSetting();

           QString getIDString() const { return getIDString(m_mixerName, m_name); }
    static QString getIDString(const QString &mixerName, const QString &m_name);

    void saveState(KConfigGroup &c, const QString &prefix) const;

    const QString &mixerName() const { return m_mixerName; }
    const QString &name()      const { return m_name;      }
          bool     use()       const { return m_use;       }
          bool     active()    const { return m_active;    }
          float    volume()    const { return m_volume;    }

protected:
    QString m_mixerName;
    QString m_name;
    bool    m_use;
    bool    m_active;
    float   m_volume;
};

#endif
