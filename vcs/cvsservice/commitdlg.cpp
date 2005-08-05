/***************************************************************************
 *   Copyright (C) 1999, 2000 by Bernd Gehrmann                            *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qpushbutton.h>

#include <kapplication.h>
#include <kbuttonbox.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "commitdlg.h"

///////////////////////////////////////////////////////////////////////////////
// class CommitDialog
///////////////////////////////////////////////////////////////////////////////


CommitDialog::CommitDialog( const QString &changeLogfileNamePath,  QWidget *parent )
    : CommitDialogBase( parent, "commitdialog", true )
{
    connect( buttonOk, SIGNAL(clicked()), SLOT(accept()) );
    connect( buttonCancel, SIGNAL(clicked()), SLOT(reject()) );

    setChangeLogFileName( changeLogfileNamePath );
}

///////////////////////////////////////////////////////////////////////////////

QStringList CommitDialog::logMessage() const
{
    QStringList textLines;
    for (int i=0; i<textEdit->paragraphs(); ++i)
    {
        textLines << textEdit->text( i );
    }
    return textLines;
}

///////////////////////////////////////////////////////////////////////////////

bool CommitDialog::mustAddToChangeLog() const
{
    return checkAddToChangelog->isChecked();
}

///////////////////////////////////////////////////////////////////////////////

void CommitDialog::accept()
{
    if (textEdit->text().isNull() || textEdit->text().isEmpty()) {
        int s = KMessageBox::warningContinueCancel( this,
            i18n("You are committing your changes without any comment. This is not a good practice. Continue anyway?"),
            i18n("CVS Commit Warning"),
            KStdGuiItem::cont(),
            i18n("askWhenCommittingEmptyLogs") );
        if ( s != KMessageBox::Continue ) {
            return;
        }
    }
    QDialog::accept();
}

///////////////////////////////////////////////////////////////////////////////

void CommitDialog::setChangeLogFileName( const QString &fileName )
{
    changeLogNameEdit->setText( fileName );
}

///////////////////////////////////////////////////////////////////////////////

QString CommitDialog::changeLogFileName() const
{
    return changeLogNameEdit->text();
}

#include "commitdlg.moc"
