/***************************************************************************
                          utils.cpp  -  description
                             -------------------
    begin                : Don Jan 9 2003
    copyright            : (C) 2003 by Martin Witte / Frank Schwanz
    email                : witte@kawo1.rwth-aachen.de / schwanz@fh-brandenburg.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qregexp.h>
#include "utils.h"
#include <linux/soundcard.h>

const char *mixerChannelLabels[] = SOUND_DEVICE_LABELS;
const char *mixerChannelNames[]  = SOUND_DEVICE_NAMES;

QString XMLEscape (const QString &s)
{
	QString c = s;
	c.replace(QRegExp("&"),  "&amp;");
	c.replace(QRegExp("<"),  "&lt;");
	c.replace(QRegExp(">"),  "&gt;");
	c.replace(QRegExp("\""), "&quot;");
	c.replace(QRegExp("'"),  "&apos;");
	return c;
}
