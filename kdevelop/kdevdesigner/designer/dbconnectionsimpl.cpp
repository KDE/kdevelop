/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "dbconnectionsimpl.h"
#include <qptrlist.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include "project.h"
#include <qlistbox.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qsqldatabase.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include "mainwindow.h"
#include "asciivalidator.h"

#include <klocale.h>

static bool blockChanges = FALSE;

/*
 *  Constructs a DatabaseConnectionsEditor which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
DatabaseConnectionsEditor::DatabaseConnectionsEditor( Project *pro, QWidget* parent,  const char* name, bool modal, WFlags fl )
    : DatabaseConnectionBase( parent, name, modal, fl ), project( pro )
{
    connect( buttonHelp, SIGNAL( clicked() ), MainWindow::self, SLOT( showDialogHelp() ) );
    connectionWidget = new DatabaseConnectionWidget( grp );
    grpLayout->addWidget( connectionWidget, 0, 0 );
#ifndef QT_NO_SQL
    QPtrList<DatabaseConnection> lst = project->databaseConnections();
    for ( DatabaseConnection *conn = lst.first(); conn; conn = lst.next() )
	listConnections->insertItem( conn->name() );
    connectionWidget->comboDriver->insertStringList( QSqlDatabase::drivers() );
#endif
    connectionWidget->editName->setValidator( new AsciiValidator( connectionWidget->editName ) );
    enableAll( FALSE );
}

DatabaseConnectionsEditor::~DatabaseConnectionsEditor()
{
}

void DatabaseConnectionsEditor::deleteConnection()
{
    if ( listConnections->currentItem() == -1 )
	return;
    project->removeDatabaseConnection( listConnections->currentText() );
    delete listConnections->item( listConnections->currentItem() );
    if ( listConnections->count() ) {
	listConnections->setCurrentItem( 0 );
	currentConnectionChanged( listConnections->currentText() );
    } else {
	enableAll( FALSE );
    }
    project->saveConnections();
}

void DatabaseConnectionsEditor::newConnection()
{
    blockChanges = TRUE;
    enableAll( TRUE );
    QString n( "(default)" );
    if ( project->databaseConnection( n ) ) {
	n = "connection";
	int i = 2;
	while ( project->databaseConnection( n + QString::number( i ) ) )
	    ++i;
	n = n + QString::number( i );
    }
    connectionWidget->editName->setText( n );
    listConnections->clearSelection();
    buttonConnect->setDefault( TRUE );
    connectionWidget->editName->setFocus();
    blockChanges = FALSE;
}

void DatabaseConnectionsEditor::doConnect()
{
#ifndef QT_NO_SQL
    if ( listConnections->currentItem() == -1 ||
	 !listConnections->item( listConnections->currentItem() )->isSelected() ) { // new connection
	// ### do error checking for duplicated connection names
	DatabaseConnection *conn = new DatabaseConnection( project );
	conn->setName( connectionWidget->editName->text() );
	conn->setDriver( connectionWidget->comboDriver->lineEdit()->text() );
	conn->setDatabase( connectionWidget->editDatabase->text() );
	conn->setUsername( connectionWidget->editUsername->text() );
	conn->setPassword( connectionWidget->editPassword->text() );
	conn->setHostname( connectionWidget->editHostname->text() );
	conn->setPort( connectionWidget->editPort->value() );
	if ( conn->refreshCatalog() ) {
	    project->addDatabaseConnection( conn );
	    listConnections->insertItem( conn->name() );
	    listConnections->setCurrentItem( listConnections->count() - 1 );
	    project->saveConnections();
	} else {
	    QMessageBox::warning( MainWindow::self, i18n( "Connection" ),
				  i18n( "Could not connect to the database.\n"
						    "Please ensure that the database server is running "
						    "and that all the connection information is correct.\n"
						    "[ " + conn->lastError() + " ]" ) );
	    delete conn;
	}
    } else { // sync // ### should this do something else? right now it just overwrites all info about the connection...
	DatabaseConnection *conn = project->databaseConnection( listConnections->currentText() );
	conn->setName( connectionWidget->editName->text() );
	conn->setDriver( connectionWidget->comboDriver->lineEdit()->text() );
	conn->setDatabase( connectionWidget->editDatabase->text() );
	conn->setUsername( connectionWidget->editUsername->text() );
	conn->setPassword( connectionWidget->editPassword->text() );
	conn->setHostname( connectionWidget->editHostname->text() );
	conn->setPort( connectionWidget->editPort->value() );
	conn->refreshCatalog();
	project->saveConnections();
    }
#endif
}

void DatabaseConnectionsEditor::currentConnectionChanged( const QString &s )
{
#ifndef QT_NO_SQL
    DatabaseConnection *conn = project->databaseConnection( s );
    blockChanges = TRUE;
    enableAll( conn != 0 );
    connectionWidget->editName->setEnabled( FALSE );
    blockChanges = FALSE;
    if ( !conn )
	return;
    blockChanges = TRUE;
    connectionWidget->editName->setText( conn->name() );
    blockChanges = FALSE;
    connectionWidget->comboDriver->lineEdit()->setText( conn->driver() );
    connectionWidget->editDatabase->setText( conn->database() );
    connectionWidget->editUsername->setText( conn->username() );
    connectionWidget->editPassword->setText( conn->password() );
    connectionWidget->editHostname->setText( conn->hostname() );
    connectionWidget->editPort->setValue( conn->port() );
#endif
}

void DatabaseConnectionsEditor::connectionNameChanged( const QString &s )
{
    if ( listConnections->currentItem() == 0 || blockChanges )
	return;
    listConnections->changeItem( s, listConnections->currentItem() );
}

void DatabaseConnectionsEditor::enableAll( bool b )
{
    connectionWidget->editName->setEnabled( b );
    connectionWidget->editName->setText( "" );
    connectionWidget->comboDriver->setEnabled( b );
    connectionWidget->comboDriver->lineEdit()->setText( "" );
    connectionWidget->editDatabase->setEnabled( b );
    connectionWidget->editDatabase->setText( "" );
    connectionWidget->editUsername->setEnabled( b );
    connectionWidget->editUsername->setText( "" );
    connectionWidget->editPassword->setEnabled( b );
    connectionWidget->editPassword->setText( "" );
    connectionWidget->editHostname->setEnabled( b );
    connectionWidget->editHostname->setText( "" );
    connectionWidget->editPort->setEnabled( b );
    connectionWidget->editPort->setValue( -1 );
    buttonConnect->setEnabled( b );
}
