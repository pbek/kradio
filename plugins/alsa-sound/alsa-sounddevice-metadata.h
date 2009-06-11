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

#ifndef _KRADIO_ALSA_SOUNDDEVICE_METADATA_H
#define _KRADIO_ALSA_SOUNDDEVICE_METADATA_H

#include <QtCore/QString>

class AlsaSoundDeviceMetaData
{
public:
    AlsaSoundDeviceMetaData(const QString &name, const QString &alsa_raw_description);
    AlsaSoundDeviceMetaData() {}

    const QString &pcmDeviceName()            const { return m_pcmDeviceName;            }
    const QString &mixerCardName()            const { return m_mixerCardName;            }
    const QString &cardDescription()          const { return m_cardDescription;          }
    const QString &deviceDescription()        const { return m_deviceDescription;        }
    const QString &deviceVerboseDescription() const { return m_deviceVerboseDescription; }

protected:
    QString  m_pcmDeviceName;
    QString  m_mixerCardName;
    QString  m_cardDescription;
    QString  m_deviceDescription;
    QString  m_deviceVerboseDescription;
};


class AlsaMixerMetaData
{
public:
    AlsaMixerMetaData(const AlsaSoundDeviceMetaData &soundDeviceMetaData);
    AlsaMixerMetaData() {}

    const QString &mixerCardName()            const { return m_mixerCardName;            }
    const QString &cardDescription()          const { return m_cardDescription;          }

protected:
    QString  m_mixerCardName;
    QString  m_cardDescription;
};



#endif

