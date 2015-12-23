/***************************************************************************
                          rds_decoder_0a.h  -  description
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

#ifndef KRADIO_V4LRADIO_RDS_DECODER_0A_H
#define KRADIO_V4LRADIO_RDS_DECODER_0A_H

#include <kdemacros.h>

#include "rds_type_decoder.h"

#include <QString>

#define RDS_0A_POS_BLOCK          RDS_BLK_B
#define RDS_0A_POS_SHIFT          0
#define RDS_0A_POS_MASK           0x0003

#define RDS_STATIONNAME_MAX_LEN   8

enum RDS_0A_State { RDS_0A_WAIT4GRP, RDS_0A_COMPLETE };

class KDE_EXPORT RDS_Type0A_Decoder : public RDSTypeDecoder
{
public:
    RDS_Type0A_Decoder();

    virtual void addGroup(const RDSGroup &g);
    virtual bool isComplete() const;

    const QString &getStationName() const;

protected:
    void           addChar(unsigned char c);
    void           clear();

    char         m_stationName[RDS_STATIONNAME_MAX_LEN+1];
    QString      m_stationNameComplete;

    unsigned int m_next_expected_position;

    RDS_0A_State m_state;
};




#endif

