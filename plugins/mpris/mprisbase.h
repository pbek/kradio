/***************************************************************************
                          mprisbase.h  -  description
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

#ifndef MPRISBASE_H
#define MPRISBASE_H

#include <QObject>
#include <QVariantMap>

class MPRISBase
{
protected:
    MPRISBase(QObject *adaptor);

    void signalPropertiesChange(const QVariantMap& properties);

private:
    QObject *m_adaptor;
};

#endif
