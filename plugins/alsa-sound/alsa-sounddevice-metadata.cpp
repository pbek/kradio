/***************************************************************************
alsa-sounddevice-metadata.h  -  description
-------------------
begin                : Thu June 11 2009
copyright            : (C) 2009 by Martin Witte
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

#include "alsa-sounddevice-metadata.h"
#include "alsa-sound.h"

#include <QtCore/QStringList>

AlsaSoundDeviceMetaData::AlsaSoundDeviceMetaData(const QString &name, const QString &alsa_raw_description)
    : m_pcmDeviceName(name)
{
    IErrorLogClient::staticLogDebug("AlsaSoundDeviceMetaData::AlsaSoundDeviceMetaData: alsa device name: >>" + name + "<<");
    IErrorLogClient::staticLogDebug("AlsaSoundDeviceMetaData::AlsaSoundDeviceMetaData: alsa raw description: >>" + alsa_raw_description + "<<");

    QStringList descrlines = alsa_raw_description.trimmed().split("\n");

    if (descrlines.size() > 1) {
        QStringList devDescrList   = descrlines[0].trimmed().split(",");
        m_cardDescription          = devDescrList[0].trimmed();
        m_deviceDescription        = devDescrList.size() > 1 ? devDescrList[1].trimmed() : QString::null;
        m_deviceVerboseDescription = descrlines[1].trimmed();
    } else {
        m_cardDescription          = QString::null;
        m_deviceDescription        = QString::null;
        m_deviceVerboseDescription = descrlines[0].trimmed();
    }

    m_mixerCardName = AlsaSoundDevice::extractMixerNameFromPCMDevice(name);
}


AlsaMixerMetaData::AlsaMixerMetaData(const AlsaSoundDeviceMetaData &soundDeviceMetaData)
    : m_mixerCardName  (soundDeviceMetaData.mixerCardName())
{
    if (soundDeviceMetaData.cardDescription().length()) {
        m_cardDescription = soundDeviceMetaData.cardDescription();
        m_cardDescription = soundDeviceMetaData.cardDescription();
    } else {
        m_cardDescription = soundDeviceMetaData.deviceVerboseDescription();
    }
}

