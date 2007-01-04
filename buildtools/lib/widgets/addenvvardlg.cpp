/* This file is part of the KDE project
   Copyright (C) 2001 Bernd Gehrmann <bernd@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <kbuttonbox.h>
#include <klocale.h>
#include <kstdguiitem.h>
#include <kdeversion.h>

#include "addenvvardlg.h"


AddEnvvarDialog::AddEnvvarDialog(QWidget *parent, const char *name)
    : QDialog(parent, name, true)
{
    setCaption(i18n("Add Environment Variable"));

    QLabel *varname_label = new QLabel(i18n("&Name:"), this);
    varname_edit = new KLineEdit(this);
    varname_edit->setFocus();
    varname_label->setBuddy(varname_edit);

    connect( varname_edit, SIGNAL( textChanged ( const QString & ) ), this, SLOT( slotTextChanged() ) );
    QLabel *value_label = new QLabel(i18n("&Value:"), this);
    value_edit = new KLineEdit(this);
    value_label->setBuddy(value_edit);
    QFontMetrics fm(value_edit->fontMetrics());
    value_edit->setMinimumWidth(fm.width('X')*35);
    connect( value_edit, SIGNAL( textChanged ( const QString & ) ), this, SLOT( slotTextChanged() ) );


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
    m_pOk = buttonbox->addButton( KStdGuiItem::ok());
    QPushButton *cancel = buttonbox->addButton(KStdGuiItem::cancel());
    m_pOk->setDefault(true);
    connect( m_pOk, SIGNAL(clicked()), this, SLOT(accept()) );
    connect( cancel, SIGNAL(clicked()), this, SLOT(reject()) );
    buttonbox->layout();
    layout->addWidget(buttonbox, 0);
    slotTextChanged();
}


AddEnvvarDialog::~AddEnvvarDialog()
{}

void AddEnvvarDialog::slotTextChanged()
{
    m_pOk->setEnabled( !varname_edit->text().isEmpty() && !value_edit->text().isEmpty() );
}

#include "addenvvardlg.moc"
