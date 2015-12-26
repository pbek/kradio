/***************************************************************************
                          mprisroot.cpp  -  description
                             -------------------
    copyright            : (C) 2014 by Pino Toscano
    email                : toscano.pino@tiscali.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mprisroot.h"
#include "mprissupport.h"

#include <QApplication>
#include <KDE/KAboutData>
#include <KDE/KCmdLineArgs>

MPRISRoot::MPRISRoot(MPRISSupport *parent)
    : QDBusAbstractAdaptor(parent)
    , MPRISBase(this)
    , m_support(parent)
{
}

void MPRISRoot::Raise() const
{
    m_support->showAllWidgetPlugins();
}

void MPRISRoot::Quit() const
{
    qApp->quit();
}

bool MPRISRoot::CanQuit() const
{
    return true;
}

bool MPRISRoot::Fullscreen() const
{
    return false;
}

void MPRISRoot::setFullscreen(bool fullscreen)
{
    Q_UNUSED(fullscreen)
}

bool MPRISRoot::CanSetFullscreen() const
{
    return false;
}

bool MPRISRoot::CanRaise() const
{
    return true;
}

bool MPRISRoot::HasTrackList() const
{
    return false;
}

QString MPRISRoot::Identity() const
{
    return KCmdLineArgs::aboutData()->programName();
}

QString MPRISRoot::DesktopEntry() const
{
    return "kde4-" + KCmdLineArgs::aboutData()->appName();
}

QStringList MPRISRoot::SupportedUriSchemes() const
{
    return QStringList();
}

QStringList MPRISRoot::SupportedMimeTypes() const
{
    return QStringList();
}

#include <mprisroot.moc>
