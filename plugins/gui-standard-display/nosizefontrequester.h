/***************************************************************************
                          nosizefontrequester.h  -  description
                             -------------------
    copyright            : (C) 2015 by Pino Toscano
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

#ifndef KRADIO_NOSIZEFONTREQUESTER_H
#define KRADIO_NOSIZEFONTREQUESTER_H

#include <QFont>
#include <QWidget>

class QLabel;

/**
 * Font selector, with size disabled.
 *
 * Much like KFontRequester, but with the size choice disabled.
 */
class NoSizeFontRequester : public QWidget
{
Q_OBJECT
Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontSelected USER true)
public:
    NoSizeFontRequester(QWidget *parent = NULL);
    ~NoSizeFontRequester();

    void setFont(const QFont &font);
    QFont font() const;

signals:
    void fontSelected(const QFont &);

private slots:
    void slotButtonClicked();

private:
    void updateText();

private:
    QFont m_font;
    QLabel *m_text;
};



#endif
