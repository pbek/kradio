/***************************************************************************
                          mprisroot.h  -  description
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

#ifndef MPRISROOT_H
#define MPRISROOT_H

#include <QDBusAbstractAdaptor>
#include <QStringList>

#include "mprisbase.h"

class MPRISSupport;

class MPRISRoot : public QDBusAbstractAdaptor,
                  public MPRISBase
{
Q_OBJECT
Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2")

Q_PROPERTY(bool CanQuit READ CanQuit)
Q_PROPERTY(bool Fullscreen READ Fullscreen WRITE setFullscreen)
Q_PROPERTY(bool CanSetFullscreen READ CanSetFullscreen)
Q_PROPERTY(bool CanRaise READ CanRaise)
Q_PROPERTY(bool HasTrackList READ HasTrackList)
Q_PROPERTY(QString Identity READ Identity)
Q_PROPERTY(QString DesktopEntry READ DesktopEntry)
Q_PROPERTY(QStringList SupportedUriSchemes READ SupportedUriSchemes)
Q_PROPERTY(QStringList SupportedMimeTypes READ SupportedMimeTypes)

public:
    MPRISRoot(MPRISSupport *parent);

public Q_SLOTS:
    void Raise() const;
    void Quit() const;

private:
    bool CanQuit() const;
    bool Fullscreen() const;
    void setFullscreen(bool fullscreen);
    bool CanSetFullscreen() const;
    bool CanRaise() const;
    bool HasTrackList() const;
    QString Identity() const;
    QString DesktopEntry() const;
    QStringList SupportedUriSchemes() const;
    QStringList SupportedMimeTypes() const;

    MPRISSupport *m_support;
};

#endif
