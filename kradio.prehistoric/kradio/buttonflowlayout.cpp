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
#include <math.h>
#include "buttonflowlayout.h"

/*********************************************/
/*              Iterator                     */
class ButtonFlowLayoutIterator :public QGLayoutIterator
{
public:
    ButtonFlowLayoutIterator( QList<QLayoutItem> *l ) :idx(0), list(l)  {}
    uint count() const;
    QLayoutItem *current();
    QLayoutItem *next();
    QLayoutItem *takeCurrent();

private:
    int idx;
    QList<QLayoutItem> *list;

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
  minHeight(100),
  cached_width(0)
{
}

ButtonFlowLayout::ButtonFlowLayout( QLayout* parentLayout, int spacing, const char *name )
  : QLayout( parentLayout, spacing, name ),
  minHeight(100),
  cached_width(0)
{
}

ButtonFlowLayout::ButtonFlowLayout( int spacing, const char *name )
  : QLayout( spacing, name ),
  minHeight(100),
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
    int x = r.x();
    float y = r.y();
    int h = 0;		//height of this line so far.
    int buttonWidth = 0;
    int buttonHeight = 0;
    int linecount = 0;
    int totalWidth  = r.right()  - r.left();
    int totalHeight = r.bottom() - r.top();

    QListIterator<QLayoutItem> it(list);
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
                           (buttonWidth + spacing());
    if (columns > it.count() ) columns=it.count();
    if (columns == 0) columns = 1; // avoid division by zero

    
    int rows   = (it.count() - 1) / columns + 1;
    float deltaH = (float)(totalHeight - rows * buttonHeight - (rows - 1) * spacing()) /
		           (float)(rows + 1) ;
    y += deltaH;
    minHeight = rows * buttonHeight + (rows - 1) * spacing();

    buttonWidth = (totalWidth - spacing()*(columns-1)) / columns;


/*    fprintf (stderr, "cols = %i      col-width  = %i\n"
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

//		fprintf (stderr, "x = %i    y = %i\n", x, (int)y);
		++it;
        if ( x + buttonWidth > r.right() && h > 0 ) {
		    x = r.x();
    	    y += h + spacing() + deltaH;
  	        h = 0;
	        linecount++;
    	}
        if (!testonly)
      	    o->setGeometry( QRect( QPoint( x, (int)y ),
			                       QSize(buttonWidth, buttonHeight) ) );

        x += buttonWidth + spacing();
        h = QMAX( h,  buttonHeight );
    }

    return (int)y + h - r.y();
}


QSize ButtonFlowLayout::minimumSize() const
{
    QSize s(0,0);
    QListIterator<QLayoutItem> it(list);
    QLayoutItem *o;
    while ( (o=it.current()) != 0 ) {
  	  ++it;
    	s = s.expandedTo( o->minimumSize() );
    }

    s.setHeight(minHeight);
    return s;
}
