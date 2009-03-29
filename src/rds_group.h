/***************************************************************************
                          rds.h  -  description
                             -------------------
    begin                : Feb 2009
    copyright            : (C) 2009 Ernst Martin Witte, Klas Kalass
    email                : emw-kradio@nocabal.de, klas@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRADIO_V4LRADIO_RDS_GROUP_H
#define KRADIO_V4LRADIO_RDS_GROUP_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kdemacros.h>

#define RDS_BLK_A          0
#define RDS_BLK_B          1
#define RDS_BLK_C          2
#define RDS_BLK_D          3
#define RDS_BLK_C_         4
#define RDS_BLK_E          5
#define RDS_BLK_E_         6
#define RDS_BLK_RESERVED   7
#define RDS_BLK_MAXCOUNT   8

#define RDS_PI_BLOCK       RDS_BLK_A
#define RDS_PI_SHIFT       0
#define RDS_PI_MASK        0xFFFF

#define RDS_GRP_BLOCK      RDS_BLK_B
#define RDS_GRP_SHIFT      11
#define RDS_GRP_MASK       0xF800

#define RDS_TP_BLOCK       RDS_BLK_B
#define RDS_TP_SHIFT       10
#define RDS_TP_MASK        0x0400

#define RDS_PTY_BLOCK      RDS_BLK_B
#define RDS_PTY_SHIFT      5
#define RDS_PTY_MASK       0x03E0

#define RDS_0A_POS_BLOCK   RDS_BLK_B
#define RDS_0A_POS_SHIFT   0
#define RDS_0A_POS_MASK    0x0003


enum RDSGroupType {
    GROUP_0A = 0, GROUP_0B,
    GROUP_1A,     GROUP_1B,
    GROUP_2A,     GROUP_2B,
    GROUP_3A,     GROUP_3B,
    GROUP_4A,     GROUP_4B,
    GROUP_5A,     GROUP_5B,
    GROUP_6A,     GROUP_6B,
    GROUP_7A,     GROUP_7B,
    GROUP_8A,     GROUP_8B,
    GROUP_9A,     GROUP_9B,
    GROUP_10A,    GROUP_10B,
    GROUP_11A,    GROUP_11B,
    GROUP_12A,    GROUP_12B,
    GROUP_13A,    GROUP_13B,
    GROUP_14A,    GROUP_14B,
    GROUP_15A,    GROUP_15B,
    GROUP_TYPE_COUNT
};

class KDE_EXPORT RDSGroup
{
public:
    RDSGroup();
    virtual ~RDSGroup();

    virtual int  addRawData(const unsigned char *rawdata, int size) = 0;
    virtual void clear();
    virtual bool isComplete() const = 0;

    int  getPI ()       const { return m_pi_code; }
    int  getTP ()       const { return m_tp; }
    int  getPTY()       const { return m_pty; }
    int  getGroupType() const { return m_group_code; }

    unsigned short getBlock(unsigned int block_nr)
                        const { return (block_nr <= RDS_BLK_D) ? m_rawdata[block_nr] : 0; }

    double statsBlockErrorRate() const;
    double statsGroupErrorRate() const;

protected:
    void   statsAccountGroupError(int nGroups, int nErrors);
    void   statsAccountBlockError(int nGroups, int nErrors);
    void   decode();

    unsigned short m_rawdata[RDS_BLK_MAXCOUNT];

    int            m_pty;
    int            m_group_code;
    bool           m_tp;
    int            m_pi_code;

    int            m_stats_group_count;
    int            m_stats_group_errors;
    int            m_stats_block_count;
    int            m_stats_block_errors;
};




#endif

