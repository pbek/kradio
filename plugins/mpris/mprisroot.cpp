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
#include <KDE/KProtocolInfo>

MPRIRoot::MPRIRoot(MPRISSupport *parent)
    : QDBusAbstractAdaptor(parent)
    , MPRISBase(this)
    , m_support(parent)
{
}

void MPRIRoot::Raise() const
{
    m_support->showAllWidgetPlugins();
}

void MPRIRoot::Quit() const
{
    qApp->quit();
}

bool MPRIRoot::CanQuit() const
{
    return true;
}

bool MPRIRoot::Fullscreen() const
{
    return false;
}

void MPRIRoot::setFullscreen(bool fullscreen)
{
    Q_UNUSED(fullscreen)
}

bool MPRIRoot::CanSetFullscreen() const
{
    return false;
}

bool MPRIRoot::CanRaise() const
{
    return true;
}

bool MPRIRoot::HasTrackList() const
{
    return false;
}

QString MPRIRoot::Identity() const
{
    return KCmdLineArgs::aboutData()->programName();
}

QString MPRIRoot::DesktopEntry() const
{
    return "kde4-" + KCmdLineArgs::aboutData()->appName();
}

QStringList MPRIRoot::SupportedUriSchemes() const
{
    return QStringList();
}

QStringList MPRIRoot::SupportedMimeTypes() const
{
    return QStringList();
}

#include <mprisroot.moc>
