/***************************************************************************
                          rds_decoder_2ab.cpp  -  description
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

#include "rds_type_2ab_decoder.h"

RDS_Type2AB_Decoder::RDS_Type2AB_Decoder(bool type_A_not_B)
    : m_type_A_not_B(type_A_not_B),
      m_workaround_completed(false)
{
    m_max_len = m_type_A_not_B ? RDS_RADIOTEXT_A_MAX_LEN : RDS_RADIOTEXT_B_MAX_LEN;
    clear();
}

void RDS_Type2AB_Decoder::addGroup(const RDSGroup &g)
{
    if (((g.getGroupType() != GROUP_2A) &&  m_type_A_not_B) ||
        ((g.getGroupType() != GROUP_2B) && !m_type_A_not_B))
    {
        return;
    }

    unsigned short C = g.getBlock(RDS_BLK_C);
    unsigned short D = g.getBlock(RDS_BLK_D);

    bool         ab_flag  = (g.getBlock(RDS_2AB_AB_FLAG_BLOCK) & RDS_2AB_AB_FLAG_MASK) != 0;
    unsigned int position = (m_type_A_not_B ? 4 : 2) * ((g.getBlock(RDS_2AB_POS_BLOCK) & RDS_2AB_POS_MASK) >> RDS_2AB_POS_SHIFT);

    if (position == 0) {
        // workaround: it seems that some radio stations do not transmit \r as message delimiter
        if (m_next_expected_position >= 8) {
            complete();
        }

        clear();
    }

    if (position == m_next_expected_position) {

        if (m_type_A_not_B) {
            addChar((C & 0xFF00) >> 8);
            addChar((C & 0x00FF) >> 0);
        }
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

bool RDS_Type2AB_Decoder::isComplete() const
{
    return m_workaround_completed || (m_state == RDS_2AB_COMPLETE);
}

const QString &RDS_Type2AB_Decoder::getRadioText() const
{
    m_workaround_completed = false;
    return m_radioTextComplete;
}

void RDS_Type2AB_Decoder::addChar(char c)
{
    if (m_state != RDS_2AB_COMPLETE) {
        if (c == '\r') {
            c = 0;
        }
        m_radioText[m_next_expected_position++] = c;
        if (!c || (m_next_expected_position >= m_max_len)) {
            complete();
        }
    }
}



void RDS_Type2AB_Decoder::complete()
{
    m_radioText[m_next_expected_position] = 0;
    m_state                = RDS_2AB_COMPLETE;
    m_radioTextComplete    = QString(m_radioText).trimmed();
    m_workaround_completed = true;
}

void RDS_Type2AB_Decoder::clear()
{
    for (int i = 0; i < RDS_RADIOTEXT_MAX_LEN+1; ++i) {
        m_radioText[i] = 0;
    }
    m_radioText[m_max_len]   = 0;
    m_next_expected_position = 0;
    m_state                  = RDS_2AB_WAIT4GRP;
}


