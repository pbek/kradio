/***************************************************************************
                          rds_decoder.h  -  description
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

#ifndef KRADIO_V4LRADIO_RDS_DECODER_H
#define KRADIO_V4LRADIO_RDS_DECODER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kdemacros.h>

#include "rds_group.h"
#include "rds_type_0a_decoder.h"
#include "rds_type_2ab_decoder.h"

class KDE_EXPORT RDSDecoder
{
public:
    RDSDecoder(RDSGroup *rds_group_decoder);
    ~RDSDecoder();

    void addRawData(unsigned char *rawdata, int n);

    const RDS_Type0A_Decoder  *getStationNameDecoder() const;
    const RDS_Type2AB_Decoder *getRadioTextADecoder () const;
    const RDS_Type2AB_Decoder *getRadioTextBDecoder () const;

    double statsBlockErrorRate() const;
    double statsGroupErrorRate() const;

protected:

    RDSGroup       *m_group_decoder;
    RDSTypeDecoder *m_type_decoders[GROUP_TYPE_COUNT];
};

#endif

