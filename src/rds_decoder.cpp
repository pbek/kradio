/***************************************************************************
                          rds_decoder.cpp  -  description
                             -------------------
    begin                : Sun Feb 15 2009
    copyright            : (C) 2009 by Ernst Martin Witte
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "rds_decoder.h"

RDSDecoder::RDSDecoder(RDSGroup *rds_group_decoder)
 : m_group_decoder(rds_group_decoder)
{
    for (int i = 0; i < GROUP_TYPE_COUNT; ++i) {
        m_type_decoders[i] = NULL;
    }
    m_type_decoders[GROUP_0A] = new RDS_Type0A_Decoder();
    m_type_decoders[GROUP_2A] = new RDS_Type2AB_Decoder(/*type_A_not_B = */true);
    m_type_decoders[GROUP_2B] = new RDS_Type2AB_Decoder(/*type_A_not_B = */false);
}

RDSDecoder::~RDSDecoder()
{
    for (int i = 0; i < GROUP_TYPE_COUNT; ++i) {
        if (m_type_decoders[i]) {
            delete m_type_decoders[i];
            m_type_decoders[i] = NULL;
        }
    }
    if (m_group_decoder) {
        delete m_group_decoder;
        m_group_decoder = NULL;
    }
}

void RDSDecoder::addRawData(unsigned char *rawdata, int n)
{
    if (m_group_decoder) {
        m_group_decoder->addRawData(rawdata, n);
        if (m_group_decoder->isComplete()) {
            unsigned int type = m_group_decoder->getGroupType();
            if (type < GROUP_TYPE_COUNT && m_type_decoders[type] != NULL) {
                m_type_decoders[type]->addGroup(*m_group_decoder);
    /*            if (m_type_decoders->isComplete()) {
                    // do something
                }*/
            }
        }
    }
}


const RDS_Type0A_Decoder *RDSDecoder::getStationNameDecoder() const
{
    return static_cast<const RDS_Type0A_Decoder*>(m_type_decoders[GROUP_0A]);
}

const RDS_Type2AB_Decoder *RDSDecoder::getRadioTextADecoder() const
{
    return static_cast<const RDS_Type2AB_Decoder*>(m_type_decoders[GROUP_2A]);
}

const RDS_Type2AB_Decoder *RDSDecoder::getRadioTextBDecoder() const
{
    return static_cast<const RDS_Type2AB_Decoder*>(m_type_decoders[GROUP_2B]);
}


double RDSDecoder::statsBlockErrorRate() const
{
    return m_group_decoder ? m_group_decoder->statsBlockErrorRate() : 0.0;
}

double RDSDecoder::statsGroupErrorRate() const
{
    return m_group_decoder ? m_group_decoder->statsGroupErrorRate() : 0.0;
}
