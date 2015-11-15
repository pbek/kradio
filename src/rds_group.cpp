/***************************************************************************
                          rds.cpp  -  description
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

#include "rds_group.h"

#define STATS_MAX_GROUP_COUNT    2000
#define STATS_MAX_BLOCK_COUNT    2000

RDSGroup::RDSGroup()
 :  m_stats_group_count(0),
    m_stats_group_errors(0),
    m_stats_block_count(0),
    m_stats_block_errors(0)
{
    clear();
}

RDSGroup::~RDSGroup()
{
}

void RDSGroup::clear()
{
    for (int i = 0; i < RDS_BLK_MAXCOUNT; ++i) {
        m_rawdata[i] = 0;
    }

    m_pi_code    = 0;
    m_group_code = 0;
    m_pty        = 0;
    m_tp         = 0;
}


void RDSGroup::decode()
{
    m_pi_code    = (m_rawdata[RDS_PI_BLOCK ] & RDS_PI_MASK ) >> RDS_PI_SHIFT;
    m_group_code = (m_rawdata[RDS_GRP_BLOCK] & RDS_GRP_MASK) >> RDS_GRP_SHIFT;
    m_pty        = (m_rawdata[RDS_PTY_BLOCK] & RDS_PTY_MASK) >> RDS_PTY_SHIFT;
    m_tp         = (m_rawdata[RDS_TP_BLOCK ] & RDS_TP_MASK ) >> RDS_TP_SHIFT;
}

void RDSGroup::statsAccountGroupError(int nGroups, int nErrors)
{
    if (m_stats_group_count > STATS_MAX_GROUP_COUNT) {
        m_stats_group_count  = 0;
        m_stats_group_errors = 0;
    }
    m_stats_group_count  += nGroups;
    m_stats_group_errors += nErrors;
}

void RDSGroup::statsAccountBlockError(int nBlocks, int nErrors)
{
    if (m_stats_block_count > STATS_MAX_BLOCK_COUNT) {
        m_stats_block_count  = 0;
        m_stats_block_errors = 0;
    }
    m_stats_block_count  += nBlocks;
    m_stats_block_errors += nErrors;
}

double RDSGroup::statsGroupErrorRate() const
{
    if (m_stats_group_count > 20) {
        return  (double)m_stats_group_errors / (double)m_stats_group_count;
    } else {
        return 0.0;
    }
}

double RDSGroup::statsBlockErrorRate() const
{
    if (m_stats_block_count > 20) {
        return  (double)m_stats_block_errors / (double)m_stats_block_count;
    } else {
        return 0.0;
    }
}
