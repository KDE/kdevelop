/***************************************************************************
                             -------------------
    begin                : Frag' mich was leichteres
    copyright            : (C) 2002 by Victor Rder
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

/** Here resides the Import-existing-files-dialog of the Automake Manager (a KDevelop build tool part) **/

#include <qapplication.h>
#include <qgroupbox.h>
#include <qlabel.h>
//#include <qlistbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qpixmap.h>

#include <kdebug.h>
#include <kfileview.h>
#include <kguiitem.h>
#include <kprogress.h>
#include <kprocess.h>
#include <kurldrag.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kiconview.h>
#include <ksqueezedtextlabel.h>

#include "autolistviewitems.h"

#include "autoprojectwidget.h"
#include "autoprojectpart.h"

#include "kimporticonview.h"

#include "addexistingfilesdlg.h"

/*
 *  Constructs a AddExistingFilesDialog which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */

AddExistingFilesDialog::AddExistingFilesDialog ( AutoProjectPart* part, AutoProjectWidget *widget, SubprojectItem* spitem, TargetItem* titem, QWidget* parent, const char* name, bool modal, WFlags fl )
        : AddExistingDlgBase ( parent, name, modal, fl )
{
	m_spitem = spitem;
	m_titem = titem;

	m_part = part;
	m_widget = widget;

	KFile::Mode mode = KFile::Files;

	if ( titem && spitem && titem->type() == ProjectItem::Target && spitem->type() == ProjectItem::Subproject )
	{

		if ( titem->name.isEmpty() )
		{
			QString target = i18n ( "%1 in %2" ).arg ( titem->primary ).arg ( titem->prefix );
			targetLabel->setText ( target );
		}
		else
		{
			targetLabel->setText ( titem->name );
		}
		directoryLabel->setText ( spitem->path );
	}

	sourceSelector = new FileSelectorWidget ( m_part, mode, sourceGroupBox, "source file selector" );
	sourceGroupBoxLayout->addWidget ( sourceSelector );

	importView = new KImportIconView ( i18n ( "Drag one or more files from the left view and drop it here." ), destGroupBox, "destination icon view" );
	destGroupBoxLayout->addWidget ( importView );
	//destGroupBoxLayout->setStretchFactor(dir, 2);

    QWidget::setTabOrder(sourceSelector, addAllButton);
    QWidget::setTabOrder(addAllButton, addSelectedButton);
    QWidget::setTabOrder(addSelectedButton, importView);
    QWidget::setTabOrder(importView, removeAllButton);
    QWidget::setTabOrder(removeAllButton, removeSelectedButton);
    QWidget::setTabOrder(removeSelectedButton, okButton);
    QWidget::setTabOrder(okButton, cancelButton);

    sourceSelector->setFocus();

	setIcon ( SmallIcon ( "fileimport.png" ) );

	init();
}


/*
 *  Destroys the object and frees any allocated resources
 */
AddExistingFilesDialog::~AddExistingFilesDialog()
{
    // no need to delete child widgets, Qt does it all for me
}


// void AddExistingFilesDialog::resizeEvent ( QResizeEvent* ev )
// {
//     AddExistingDlgBase::resizeEvent ( ev );
//     //importView->update();
// }


void AddExistingFilesDialog::init()
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

    Q_ASSERT( m_spitem );
    sourceSelector->setDir ( m_spitem->path );
}

