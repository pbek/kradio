/***************************************************************************
                          pluginsdelegate.cpp  -  description
                             -------------------
    copyright            : (C) 2016 by Pino Toscano
    email                : toscano.pino@tiscali.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "pluginsdelegate.h"

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QPainter>
#include <QStyle>
#include <QTextDocument>

#define MARGIN 1

PluginsDelegate::PluginsDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

PluginsDelegate::~PluginsDelegate()
{
}

void PluginsDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItemV4 optionV4 = option;
    initStyleOption(&optionV4, index);

    QStyle *style = optionV4.widget ? optionV4.widget->style() : QApplication::style();

    QTextDocument doc;
    doc.setHtml(optionV4.text);
    doc.setDocumentMargin(0);
    doc.setDefaultFont(optionV4.font);

    optionV4.text = QString();
    style->drawControl(QStyle::CE_ItemViewItem, &optionV4, painter);

    QAbstractTextDocumentLayout::PaintContext ctx;
    if (optionV4.state & QStyle::State_Selected)
        ctx.palette.setColor(QPalette::Text, optionV4.palette.color(QPalette::Active, QPalette::HighlightedText));
    else
        ctx.palette.setColor(QPalette::Text, optionV4.palette.color(QPalette::Active, QPalette::Text));

    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &optionV4);
    textRect.setWidth(textRect.width() - 2 * MARGIN);
    textRect.translate(MARGIN, MARGIN + (textRect.height() - 2 * MARGIN - doc.size().height()) / 2);
    painter->save();
    painter->translate(textRect.topLeft());
    painter->setClipRect(textRect.translated(-textRect.topLeft()));
    doc.documentLayout()->draw(painter, ctx);
    painter->restore();
}

QSize PluginsDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItemV4 optionV4 = option;
    initStyleOption(&optionV4, index);

    QSize ret;

    QTextDocument doc;
    doc.setHtml(optionV4.text);
    doc.setDocumentMargin(0);
    doc.setDefaultFont(optionV4.font);
    doc.setTextWidth(optionV4.rect.width());
    const QSize docSize = QSize(doc.idealWidth(), doc.size().height());
    ret = docSize;

    ret.rwidth() += 2 * MARGIN;
    ret.rheight() += 2 * MARGIN;

    return ret;
}
