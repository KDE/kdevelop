/***************************************************************************
 *   Copyright (C) 1999, 2000 by Bernd Gehrmann                            *
 *   bernd@kdevelop.org                                                    *
 *   Modified for perforce 2002 by Harald Fernengel <harry@kdevelop.org>   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "commitdlg.h"

#include <qlayout.h>
#include <qlabel.h>
#include <q3textedit.h>
#include <qpushbutton.h>
#include <qregexp.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <kprocess.h>
#include <kapplication.h>
#include <klocale.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include <stdlib.h>

#include "execcommand.h"

CommitDialog::CommitDialog( QWidget *parent, const char *name )
    : KDialogBase( parent, name, true, i18n("Perforce Submit"), Ok|Cancel|Details )
{
    QWidget *w = new QWidget( this, "main widget" );
    setMainWidget( w );

    edit = new Q3TextEdit( w );
    QFontMetrics fm(edit->fontMetrics());
    edit->setMinimumSize(fm.width("0")*40, fm.lineSpacing()*3);

    QVBoxLayout *layout = new QVBoxLayout( w, 0, spacingHint() );
    QLabel *editLabel = new QLabel(i18n("&Enter description:"), w);
    editLabel->setBuddy(edit);
    layout->addWidget(editLabel);
    layout->addWidget(edit);

    w = new QWidget( this, "details widget" );

    clientEdit = new KLineEdit( w );
    userEdit = new KLineEdit( w );
    filesBox = new KListBox( w );

    layout = new QVBoxLayout( w, 0, spacingHint() );
    QLabel *clientLabel = new QLabel(i18n("C&lient:"), w);
    clientLabel->setBuddy(clientEdit);
    layout->addWidget(clientLabel);
    layout->addWidget( clientEdit );
    QLabel *userLabel = new QLabel(i18n("&User:"), w);
    userLabel->setBuddy(userEdit);
    layout->addWidget( userLabel );
    layout->addWidget( userEdit );
    QLabel *filesLabel = new QLabel(i18n("&File(s):"), w);
    filesLabel->setBuddy(filesBox);
    layout->addWidget( filesLabel );
    layout->addWidget( filesBox );

    setDetailsWidget( w );
    autoGuess();
    edit->setFocus();
}

CommitDialog::~CommitDialog()
{
}

void CommitDialog::autoGuess()
{
    char *cenv;

    cenv = getenv( "P4USER" );
    if ( cenv ) {
        setUser( QString::fromLocal8Bit( cenv ) );
    }

    cenv = getenv( "P4CLIENT" );
    if ( cenv ) {
        setClient( QString::fromLocal8Bit( cenv ) );
    }
}

void CommitDialog::setFiles( const QStringList& lst )
{
    filesBox->clear();
    setDepotFiles( lst );
}

void CommitDialog::setDepotFiles( const QStringList& lst )
{
    QStringList args;

    args << "files";
    for ( QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
        args << (*it);
    }

    ExecCommand* cmd = new ExecCommand( "p4", args, QString::null, QStringList(), this );
    connect( cmd, SIGNAL(finished( const QString&, const QString& )),
	     this, SLOT(getFilesFinished( const QString&, const QString& )) );
}

void CommitDialog::getFilesFinished( const QString& out, const QString& /* err */ )
{
    QStringList lst = QStringList::split( QChar('\n'), out );
    for ( QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
        int pos = (*it).find( QChar('#') );
        if ( pos > 1 && (*it).startsWith( "//" ) ) {
            filesBox->insertItem( (*it).left( pos ) );
        }
    }
}

QString CommitDialog::changeList() const
{
    QString lst;

    lst += "Change: new\n"
           "Client: " + client() + "\n"
           "User: " + user() + "\n"
           "Status: new\n"
           "Description:\n        ";

    lst += logMessage().replace(QRegExp("\n"), "\n        ") + "\n\n";

    lst += "Files:\n";

    for ( uint i = 0; i < filesBox->count(); ++i ) {
        lst += "       " + filesBox->text( i ) + "\n";
    }

    return lst;
}

void CommitDialog::accept()
{
    if ( client().isEmpty() ) {
        setDetails( true );
        KMessageBox::error( this, i18n("Please enter the P4 client name.") );
        clientEdit->setFocus();
    } else if ( user().isEmpty() ) {
        setDetails( true );
        KMessageBox::error( this, i18n("Please enter the P4 user.") );
        userEdit->setFocus();
    } else if ( filesBox->count() == 0 ) {
        setDetails( true );
        KMessageBox::error( this, i18n("The changelist does not contain any files.") );
    } else {
        KDialogBase::accept();
    }
}

#include "commitdlg.moc"
