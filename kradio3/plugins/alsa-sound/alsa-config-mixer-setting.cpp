/***************************************************************************
                          alsa-config-mixer-setting.cpp  -  description
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

#include "alsa-config-mixer-setting.h"

#include <kconfig.h>

AlsaConfigMixerSetting::AlsaConfigMixerSetting()
 : m_card(-1),
   m_name(QString::null),
   m_use(false),
   m_active(false),
   m_volume(-1)
{
}

AlsaConfigMixerSetting::AlsaConfigMixerSetting(KConfig *c, const QString &prefix)
{
    m_card   = c->readNumEntry      (prefix+"card",   -1);
    m_name   = c->readEntry         (prefix+"name",   QString::null);
    m_use    = c->readBoolEntry     (prefix+"use",    false);
    m_active = c->readBoolEntry     (prefix+"active", false);
    m_volume = c->readDoubleNumEntry(prefix+"volume", 0);
}

AlsaConfigMixerSetting::AlsaConfigMixerSetting(int card, const QString &name, bool use, bool active, float volume)
 : m_card(card),
   m_name(name),
   m_use(use),
   m_active(active),
   m_volume(volume)
{
}

AlsaConfigMixerSetting::~AlsaConfigMixerSetting()
{
}

QString AlsaConfigMixerSetting::getIDString(int card, const QString &name)
{
    return QString::number(card) + "-" + name;
}

void AlsaConfigMixerSetting::saveState(KConfig *c, const QString &prefix) const
{
    c->writeEntry(prefix+"card",   m_card);
    c->writeEntry(prefix+"name",   m_name);
    c->writeEntry(prefix+"use",    m_use);
    c->writeEntry(prefix+"active", m_active);
    c->writeEntry(prefix+"volume", m_volume);
}


