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
#include <klocale.h>

#include "addenvvardlg.h"


AddEnvvarDialog::AddEnvvarDialog(QWidget *parent, const char *name)
    : QDialog(parent, name, true)
{
    setCaption(("Add environment variable"));
    
    QLabel *varname_label = new QLabel(i18n("&Name:"), this);
    varname_edit = new QLineEdit(this);
    varname_edit->setFocus();
    varname_label->setBuddy(varname_edit);

    QLabel *value_label = new QLabel(i18n("&Value:"), this);
    value_edit = new QLineEdit(this);
    value_label->setBuddy(value_edit);
    QFontMetrics fm(value_edit->fontMetrics());
    value_edit->setMinimumWidth(fm.width('X')*35);
    
    QVBoxLayout *layout = new QVBoxLayout(this, 10);
    
    QGridLayout *grid = new QGridLayout(2, 2);
    layout->addLayout(grid);
    grid->addWidget(varname_label, 0, 0);
    grid->addWidget(varname_edit, 0, 1);
    grid->addWidget(value_label, 1, 0);
    grid->addWidget(value_edit, 1, 1);

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


AddEnvvarDialog::~AddEnvvarDialog()
{}

#include "addenvvardlg.moc"
