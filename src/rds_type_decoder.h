/***************************************************************************
                          rds_type_decoder.h  -  description
                             -------------------
    begin                : Feb 2009
    copyright            : (C) 2009 Ernst Martin Witte
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

#ifndef KRADIO_V4LRADIO_RDS_TYPE_DECODER_H
#define KRADIO_V4LRADIO_RDS_TYPE_DECODER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kdemacros.h>

#include "rds_group.h"

class KDE_EXPORT RDSTypeDecoder
{
public:
    RDSTypeDecoder();
    virtual ~RDSTypeDecoder();
    virtual void addGroup(const RDSGroup &g) = 0;
    virtual bool isComplete() const = 0;
};

#endif

