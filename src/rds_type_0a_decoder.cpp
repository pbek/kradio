/***************************************************************************
                          rds_decoder_0a.cpp  -  description
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

#include "rds_type_0a_decoder.h"

RDS_Type0A_Decoder::RDS_Type0A_Decoder()
{
    clear();
}

void RDS_Type0A_Decoder::addGroup(const RDSGroup &g)
{
    if (g.getGroupType() != GROUP_0A) {
        return;
    }

    unsigned short D = g.getBlock(RDS_BLK_D);

    unsigned int position = 2 * ((g.getBlock(RDS_0A_POS_BLOCK) & RDS_0A_POS_MASK) >> RDS_0A_POS_SHIFT);

    if (position == 0) {
        clear();
    }

    if (position == m_next_expected_position) {
        addChar((D & 0xFF00) >> 8);
        addChar((D & 0x00FF) >> 0);
        if (isComplete()) {
            m_next_expected_position = 0;
        }
    }
    else {
        clear();
    }
}

bool RDS_Type0A_Decoder::isComplete() const
{
    return m_state == RDS_0A_COMPLETE;
}

const QString &RDS_Type0A_Decoder::getStationName() const
{
    return m_stationNameComplete;
}


void RDS_Type0A_Decoder::addChar(unsigned char c)
{
    if (m_state != RDS_0A_COMPLETE) {
        if (c == '\r') {
            c = 0;
        }
        m_stationName[m_next_expected_position++] = c;
        if (!c || (m_next_expected_position >= RDS_STATIONNAME_MAX_LEN)) {
            m_stationName[m_next_expected_position] = 0;
            m_state                  = RDS_0A_COMPLETE;
            m_stationNameComplete    = QString(m_stationName).trimmed();
        }
    }
}

void RDS_Type0A_Decoder::clear()
{
    for (int i = 0; i < RDS_STATIONNAME_MAX_LEN+1; ++i) {
        m_stationName[i] = 0;
    }
    m_next_expected_position               = 0;
    m_state                                = RDS_0A_WAIT4GRP;
}


