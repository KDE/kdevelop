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

#include "addprefixdlg.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <kbuttonbox.h>
#include <kfiledialog.h>
#include <klocale.h>


AddPrefixDialog::AddPrefixDialog( const QString& nameEdit, const QString& pathEdit,
        QWidget *parent, const char *name)
    : QDialog(parent, name, true)
{
    setCaption(i18n("Add Prefix"));
    
    QLabel *name_label = new QLabel(i18n("&Name:"), this);
    name_edit = new QLineEdit(nameEdit, this);
    name_edit->setFocus();
    name_label->setBuddy(name_edit);

    QLabel *path_label = new QLabel(i18n("&Path:"), this);
    path_edit = new QLineEdit(pathEdit, this);
    path_label->setBuddy(path_edit);
    QFontMetrics fm(path_edit->fontMetrics());
    path_edit->setMinimumWidth(fm.width('X')*35);
    
    QVBoxLayout *layout = new QVBoxLayout(this, 10);
    
    QGridLayout *grid = new QGridLayout(2, 2);
    layout->addLayout(grid);
    grid->addWidget(name_label, 0, 0);
    grid->addWidget(name_edit, 0, 1);
    grid->addWidget(path_label, 1, 0);
    grid->addWidget(path_edit, 1, 1);

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


AddPrefixDialog::~AddPrefixDialog()
{}

#include "addprefixdlg.moc"
