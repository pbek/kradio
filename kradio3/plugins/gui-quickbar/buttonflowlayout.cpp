/****************************************************************************
** $Id$
**
** Implementing your own layout: flow example
**
** Copyright (C) 1996 by Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/
/**
   Modified 2002 by Klas Kalass (klas.kalass@gmx.de) for kradio
 */

#include <kdebug.h>

#include "buttonflowlayout.h"

/*********************************************/
/*              Iterator                     */
class ButtonFlowLayoutIterator :public QGLayoutIterator
{
public:
    ButtonFlowLayoutIterator( QPtrList<QLayoutItem> *l ) :idx(0), list(l)  {}
    uint count() const;
    QLayoutItem *current();
    QLayoutItem *next();
    QLayoutItem *takeCurrent();

private:
    int idx;
    QPtrList<QLayoutItem> *list;

};

uint ButtonFlowLayoutIterator::count() const
{
    return list->count();
}

QLayoutItem *ButtonFlowLayoutIterator::current()
{
    return idx < int(count()) ? list->at(idx) : 0;
}

QLayoutItem *ButtonFlowLayoutIterator::next()
{
    idx++; return current();
}

QLayoutItem *ButtonFlowLayoutIterator::takeCurrent()
{
    return idx < int(count()) ? list->take( idx ) : 0;
}

/**************************************************************/

ButtonFlowLayout::ButtonFlowLayout( QWidget *parent, int margin, int spacing,
        const char *name )
  : QLayout( parent, margin, spacing, name ),
  cached_width(0)
{
}

ButtonFlowLayout::ButtonFlowLayout( QLayout* parentLayout, int spacing, const char *name )
  : QLayout( parentLayout, spacing, name ),
  cached_width(0)
{
}

ButtonFlowLayout::ButtonFlowLayout( int spacing, const char *name )
  : QLayout( spacing, name ),
  cached_width(0)
{
}

ButtonFlowLayout::~ButtonFlowLayout()
{
  deleteAllItems();
}


int ButtonFlowLayout::heightForWidth( int w ) const
{
    if ( cached_width != w ) {
        //Not all C++ compilers support "mutable" yet:
        ButtonFlowLayout * mthis = (ButtonFlowLayout*)this;
        int h = mthis->doLayout( QRect(0,0,w,0), TRUE );
        mthis->cached_hfw = h;
        mthis->cached_width = w;
        return h;
    }
    return cached_hfw;
}

void ButtonFlowLayout::addItem( QLayoutItem *item)
{
    list.append( item );
}

bool ButtonFlowLayout::hasHeightForWidth() const
{
    return TRUE;
}

QSize ButtonFlowLayout::sizeHint() const
{
    return minimumSize();
}

QSizePolicy::ExpandData ButtonFlowLayout::expanding() const
{
    return QSizePolicy::NoDirection;
}

QLayoutIterator ButtonFlowLayout::iterator()
{
    return QLayoutIterator( new ButtonFlowLayoutIterator( &list ) );
}

void ButtonFlowLayout::setGeometry( const QRect &r )
{
    QLayout::setGeometry( r );
    doLayout( r );
}

int ButtonFlowLayout::doLayout( const QRect &r, bool testonly )
{
/*    kdDebug() << "buttonflowlayout::doLayout ("
              << r.x()     << "," << r.y()      << ","
              << r.width() << "," << r.height() << ", " << testonly << ")\n";
*/
    float x = r.x();
    float y = r.y();
    int h = 0;        //height of this line so far.
    float buttonWidth = 0;
    int   buttonHeight = 0;
    int linecount = 0;
    int totalWidth  = r.width();
    int totalHeight = r.height();

    QPtrListIterator<QLayoutItem> it(list);
    QLayoutItem *o;

    // get the width of the biggest Button

    it.toFirst();
    while ( (o=it.current()) != 0 ) {
      ++it;
      buttonWidth  = QMAX( buttonWidth,  o->sizeHint().width() );
      buttonHeight = QMAX( buttonHeight, o->sizeHint().height() );
    }

    // calculate the optimal width
    unsigned int columns = (totalWidth + spacing()) /
                           ((int)buttonWidth + spacing());
    if (columns > it.count() ) columns = it.count();
    if (columns == 0) columns = 1; // avoid division by zero


    int rows   = (it.count() - 1) / columns + 1;
    float deltaH = (float)(totalHeight - rows * buttonHeight - (rows - 1) * spacing())
                   / (float)(rows + 1) ;
    if (deltaH < 0) deltaH = 0;

    y += deltaH;

    buttonWidth = (float)(totalWidth - spacing()*(columns-1)) / (float)columns;

/*    fprintf (stderr, "cols = %i      col-width  = %f\n"
                     "rows = %i      row-height = %i\n"
                     "w = %i         h = %i\n",
             columns, buttonWidth,
             rows, buttonHeight,
             totalWidth, totalHeight
             );
*/
    // calculate the positions and sizes
    it.toFirst();
    while ( (o = it.current()) != 0 ) {

//        fprintf (stderr, "x = %i    y = %i\n", x, (int)y);
        ++it;
        int btnRight = (int)rint(x + buttonWidth) - 1,
            btnLeft  = (int)rint(x);

        if ( btnRight > r.right() && h > 0 ) {
            x = r.x();
            btnRight = (int)rint(x + buttonWidth) - 1;
            btnLeft  = (int)rint(x);

            y += h + spacing() + deltaH;
              h = 0;
            linecount++;
        }
        if (!testonly)
              o->setGeometry( QRect( QPoint( btnLeft, (int)rint(y) ),
                                   QSize(  btnRight - btnLeft + 1,
                                           buttonHeight) )
                          );

        x += buttonWidth + spacing();
        h = QMAX( h,  buttonHeight );
    }

    int ret = (int)rint(y + h + deltaH) - r.y();

//    kdDebug() << "ButtonFlowLayout::doLayout() = " << ret << endl;
    return ret;
}


QSize ButtonFlowLayout::minimumSize() const
{
    return minimumSize(geometry().size());
}


QSize ButtonFlowLayout::minimumSize(const QSize &r) const
{
    QSize s(0, 0);

    for (QPtrListIterator<QLayoutItem> it(list); it.current(); ++it) {
        QLayoutItem *o = it.current();
        s = s.expandedTo( o->sizeHint()); //minimumSize() );
    }

    s.setHeight(heightForWidth(r.width()));

    return s;
}
