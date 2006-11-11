/****************************************************************************
** $Id$
**
** Definition of simple flow layout for custom layout example
**
** Created : 979899
**
** Copyright (C) 1997 by Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/
/**
   Modified 2002 by Klas Kalass (klas.kalass@gmx.de) for kradio
 */
#ifndef BUTTONFLOWLAYOUT_H
#define BUTTONFLOWLAYOUT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../../src/include/utils.h"

#include <qlayout.h>
#include <qptrlist.h>

class ButtonFlowLayout : public QLayout
{
public:
    ButtonFlowLayout( QWidget *parent, int margin = 0, int spacing=-1,
                      const char *name=0 );

    ButtonFlowLayout( QLayout* parentLayout, int spacing=-1, const char *name=0 );

    ButtonFlowLayout( int spacing=-1, const char *name=0 );

    ~ButtonFlowLayout();

    void addItem( QLayoutItem *item);
    bool hasHeightForWidth() const;
    int heightForWidth( int ) const;
    QSize sizeHint() const;
    QSize minimumSize() const;
    QSize minimumSize(const QSize &r) const;  // minimumSize is dependent from width
    QLayoutIterator iterator();
    QSizePolicy::ExpandData expanding() const;

protected:
    void setGeometry( const QRect& );

private:
    int doLayout( const QRect&, bool testonly = FALSE );
    QPtrList<QLayoutItem> list;
    int cached_width;
    int cached_hfw;
};

#endif
