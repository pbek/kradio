/***************************************************************************
                          nosizefontrequester.cpp  -  description
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

#include "nosizefontrequester.h"

#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <kfontchooser.h>
#include <klocalizedstring.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

NoSizeFontRequester::NoSizeFontRequester(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);

    m_text = new QLabel(this);
    m_text->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    layout->addWidget(m_text, 1);

    QPushButton *button = new QPushButton(i18nc("@action:button Choose font", "Choose..."), this);
    button->setToolTip(i18nc("@info:tooltip", "Click to select a font"));
    connect(button, SIGNAL(clicked()), SLOT(slotButtonClicked()));
    layout->addWidget(button);

    setFocusProxy(button);

    updateText();
}


NoSizeFontRequester::~NoSizeFontRequester()
{
}


void NoSizeFontRequester::setFont(const QFont &font)
{
    m_font = font;
    updateText();
    emit fontSelected(m_font);
}


QFont NoSizeFontRequester::font() const
{
    return m_font;
}


void NoSizeFontRequester::slotButtonClicked()
{
    QDialog          diag(this);
    
    QDialogButtonBox *btns    = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &diag);
    KFontChooser     *chooser = new KFontChooser    (&diag, KFontChooser::NoDisplayFlags);
    QVBoxLayout      *vlayout = new QVBoxLayout;
    
    vlayout->addWidget(chooser);
    vlayout->addWidget(btns);
    connect(btns, &QDialogButtonBox::accepted, &diag, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, &diag, &QDialog::reject);
    
    diag.setWindowTitle(i18nc("@title:window", "Select Font"));
    
    chooser->enableColumn(KFontChooser::SizeList, false);
    chooser->setFont(m_font);
    
    diag.setLayout(vlayout);
    
    if (diag.exec() == QDialog::Rejected) {
        return;
    }

    m_font = chooser->font();
    updateText();
    Q_EMIT fontSelected(m_font);
}


void NoSizeFontRequester::updateText()
{
    m_text->setFont(m_font);
    m_text->setText(m_font.family());
}