void AddExistingFilesDialog::importItems()
{
    if( !importView->items() )
	return;

	// items added via button or drag 'n drop
	KFileItemListIterator itemList ( m_importList );

	// items already added to the importView
	KFileItemListIterator importedList ( *importView->items() );

	QListViewItem* child = m_titem->firstChild();

	QStringList duplicateList;

	while ( child )
	{
		FileItem* curItem = static_cast<FileItem*> ( child );

		itemList.toFirst();

		for ( ; itemList.current(); ++itemList )
		{
			if ( ( *itemList )->name() == curItem->name )
			{
				duplicateList.append ( ( *itemList )->name() );
				m_importList.remove ( ( *itemList ) );
			}
		}

		child = child->nextSibling();
	}

	importedList.toFirst();

	for ( ; importedList.current(); ++importedList )
	{
		itemList.toFirst();

		for ( ; itemList.current(); ++itemList )
		{
			if ( ( *importedList )->name() == ( *itemList )->name() )
			{
				m_importList.remove ( ( *itemList ) );

				// to avoid that a item is added twice
				if ( !duplicateList.remove ( ( *importedList )->name() ) )
				{
					duplicateList.append ( ( *importedList )->name() );
				}
			}
		}
	}

	if ( duplicateList.count() > 0 )
	{
		if ( KMessageBox::warningContinueCancelList ( this, i18n (
																		"The following file(s) already exist(s) in the target!\n"
																		"Press Continue to import only the new files.\n"
																		"Press Cancel to abort the complete import." ),
																		duplicateList, "Warning", KGuiItem ( i18n ( "Continue" ) ) ) == KMessageBox::Cancel )
		{
			m_importList.clear();
			return;
		}
	}

	itemList.toFirst();

	for ( ; itemList.current(); ++itemList )
	{
		if ( !( *itemList )->isDir() )
		{
			importView->insertItem ( ( *itemList ) );
		}
	}

	importView->somethingDropped ( true );

	m_importList.clear();

	importView->update ();
}

void AddExistingFilesDialog::slotOk()
{
	if ( importView->items()->count() == 0 ) QDialog::reject();

	progressBar->show();
	progressBar->setFormat ( i18n ( "Importing... %p%" ) );

	qApp->processEvents();

	KFileItemListIterator items ( *importView->items() );

	// contains at the end only the imported files outside the subproject directory
	KFileItemList outsideList;

	QStringList stringList;

	for ( ; items.current(); ++items )
	{
		// kdDebug ( 9000 ) << " **** " << (  *items )->url().directory() << "***** " << m_spitem->path << endl;
		if ( (  *items )->url().directory() != m_spitem->path )
		{
			stringList.append ( ( *items )->name() );
			outsideList.append ( ( *items ) );
		}
	}

	progressBar->setTotalSteps ( outsideList.count() + importView->items()->count() );

	if ( outsideList.count() > 0 )
	{
		if ( KMessageBox::questionYesNoList ( this, i18n (
								"The following file(s) are not in the Subproject directory.\n"
								"Press Link to add the files by creating symbolic links.\n"
								"Press Copy to copy the files into the directory." ),
								stringList, i18n("Warning"), KGuiItem ( i18n ( "Link (recommended)" ) ), KGuiItem ( i18n ( "Copy (not recommended)" ) ) ) == KMessageBox::No )
		{
			// Copy files into the Subproject directory
			KFileItemListIterator it ( outsideList ) ;

			for ( ; it.current(); ++it )
			{
				KProcess proc;

				proc << "cp";
				proc << ( *it )->url().path();
				proc << m_spitem->path;
				proc.start(KProcess::DontCare);

				progressBar->setValue ( progressBar->value() + 1 );
			}
		}
		else
		{
			// Link them into the Subproject directory
			KFileItemListIterator it ( outsideList ) ;

			for ( ; it.current(); ++it )
			{
				KProcess proc;

				proc << "ln";
				proc << "-s";
				proc << ( *it )->url().path();
				proc << m_spitem->path;
				proc.start(KProcess::DontCare);

				progressBar->setValue ( progressBar->value() + 1 );
			}
		}
	}

	items.toFirst();

	QString canontargetname = AutoProjectTool::canonicalize ( m_titem->name );
        QString varname;
        if( m_titem->primary == "PROGRAMS" || m_titem->primary == "LIBRARIES" || m_titem->primary == "LTLIBRARIES" )
            varname = canontargetname + "_SOURCES";
        else
            varname = m_titem->prefix + "_" + m_titem->primary;

	QMap<QString,QString> replaceMap;
	FileItem* fitem = 0L;
	QStringList fileList;

	for ( ; items.current(); ++items )
	{
		m_spitem->variables [ varname ] += ( " " + ( *items )->name() );
		replaceMap.insert ( varname, m_spitem->variables [ varname ] );

		fitem = m_widget->createFileItem ( ( *items )->name(), m_spitem );
		m_titem->sources.append ( fitem );
		m_titem->insertItem ( fitem );

		fileList.append ( m_spitem->path.mid ( m_part->projectDirectory().length() + 1 ) + "/" + ( *items )->name() );

		progressBar->setValue ( progressBar->value() + 1 );
	}

	m_widget->emitAddedFiles ( fileList );

	AutoProjectTool::modifyMakefileam ( m_spitem->path + "/Makefile.am", replaceMap );

	QDialog::accept();

}

