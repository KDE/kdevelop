/**********************************************************************
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
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

#include "projectsettingsimpl.h"
#include "project.h"
#include "formwindow.h"
#include "metadatabase.h"
#include "mainwindow.h"
#include "asciivalidator.h"
#include "mainwindow.h"
#include "sourcefile.h"
#include "workspace.h"

#include <kfiledialog.h>
#include <klocale.h>

#include <qlineedit.h>
#include <q3textedit.h>
#include <qcheckbox.h>
#include <q3filedialog.h>
#include <qcombobox.h>
#include <qobject.h>
#include <q3header.h>
#include <qpushbutton.h>
#include <qlabel.h>

/*
 *  Constructs a ProjectSettings which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
ProjectSettings::ProjectSettings( Project *pro, QWidget* parent,  const char* name, bool modal, Qt::WFlags fl )
    : ProjectSettingsBase( parent, name, modal, fl ), project( pro )
{
    connect( buttonHelp, SIGNAL( clicked() ), MainWindow::self, SLOT( showDialogHelp() ) );

    editProjectFile->setFocus();

    if ( project->isDummy() ) {
	editProjectFile->setEnabled( FALSE );
	editProjectFile->setText( project->projectName() );
    } else {
	if ( project->fileName().isEmpty() || project->fileName() == ".pro" ) {
	    editProjectFile->setText( tr( "unnamed.pro" ) );
	    editProjectFile->selectAll();
	} else {
	    editProjectFile->setText( project->fileName() );
	}
    }

    editDatabaseFile->setText( project->databaseDescription() );

    comboLanguage->insertStringList( MetaDataBase::languages() );
    for ( int j = 0; j < (int)comboLanguage->count(); ++j ) {
	if ( project->language() == comboLanguage->text( j ) ) {
	    comboLanguage->setCurrentItem( j );
	    break;
	}
    }
}

/*
 *  Destroys the object and frees any allocated resources
 */
ProjectSettings::~ProjectSettings()
{
}

void ProjectSettings::chooseDatabaseFile()
{
    QString fn = KFileDialog::getSaveFileName( editDatabaseFile->text(), i18n( "*.db|Database Files\n*|All Files" ), this );
    if ( fn.isEmpty() )
	return;
    editDatabaseFile->setText( fn );
}

void ProjectSettings::chooseProjectFile()
{
    QString fn = KFileDialog::getSaveFileName( editDatabaseFile->text(), i18n( "*.pro|Project Files\n*|All Files" ), this );
    if ( fn.isEmpty() )
	return;
    editProjectFile->setText( fn );
}

void ProjectSettings::helpClicked()
{
}

void ProjectSettings::okClicked()
{
    // ### check for validity
    project->setFileName( editProjectFile->text(), FALSE );
    project->setDatabaseDescription( editDatabaseFile->text() );
    project->setLanguage( comboLanguage->text( comboLanguage->currentItem() ) );
    project->setModified( TRUE );
    accept();
}

void ProjectSettings::languageChanged( const QString & )
{
}
