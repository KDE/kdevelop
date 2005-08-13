/***************************************************************************
*   Copyright (C) 2003 by Roberto Raggi                                   *
*   roberto@kdevelop.org                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <qdir.h>

#include <klistbox.h>
#include <kcombobox.h>
#include <kurlrequester.h>
#include <kdeversion.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <klineedit.h>

#include <keditlistbox.h>

// should be included after possible KEditListBox redefinition
#include "settingsdialog.h"

#include <qfile.h>
#include <qregexp.h>
#include <qlayout.h>
#include <qcheckbox.h>

#include <cstdlib>

SettingsDialog::SettingsDialog( QWidget* parent, const char* name, WFlags fl )
		: SettingsDialogBase( parent, name, fl )
{
	KURLRequester * req = new KURLRequester( this );
	req->setMode( KFile::Directory );
	KEditListBox::CustomEditor pCustomEditor;
	pCustomEditor = req->customEditor();
	elb = new KEditListBox( i18n( "Directories to Parse" ), pCustomEditor, this );

	grid->addMultiCellWidget( elb, 2, 2, 0, grid->numCols() );

	connect( dbName_edit, SIGNAL( textChanged( const QString& ) ), this, SLOT( validate() ) );
	connect( elb->addButton(), SIGNAL( clicked() ), this, SLOT( validate() ) );
	connect( elb->removeButton(), SIGNAL( clicked() ), this, SLOT( validate() ) );
	connect( elb, SIGNAL( added( const QString& ) ), this, SLOT( validateDirectory( const QString& ) ) );
}

SettingsDialog::~SettingsDialog()
{}

QString SettingsDialog::dbName( ) const
{
	return dbName_edit->text();
}

QStringList SettingsDialog::dirs( ) const
{
	return elb->items();
}

bool SettingsDialog::recursive( ) const
{
	return recursive_box->isChecked();
}

void SettingsDialog::validate()
{
	emit enabled( !dbName_edit->text().isEmpty() && elb->listBox() ->count() > 0 );
}

void SettingsDialog::validateDirectory( const QString & dir )
{
	QDir d( dir, QString::null, QDir::DefaultSort, QDir::Dirs );
	if ( !d.exists() )
	{
		elb->lineEdit() ->setText( dir );

		if ( QListBoxItem * item = elb->listBox() ->findItem( dir, Qt::ExactMatch ) )
		{
			elb->listBox() ->removeItem( elb->listBox() ->index( item ) );
		}

		QString errormsg = QString( "<qt><b>%1</b> is not a directory</qt>" ).arg( dir );
		KMessageBox::error( 0, errormsg, "Couldn't find directory" );
	}
}

#include "settingsdialog.moc" 
//kate: indent-mode csands; tab-width 4; space-indent off;



