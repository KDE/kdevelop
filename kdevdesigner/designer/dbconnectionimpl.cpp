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

#include "dbconnectionimpl.h"
#include "dbconnection.h"
#include "project.h"
#include "asciivalidator.h"
#include <qgroupbox.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <klineedit.h>
#include <qlayout.h>
#include <qsqldatabase.h>

DatabaseConnectionEditor::DatabaseConnectionEditor( DatabaseConnection* connection, QWidget* parent,
						    const char* name, bool modal, WFlags fl )
    : DatabaseConnectionEditorBase( parent, name, modal, fl ), conn( connection )
{
    connectionWidget = new DatabaseConnectionWidget( grp );
    grpLayout->addWidget( connectionWidget, 0, 0 );
    init();
}

DatabaseConnectionEditor::~DatabaseConnectionEditor()
{
}

void DatabaseConnectionEditor::accept()
{
#ifndef QT_NO_SQL
    conn->setUsername( connectionWidget->editUsername->text() );
    conn->setPassword( connectionWidget->editPassword->text() );
    conn->setHostname( connectionWidget->editHostname->text() );
#endif
    DatabaseConnectionEditorBase::accept();
}

void DatabaseConnectionEditor::init()
{
    connectionWidget->editName->setEnabled( FALSE );
    connectionWidget->editName->setValidator( new AsciiValidator( connectionWidget->editName ) );
    connectionWidget->editName->setText( conn->name() );
    connectionWidget->comboDriver->setEnabled( FALSE );
    connectionWidget->comboDriver->lineEdit()->setText( conn->driver() );
    connectionWidget->editDatabase->setEnabled( FALSE );
    connectionWidget->editDatabase->setText( conn->database() );
    connectionWidget->editUsername->setEnabled( TRUE );
    connectionWidget->editUsername->setText( conn->username() );
    connectionWidget->editPassword->setEnabled( TRUE );
    connectionWidget->editPassword->setText( "" );
    connectionWidget->editHostname->setEnabled( TRUE );
    connectionWidget->editHostname->setText( conn->hostname() );
    connectionWidget->editPort->setEnabled( TRUE );
    connectionWidget->editPort->setValue( conn->port() );
    connectionWidget->editUsername->setFocus();
    connectionWidget->editUsername->selectAll();
}
