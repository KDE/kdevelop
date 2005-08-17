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
//Added by qt3to4:
#include <QVBoxLayout>
#include <Q3Frame>
#include <QGridLayout>
#include <kbuttonbox.h>
#include <klocale.h>
#include <kstdguiitem.h>
#include <kdeversion.h>

#include "addfilegroupdlg.h"


AddFileGroupDialog::AddFileGroupDialog(const QString& old_title, const QString& old_pattern, QWidget *parent, const char *name)
    : QDialog(parent, name, true)
{
    QLabel *title_label = new QLabel(i18n("&Title:"), this);
    title_edit = new QLineEdit(old_title, this);
    title_edit->setFocus();
    title_label->setBuddy(title_edit);
    connect( title_edit, SIGNAL( textChanged ( const QString & ) ), this, SLOT( slotTextChanged() ) );

    QLabel *pattern_label = new QLabel(i18n("&Pattern:"), this);
    pattern_edit = new QLineEdit(old_pattern, this);
    pattern_label->setBuddy(pattern_edit);
    QFontMetrics fm(pattern_edit->fontMetrics());
    pattern_edit->setMinimumWidth(fm.width('X')*35);
    connect( pattern_edit, SIGNAL( textChanged ( const QString & ) ), this, SLOT( slotTextChanged() ) );
    QVBoxLayout *layout = new QVBoxLayout(this, 10);

    QGridLayout *grid = new QGridLayout(2, 2);
    layout->addLayout(grid);
    grid->addWidget(title_label, 0, 0);
    grid->addWidget(title_edit, 0, 1);
    grid->addWidget(pattern_label, 1, 0);
    grid->addWidget(pattern_edit, 1, 1);

    Q3Frame *frame = new Q3Frame(this);
    frame->setFrameStyle(Q3Frame::HLine | Q3Frame::Sunken);
    layout->addWidget(frame, 0);

    KButtonBox *buttonbox = new KButtonBox(this);
    buttonbox->addStretch();
#if KDE_IS_VERSION( 3, 2, 90 )
    m_pOk = buttonbox->addButton(KStdGuiItem::ok());
    QPushButton *cancel = buttonbox->addButton(KStdGuiItem::cancel());
#else
    m_pOk = buttonbox->addButton(KStdGuiItem::ok().text());
    QPushButton *cancel = buttonbox->addButton(KStdGuiItem::cancel().text());
#endif

    m_pOk->setDefault(true);
    connect( m_pOk, SIGNAL(clicked()), this, SLOT(accept()) );
    connect( cancel, SIGNAL(clicked()), this, SLOT(reject()) );
    buttonbox->layout();
    layout->addWidget(buttonbox, 0);
    slotTextChanged();
}


AddFileGroupDialog::~AddFileGroupDialog()
{}

void AddFileGroupDialog::slotTextChanged()
{
    m_pOk->setEnabled( !title_edit->text().isEmpty() && !pattern_edit->text().isEmpty() );
}

#include "addfilegroupdlg.moc"
