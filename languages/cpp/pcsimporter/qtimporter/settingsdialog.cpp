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

#include "settingsdialog.h"
#include <klistbox.h>
#include <kcombobox.h>
#include <qfile.h>
#include <qdir.h>
#include <qregexp.h>
#include <cstdlib>

QListBoxItem* QListBox_selectedItem( QListBox* cpQListBox )
{
	if ( cpQListBox->selectionMode() != QListBox::Single )
		return 0;
	if ( cpQListBox->isSelected( cpQListBox->currentItem() ) )
		return cpQListBox->item( cpQListBox->currentItem() );
	return 0;
}

SettingsDialog::SettingsDialog( QWidget* parent, const char* name, WFlags fl )
		: SettingsDialogBase( parent, name, fl )
{
	QStringList qtdirs;
	qtdirs.push_back( ::getenv( "QTDIR" ) );
	qtdirs.push_back( "/usr/lib/qt3" );
	qtdirs.push_back( "/usr/lib/qt" );
	qtdirs.push_back( "/usr/share/qt3" );

	for ( QStringList::Iterator it = qtdirs.begin(); it != qtdirs.end(); ++it )
	{
		QString qtdir = *it;
		if ( !qtdir.isEmpty() && isValidQtDir( qtdir ) )
			if ( !qtListBox->findItem( qtdir, ExactMatch ) )
				qtListBox->insertItem( qtdir );
	}
}

SettingsDialog::~SettingsDialog()
{}

void SettingsDialog::slotSelectionChanged( QListBoxItem* )
{
	if ( !qtListBox->selectedItem() )
	{
		emit enabled( false );
		return ;
	}

	QDir dir( qtDir() + "/include" );
	QStringList qconfigFileList = dir.entryList( "qconfig-*.h" );
	qtConfiguration->clear();
	QRegExp rx( "qconfig-(\\w+)\\.h" );
	for ( QStringList::Iterator it = qconfigFileList.begin(); it != qconfigFileList.end(); ++it )
	{
		( void ) rx.exactMatch( *it );
		qtConfiguration->insertItem( rx.cap( 1 ) );
	}

	emit enabled( true );
}

bool SettingsDialog::isValidQtDir( const QString & path ) const
{
	return QFile::exists( path + "/include/qt.h" );
}

QString SettingsDialog::qtDir( ) const
{
	return qtListBox->currentText();
}

QString SettingsDialog::configuration( ) const
{
	return qtConfiguration->currentText();
}

#include "settingsdialog.moc" 
//kate: indent-mode csands; tab-width 4; space-indent off;
