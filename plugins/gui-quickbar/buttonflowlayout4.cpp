/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the example classes of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

/** extended by Martin Witte for kradio4 **/


#include "buttonflowlayout4.h"
#include <math.h>

ButtonFlowLayout4::ButtonFlowLayout4(QWidget *parent, int margin, int spacing)
    : QLayout(parent)
{
    setMargin(margin);
    setSpacing(spacing);
}

ButtonFlowLayout4::ButtonFlowLayout4(int spacing)
{
    setSpacing(spacing);
}

ButtonFlowLayout4::~ButtonFlowLayout4()
{
    QLayoutItem *item;
    while ((item = takeAt(0)))
        delete item;
}

void ButtonFlowLayout4::addItem(QLayoutItem *item)
{
    m_itemList.append(item);
}

int ButtonFlowLayout4::count() const
{
    return m_itemList.size();
}

QLayoutItem *ButtonFlowLayout4::itemAt(int index) const
{
    return (index >= 0 && index < count()) ? m_itemList.value(index) : NULL;
}

QLayoutItem *ButtonFlowLayout4::takeAt(int index)
{
    if (index >= 0 && index < m_itemList.size())
        return m_itemList.takeAt(index);
    else
        return 0;
}

Qt::Orientations ButtonFlowLayout4::expandingDirections() const
{
    return 0;
}

bool ButtonFlowLayout4::hasHeightForWidth() const
{
    return true;
}

int ButtonFlowLayout4::heightForWidth(int width) const
{
    if ( m_cached_width != width ) {
        int height = doLayout(QRect(0, 0, width, 0), true);

        //Not all C++ compilers support "mutable" yet:
        ButtonFlowLayout4 * mthis = const_cast<ButtonFlowLayout4*>(this);

        mthis->m_cached_hfw   = height;
        mthis->m_cached_width = width;
    }
    return m_cached_hfw;
}

void ButtonFlowLayout4::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

QSize ButtonFlowLayout4::sizeHint() const
{
    return minimumSize();
}

QSize ButtonFlowLayout4::minimumSize() const
{
    return minimumSize(geometry().size());
}

QSize ButtonFlowLayout4::minimumSize(const QSize &s) const
{
    QSize size;
    QLayoutItem *item;
    foreach (item, m_itemList) {
        size = size.expandedTo(item->sizeHint());
    }

    size.setHeight(heightForWidth(s.width()));;
    return size;
}

int ButtonFlowLayout4::doLayout(const QRect &r, bool testonly) const
{
    float x            = r.x();
    float y            = r.y();
    int   h            = 0;        //height of this line so far.
    float buttonWidth  = 0;
    int   buttonHeight = 0;
    int   linecount    = 0;
    int   totalWidth   = r.width();
    int   totalHeight  = r.height();

    QList<QLayoutItem*>::const_iterator  it = m_itemList.begin();

    // get the width of the biggest Button
    QLayoutItem *item;
    foreach(item, m_itemList) {
        buttonWidth  = qMax(buttonWidth,  (float)item->sizeHint().width());
        buttonHeight = qMax(buttonHeight, item->sizeHint().height());
    }

    // calculate the optimal width
    int bw_plus_sp = (int)buttonWidth + spacing();
    int columns = bw_plus_sp ? (totalWidth + spacing()) / bw_plus_sp : 1;
    if (columns > m_itemList.size()) {
        columns = m_itemList.size();
    }
    if (columns <= 0) {
        columns = 1; // avoid division by zero
    }

    int   rows   = (m_itemList.size() - 1) / columns + 1;
    float deltaH = (float)(totalHeight - rows * buttonHeight - (rows - 1) * spacing())
                   / (float)(rows + 1) ;
    if (deltaH < 0) {
        deltaH = 0;
    }

    y += deltaH;

    buttonWidth = (float)(totalWidth - spacing() * (columns-1)) / (float)columns;

    // calculate the positions and sizes
    foreach(item, m_itemList) {

        int btnRight = (int)rint(x + buttonWidth) - 1,
            btnLeft  = (int)rint(x);

        if (btnRight > r.right() && h > 0) {
            x = r.x();
            btnRight = (int)rint(x + buttonWidth) - 1;
            btnLeft  = (int)rint(x);

            y += h + spacing() + deltaH;
            h = 0;
            linecount++;
        }
        if (!testonly) {
            item->setGeometry( QRect( QPoint( btnLeft, (int)rint(y) ),
                                      QSize ( btnRight - btnLeft + 1, buttonHeight)));
        }
        x += buttonWidth + spacing();
        h =  qMax(h, buttonHeight);
    }

    int ret = (int)rint(y + h + deltaH) - r.y();

    return ret;
}
