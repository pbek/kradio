/***************************************************************************
                          alarm.h  -  description
                             -------------------
    begin                : Mon Feb 4 2002
    copyright            : (C) 2002 by Martin Witte / Frank Schwanz
    email                : emw-kradio@nocabal.de / schwanz@fh-brandenburg.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRADIO_ALARM_H
#define KRADIO_ALARM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QtCore/QDateTime>
#include <QtCore/QVector>

#include <kdemacros.h>
#include <kconfig.h>
#include <kconfiggroup.h>

#include "recording_template.h"

/**
  *@author Martin Witte
  */



class KDE_EXPORT Alarm
{
public:

    enum AlarmType { StartPlaying, StopPlaying, StartRecording, StopRecording };

protected:
    QDateTime     m_time;

    bool          m_daily;
    int           m_weekdayMask;

    bool          m_enabled;
    QString       m_stationID;
    float         m_volumePreset;  // < 0: disabled

    AlarmType     m_type;
    recordingTemplate_t       m_recordingTemplate;

    int           m_ID;

    static int    m_LastID;

public:
    Alarm();
    Alarm(const QDateTime &time, bool daily, bool enabled);
    Alarm(const Alarm &);
    ~Alarm();

    bool           isEnabled() const                       { return m_enabled;             }
    bool           isDaily() const                         { return m_daily;               }
    int            weekdayMask() const                     { return m_weekdayMask;         }
    QDateTime      alarmTime () const                      { return m_time;                }
    QDateTime      nextAlarm (bool ignoreEnable = false) const;
    const QString &stationID () const                      { return m_stationID;           }
    float          volumePreset () const                   { return m_volumePreset;        }
    AlarmType      alarmType() const                       { return m_type;                }
    const recordingTemplate_t &recordingTemplate() const   { return m_recordingTemplate;   }

    int            ID() const                              { return m_ID;                  }

    void           setEnabled (bool enable = true)         { m_enabled           = enable; }
    void           setDaily (bool d = true)                { m_daily             = d;      }
    void           setWeekdayMask(int m = 0x7F)            { m_weekdayMask       = m;      }
    void           setDate (const QDate &d)                { m_time.setDate(d);            }
    void           setTime (const QTime &d)                { m_time.setTime(d);            }
    void           setVolumePreset(float v)                { m_volumePreset      = v;      }
    void           setStationID(const QString &id)         { m_stationID         = id;     }
    void           setAlarmType(AlarmType t)               { m_type              = t;      }
    void           setRecordingTemplate(const recordingTemplate_t & t) { m_recordingTemplate = t;  }


    bool  operator == (const Alarm &x) const {
        return
            m_time              == x.m_time &&
            m_daily             == x.m_daily &&
            m_weekdayMask       == x.m_weekdayMask &&
            m_enabled           == x.m_enabled &&
            m_stationID         == x.m_stationID &&
            m_volumePreset      == x.m_volumePreset &&
            m_type              == x.m_type &&
            m_recordingTemplate == x.m_recordingTemplate &&
            m_ID                == x.m_ID;
    }
    bool  operator != (const Alarm &x) const { return ! operator == (x); }

};

//using namespace std;

typedef QVector<Alarm>                 AlarmVector;
typedef AlarmVector::iterator          iAlarmVector;
typedef AlarmVector::const_iterator    ciAlarmVector;



#endif
