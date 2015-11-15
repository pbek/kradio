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

#include "rds_group_v4l.h"

RDSGroupV4L::RDSGroupV4L()
{
    clear();
}


RDSGroupV4L::~RDSGroupV4L()
{
}


void RDSGroupV4L::clear()
{
    RDSGroup::clear();
    m_state               = RDS_GRP_V4L_WAIT4BLOCK;
    m_next_expected_block = RDS_BLK_A;
}


bool RDSGroupV4L::isComplete() const
{
    return m_state == RDS_GRP_V4L_COMPLETE;
}


int RDSGroupV4L::addRawData(const unsigned char *_rawdata, int size)
{
    int nblocks = size/3;
    for (int ib = 0; ib < nblocks; ++ib) {
        const unsigned char *rawdata = _rawdata + 3*ib;
        bool error_uncorrecable = (rawdata[2] & RDS_V4L_ERROR_UNCORRECTABLE) != 0;
        bool error_corrected    = (rawdata[2] & RDS_V4L_ERROR_CORRECTED    ) != 0;
        statsAccountBlockError(1, error_uncorrecable || error_corrected);
        if (!error_uncorrecable) {
            int blk_nr = (rawdata[2] & RDS_V4L_BLOCKNR_MASK) >> RDS_V4L_BLOCKNR_SHIFT;

            if (blk_nr == RDS_BLK_C_) {
                blk_nr = RDS_BLK_C;
            }

            if (blk_nr != m_next_expected_block) {
                // accounting that _previous_ group is broken
                statsAccountGroupError(1, 1);
                clear();
            }

            // reset if we start a new group
            if (blk_nr == RDS_BLK_A) {
                clear();
            }

            if (blk_nr == m_next_expected_block) {

                // add new data
                m_rawdata[blk_nr] = (rawdata[0] & 0xFF) | ((rawdata[1] << 8) & 0xFF00);

                ++m_next_expected_block;

                // are we finished?
                if (blk_nr == RDS_BLK_D) {
                    // accounting that _current_ group is OK
                    statsAccountGroupError(1, 0);
                    m_next_expected_block = RDS_BLK_A;
                    m_state               = RDS_GRP_V4L_COMPLETE;
                    decode();
                }
            } else {
                // accounting that _current_ group is broken
                statsAccountGroupError(1, 1);
            }
        }
        else {
            clear();
        }
    }
    return nblocks * 3;
}
