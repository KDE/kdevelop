/***************************************************************************
                             -------------------
    begin                : 12/21/2002
    copyright            : (C) 2002 by Victor Röder
    email                : victor_roeder@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "addexistingdirectoriesdlg.h"

#include <qgroupbox.h>
#include <qlayout.h>

#include <kprogress.h>

#include "autoprojectwidget.h"
#include "autoprojectpart.h"

#include "misc.h"

AddExistingDirectoriesDialog::AddExistingDirectoriesDialog ( AutoProjectPart* part, AutoProjectWidget *widget, SubprojectItem* spitem, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 )
  : AddExistingDlgBase ( parent, name, modal, fl )
{
	setIcon ( SmallIcon ( "fileimport.png" ) );

	m_spitem = spitem;

	m_part = part;
	m_widget = widget;

    KFile::Mode mode = KFile::Directory;

//     if ( spitem && spitem->type() == ProjectItem::Subproject )
//     {
//         destStaticLabel->setText ( i18n ( "Subproject:" ) );
//         destLabel->setText ( spitem->subdir );
//         targetLabel->setText ( i18n ( "none" ) );
//         directoryLabel->setText ( i18n ( spitem->path ) );
//     }

	sourceSelector = new FileSelectorWidget ( part, mode, sourceGroupBox, "source file selector" );
	sourceGroupBoxLayout->addWidget ( sourceSelector );

	importView = new KImportIconView ( "Drag one or more directories with an existing Makefile.am from above and drop it here!", destGroupBox, "destination icon view" );
	destGroupBoxLayout->addWidget ( importView );

	setIcon ( SmallIcon ( "fileimport.png" ) );

	init();
}


AddExistingDirectoriesDialog::~AddExistingDirectoriesDialog()
{
}

void AddExistingDirectoriesDialog::init()
{
	progressBar->hide();

	importView->setMode ( KIconView::Select );
    importView->setItemsMovable ( false );

	connect ( okButton, SIGNAL ( clicked () ), this, SLOT ( slotOk () ) );

    connect ( addSelectedButton, SIGNAL ( clicked () ), this, SLOT ( slotAddSelected() ) );
    connect ( addAllButton, SIGNAL ( clicked () ), this, SLOT ( slotAddAll() ) );
    connect ( removeSelectedButton, SIGNAL ( clicked () ), this, SLOT ( slotRemoveSelected() ) );
    connect ( removeAllButton, SIGNAL ( clicked () ), this, SLOT ( slotRemoveAll() ) );

    connect ( importView, SIGNAL ( dropped( QDropEvent* ) ), this, SLOT ( slotDropped ( QDropEvent* ) ) );

    importView->setSelectionMode ( KFile::Multi );
	
	sourceSelector->setDir ( m_spitem->path );
}

void AddExistingDirectoriesDialog::importItems()
{
}

void AddExistingDirectoriesDialog::slotOk()
{
	if ( importView->items()->count() == 0 ) QDialog::reject();
    
}

void AddExistingDirectoriesDialog::slotAddSelected()
{
}

void AddExistingDirectoriesDialog::slotAddAll()
{
}

void AddExistingDirectoriesDialog::slotRemoveAll()
{
}

void AddExistingDirectoriesDialog::slotRemoveSelected()
{
}

void AddExistingDirectoriesDialog::slotDropped ( QDropEvent* ev )
{
}

#include "addexistingdirectoriesdlg.h"
