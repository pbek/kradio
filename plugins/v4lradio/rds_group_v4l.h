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

#ifndef KRADIO_V4LRADIO_RDS_GROUP_V4L_H
#define KRADIO_V4LRADIO_RDS_GROUP_V4L_H

#include "rds_group.h"

#define RDS_V4L_ERROR_UNCORRECTABLE 0x80
#define RDS_V4L_ERROR_CORRECTED     0x40
#define RDS_V4L_BLOCKNR_MASK        0x07
#define RDS_V4L_BLOCKNR_SHIFT       0

enum RDSV4LGroupState { RDS_GRP_V4L_WAIT4BLOCK, RDS_GRP_V4L_COMPLETE };

class RDSGroupV4L : public RDSGroup
{
public:
    RDSGroupV4L();
    virtual ~RDSGroupV4L();

    virtual int  addRawData(const unsigned char *rawdata, int size);
    virtual bool isComplete() const;
    virtual void clear();

protected:
    RDSV4LGroupState  m_state;
    int               m_next_expected_block;
};




#endif

