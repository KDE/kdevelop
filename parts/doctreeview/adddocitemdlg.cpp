/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <kbuttonbox.h>
#include <kfiledialog.h>
#include <klocale.h>

#include "adddocitemdlg.h"


AddDocItemDialog::AddDocItemDialog(QWidget *parent, const char *name)
    : QDialog(parent, name, true)
{
    setCaption(("Add Documentation Entry"));
    
    QLabel *title_label = new QLabel(i18n("&Title:"), this);
    title_edit = new QLineEdit(this);
    title_edit->setFocus();
    title_label->setBuddy(title_edit);

    QLabel *url_label = new QLabel(i18n("&File name:"), this);
    url_edit = new QLineEdit(this);
    url_label->setBuddy(url_edit);
    QFontMetrics fm(url_edit->fontMetrics());
    url_edit->setMinimumWidth(fm.width('X')*35);
    
    QPushButton *url_button = new QPushButton("...", this);
    url_button->setFixedSize(30, 25);

    connect( url_button, SIGNAL(clicked()), this, SLOT(fileButtonClicked()));
        
    QVBoxLayout *layout = new QVBoxLayout(this, 10);
    
    QGridLayout *grid = new QGridLayout(2, 3);
    layout->addLayout(grid);
    grid->addWidget(title_label, 0, 0);
    grid->addMultiCellWidget(title_edit, 0, 0, 1, 2);
    grid->addWidget(url_label, 1, 0);
    grid->addWidget(url_edit, 1, 1);
    grid->addWidget(url_button, 1, 2);

    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(frame, 0);

    KButtonBox *buttonbox = new KButtonBox(this);
    buttonbox->addStretch();
    QPushButton *ok = buttonbox->addButton(i18n("&OK"));
    QPushButton *cancel = buttonbox->addButton(i18n("Cancel"));
    ok->setDefault(true);
    connect( ok, SIGNAL(clicked()), this, SLOT(accept()) );
    connect( cancel, SIGNAL(clicked()), this, SLOT(reject()) );
    buttonbox->layout();
    layout->addWidget(buttonbox, 0);
}


AddDocItemDialog::~AddDocItemDialog()
{}


void AddDocItemDialog::fileButtonClicked()
{
    QString fileName = KFileDialog::getOpenFileName(QString::null, "*.html", this,
                                                    i18n("Choose a File Name"));
    if (!fileName.isEmpty())
        url_edit->setText(fileName);
}

#include "adddocitemdlg.moc"
