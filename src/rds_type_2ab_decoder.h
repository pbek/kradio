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

#ifndef KRADIO_V4LRADIO_RDS_DECODER_2AB_H
#define KRADIO_V4LRADIO_RDS_DECODER_2AB_H

#include <kdemacros.h>

#include "rds_type_decoder.h"

#include <QString>

#define rds_max(a,b)  ((a) > (b) ? (a) : (b))

#define RDS_2AB_POS_BLOCK          RDS_BLK_B
#define RDS_2AB_POS_SHIFT          0
#define RDS_2AB_POS_MASK           0x000F

#define RDS_2AB_AB_FLAG_BLOCK      RDS_BLK_B
#define RDS_2AB_AB_FLAG_MASK       0x0010

#define RDS_RADIOTEXT_A_MAX_LEN    64
#define RDS_RADIOTEXT_B_MAX_LEN    32
#define RDS_RADIOTEXT_MAX_LEN      (rds_max(RDS_RADIOTEXT_A_MAX_LEN, RDS_RADIOTEXT_B_MAX_LEN))

enum RDS_2AB_State { RDS_2AB_WAIT4GRP, RDS_2AB_COMPLETE };

class KDE_EXPORT RDS_Type2AB_Decoder : public RDSTypeDecoder
{
public:
    RDS_Type2AB_Decoder(bool type_A_not_B);

    virtual void addGroup(const RDSGroup &g);
    virtual bool isComplete() const;

    const QString &getRadioText() const;

protected:
    void           addChar(char c);
    void           clear();
    void           complete();

    char           m_radioText[RDS_RADIOTEXT_MAX_LEN+1];
    QString        m_radioTextComplete;

    unsigned int   m_next_expected_position;
    unsigned int   m_max_len;
    bool           m_type_A_not_B;

    RDS_2AB_State  m_state;
    mutable bool   m_workaround_completed;
};




#endif