void AddExistingFilesDialog::slotAddSelected()
{
	KFileItemListIterator it ( *sourceSelector->dirOperator()->selectedItems() );

	for ( ; it.current(); ++it )
	{
 		if ( ( *it )->url().isLocalFile() ) // maybe unnecessary
 		{
			m_importList.append ( ( *it ) );
		}
	}

	importItems();
}


void AddExistingFilesDialog::slotAddAll()
{
	KFileItemListIterator it ( *sourceSelector->dirOperator()->view()->items() );

	for ( ; it.current(); ++it )
	{
 		if ( ( *it )->url().isLocalFile() ) // maybe unnecessary
 		{
			m_importList.append ( ( *it ) );
		}
	}

	importItems();
}

void AddExistingFilesDialog::slotRemoveAll()
{
	KURL::List deletedFiles;
	KFileItemListIterator it ( *importView->items() );

	for ( ; it.current(); ++it )
	{
		kdDebug ( 9000 ) << "AddExistingFilesDialog::slotRemoveAll()" << endl;
		//deletedFiles.append ( ( *it )->url() );
		if ( (*it ) ) importView->removeItem ( *it );
	}

	importView->somethingDropped ( false );

	importView->viewport()->update();
}

void AddExistingFilesDialog::slotRemoveSelected()
{
	KFileItemListIterator items ( *importView->items() );

	KFileItemList* selectedList = (KFileItemList*) importView->selectedItems();

	KFileItem * deleteItem = 0L;

	for ( ; items.current(); ++items )
	{
		deleteItem = selectedList->first();

		while ( deleteItem )
		{
			if ( deleteItem == ( *items ) )
			{
				importView->removeItem ( deleteItem );
				deleteItem = selectedList->current();
			}
			else
			{
				deleteItem = selectedList->next();
			}
		}
	}

	if ( importView->items()->count() == 0 ) importView->somethingDropped ( false );

	importView->viewport()->update();
}


void AddExistingFilesDialog::slotDropped ( QDropEvent* ev )
{
    kdDebug ( 9000 ) << "AddExistingFilesDialog::dropped()" << endl;

	KURL::List urls;

	KURLDrag::decode( ev, urls );

	KFileItem* item = 0L;
	KMimeType::Ptr type = 0L;


    for ( KURL::List::Iterator it = urls.begin(); it != urls.end(); ++it )
    {
 		if ( ( *it ).isLocalFile() ) // maybe unnecessary
 		{
			type = KMimeType::findByURL ( ( *it ) );

			if ( type->name() != KMimeType::defaultMimeType() )
			{
				item = new KFileItem ( ( *it ) , type->name(), 0 );
			}
			else
			{
				// take a text-file-icon instead of the ugly question-mark-icon
				item = new KFileItem ( ( *it ), "text/plain", 0 );
			}

			m_importList.append ( item );
		}
    }

	importItems();
}

#include "addexistingfilesdlg.moc"
