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
#include <kapplication.h> 
#include <kstandarddirs.h> 
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
	KApplication::kApplication()->dirs()->addResourceType("include","include"); 
	QStringList kdedirs=KApplication::kApplication()->dirs()->findDirs("include","");
	for( QStringList::Iterator it=kdedirs.begin(); it!=kdedirs.end();    ++it ) 
	{ 
		QString kdedir = *it;
		if ( !kdedir.isEmpty() && isValidKDELibsDir( kdedir ) )
			if ( !kdeListBox->findItem( kdedir, ExactMatch ) )
				kdeListBox->insertItem( kdedir );
	}
}
	
SettingsDialog::~SettingsDialog()
{}

void SettingsDialog::slotSelectionChanged( QListBoxItem* )
{
	emit enabled( kdeListBox->selectedItem() != 0 );
}

bool SettingsDialog::isValidKDELibsDir( const QString & path ) const
{
	return QFile::exists( path + "/kapplication.h" );
}

QString SettingsDialog::kdeDir( ) const
{
	return kdeListBox->currentText();
}


#include "settingsdialog.moc"
//kate: indent-mode csands; tab-width 4; space-indent off;



