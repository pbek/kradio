/***************************************************************************
                          mprisbase.cpp  -  description
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

#include "mprisbase.h"

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtCore/QMetaClassInfo>
#include <QtCore/QStringList>

MPRISBase::MPRISBase(QObject *adaptor)
    : m_adaptor(adaptor)
{
    Q_ASSERT(m_adaptor);
}

void MPRISBase::signalPropertiesChange(const QVariantMap& properties)
{
    QDBusMessage msg = QDBusMessage::createSignal("/org/mpris/MediaPlayer2",
        "org.freedesktop.DBus.Properties", "PropertiesChanged");

    QVariantList args;
    args << m_adaptor->metaObject()->classInfo(0).value();
    args << properties;
    args << QStringList();

    msg.setArguments(args);

    QDBusConnection::sessionBus().send(msg);
}
