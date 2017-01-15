/***************************************************************************
                          alsa-config-mixer-setting.cpp  -  description
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

#include "alsa-config-mixer-setting.h"

#include <kconfiggroup.h>

AlsaConfigMixerSetting::AlsaConfigMixerSetting()
 : m_mixerName(),
   m_name(),
   m_use(false),
   m_active(false),
   m_volume(-1)
{
}

AlsaConfigMixerSetting::AlsaConfigMixerSetting(const KConfigGroup &c, const QString &prefix)
{
    m_mixerName = c.readEntry(prefix + "mixerName",   "default");
    m_name      = c.readEntry(prefix + "name",        QString());
    m_use       = c.readEntry(prefix + "use",         false);
    m_active    = c.readEntry(prefix + "active",      false);
    m_volume    = c.readEntry(prefix + "volume",      0.0);
}

AlsaConfigMixerSetting::AlsaConfigMixerSetting(const QString &mixerName, const QString &name, bool use, bool active, float volume)
 : m_mixerName(mixerName),
   m_name(name),
   m_use(use),
   m_active(active),
   m_volume(volume)
{
}

AlsaConfigMixerSetting::~AlsaConfigMixerSetting()
{
}

QString AlsaConfigMixerSetting::getIDString(const QString &mixerName, const QString &name)
{
    return mixerName + "-" + name;
}

void AlsaConfigMixerSetting::saveState(KConfigGroup &c, const QString &prefix) const
{
    c.writeEntry(prefix + "mixerName",   m_mixerName);
    c.writeEntry(prefix + "name",        m_name);
    c.writeEntry(prefix + "use",         m_use);
    c.writeEntry(prefix + "active",      m_active);
    c.writeEntry(prefix + "volume",      m_volume);
}


