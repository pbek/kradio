/***************************************************************************
                          recording_template.h  -  description
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

#ifndef KRADIO_RECORDING_TEMPLATE_H
#define KRADIO_RECORDING_TEMPLATE_H

#include <QString>
#include <QDate>
#include <QTime>

#include <kconfig.h>
#include <kconfiggroup.h>

#include "radiostation.h"

/**
  *@author Martin Witte
  */

struct recordingTemplate_t
{
    QString  filename;
    QString  id3Title;
    QString  id3Artist;
    QString  id3Genre;

    recordingTemplate_t()
    {
        // keep all strings empty. required in recording to distinguish
        // uninitialized templates (recording started e.g. by GUI button)
        // from initialized (recording started by alarm)
    }

    recordingTemplate_t(const QString &pFilename,
                        const QString &pId3title,
                        const QString &pId3Artist,
                        const QString &pId3Genre = "")
      : filename(pFilename),
        id3Title(pId3title),
        id3Artist(pId3Artist),
        id3Genre (pId3Genre)
    {
    }

    inline
    bool operator == (const recordingTemplate_t &x) const
    {
        return filename  == x.filename  &&
               id3Title  == x.id3Title  &&
               id3Artist == x.id3Artist &&
               id3Genre  == x.id3Genre;
    }
    
    inline
    bool operator != (const recordingTemplate_t &x) const {
        return !operator==(x);
    }

    void restoreState (const QString &prefix, const KConfigGroup &config, const QString &compatFNameField)
    {
        QString defaultFName    = "kradio-recording-%s-%Y.%m.%d-%H.%M.%S";
        QString compatFilename  = compatFNameField.length()
                                     ? config.readEntry(compatFNameField, defaultFName)
                                     : defaultFName;
        filename  = config.readEntry(prefix + "_filename",  compatFilename);
        id3Title  = config.readEntry(prefix + "_id3Title",  i18n("%s, %m/%d/%Y, %H:%M:%S, kradio5 recording"));
        id3Artist = config.readEntry(prefix + "_id3Artist", "%s");
        id3Genre  = config.readEntry(prefix + "_id3Genre",  "");
    }

    void saveState    (const QString &prefix, KConfigGroup &config) const
    {
        config.writeEntry (prefix + "_filename",  filename);
        config.writeEntry (prefix + "_id3Title",  id3Title);
        config.writeEntry (prefix + "_id3Artist", id3Artist);
        config.writeEntry (prefix + "_id3Genre",  id3Genre);
    }


    void realizeTemplateParameters(const RadioStation *rs, int stationIdx)
    {
        QDate               date        = QDate::currentDate();
        QTime               time        = QTime::currentTime();
        QString             stationName = rs ? rs->name(): "unknown";
        stationName.replace(QRegExp("[/*?]"), "_");

        if (!filename.length()) {
            filename = "kradio-recording-%s-%Y.%m.%d-%H.%M.%S";
        }

        realizeTemplateParameters(filename,  date, time, stationName, stationIdx);
        realizeTemplateParameters(id3Title,  date, time, stationName, stationIdx);
        realizeTemplateParameters(id3Artist, date, time, stationName, stationIdx);
        realizeTemplateParameters(id3Genre,  date, time, stationName, stationIdx);
    }

    
    void realizeTemplateParameters(QString &x, const QDate &date, const QTime &time, const QString &stationName, int stationIdx)
    {
        x.replace(QString("%s"), stationName);
        x.replace(QString("%i"), QString::number(stationIdx));
        x.replace(QString("%Y"), QString().sprintf("%04d", date.year()));
        x.replace(QString("%m"), QString().sprintf("%02d", date.month()));
        x.replace(QString("%d"), QString().sprintf("%02d", date.day()));
        x.replace(QString("%H"), QString().sprintf("%02d", time.hour()));
        x.replace(QString("%M"), QString().sprintf("%02d", time.minute()));
        x.replace(QString("%S"), QString().sprintf("%02d", time.second()));
        x.replace(QString("%A"), QDate::longDayName(date.dayOfWeek()));
        x.replace(QString("%B"), QDate::longMonthName(date.month()));

        x.replace("%%", "%");
    }

};

#endif
