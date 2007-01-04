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
#include <kstdguiitem.h>
#include <kdeversion.h>

AddPrefixDialog::AddPrefixDialog( const QString& nameEdit, const QString& pathEdit,
        QWidget *parent, const char *name)
    : QDialog(parent, name, true)
{
    setCaption(i18n("Add Prefix"));

    QLabel *name_label = new QLabel(i18n("&Name:"), this);
    name_edit = new KLineEdit(nameEdit, this);
    name_edit->setFocus();
    name_label->setBuddy(name_edit);
    connect( name_edit, SIGNAL( textChanged ( const QString & ) ), SLOT( slotPrefixChanged() ) );

    QLabel *path_label = new QLabel(i18n("&Path:"), this);
    path_edit = new KLineEdit(pathEdit, this);
    path_label->setBuddy(path_edit);
    QFontMetrics fm(path_edit->fontMetrics());
    path_edit->setMinimumWidth(fm.width('X')*35);
    connect( path_edit, SIGNAL( textChanged ( const QString & ) ), SLOT( slotPrefixChanged() ) );

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
    m_pOk = buttonbox->addButton(KStdGuiItem::ok());
    QPushButton *cancel = buttonbox->addButton(KStdGuiItem::cancel());
    m_pOk->setDefault(true);
    connect( m_pOk, SIGNAL(clicked()), this, SLOT(accept()) );
    connect( cancel, SIGNAL(clicked()), this, SLOT(reject()) );
    buttonbox->layout();
    layout->addWidget(buttonbox, 0);
    slotPrefixChanged();
}


AddPrefixDialog::~AddPrefixDialog()
{}

void AddPrefixDialog::slotPrefixChanged()
{
    m_pOk->setEnabled( !name_edit->text().isEmpty() && !path_edit->text().isEmpty() );
}

#include "addprefixdlg.moc"
