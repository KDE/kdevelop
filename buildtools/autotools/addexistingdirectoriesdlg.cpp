/***************************************************************************
                             -------------------
    begin                : 12/21/2002
    copyright            : (C) 2002 by Victor R�er
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

#include <qgroupbox.h>
#include <qlayout.h>

#include <kprogress.h>
#include <klocale.h>
#include <kio/netaccess.h>
#include <kprocess.h>

#include "autolistviewitems.h"
#include "autosubprojectview.h"

#include "autoprojectwidget.h"
#include "autoprojectpart.h"

#include "kimporticonview.h"

#include "urlutil.h"

#include "addexistingdirectoriesdlg.h"

AddExistingDirectoriesDialog::AddExistingDirectoriesDialog ( AutoProjectPart* part, AutoProjectWidget *widget, SubprojectItem* spitem, QWidget* parent, const char* name, bool modal, WFlags fl )
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

	importView = new KImportIconView ( i18n("Drag one or more directories with an existing Makefile.am from the left view and drop it here."), destGroupBox, "destination icon view" );
	destGroupBoxLayout->addWidget ( importView );

	setIcon ( SmallIcon ( "fileimport.png" ) );

    QWidget::setTabOrder(sourceSelector, addAllButton);
    QWidget::setTabOrder(addAllButton, addSelectedButton);
    QWidget::setTabOrder(addSelectedButton, importView);
    QWidget::setTabOrder(importView, removeAllButton);
    QWidget::setTabOrder(removeAllButton, removeSelectedButton);
    QWidget::setTabOrder(removeSelectedButton, okButton);
    QWidget::setTabOrder(okButton, cancelButton);

    sourceSelector->setFocus();
    
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
    if( !importView->items() )
	return;

	// items added via button or drag 'n drop
	KFileItemListIterator itemList ( m_importList );

	// items already added to the importView
	KFileItemListIterator importedList ( *importView->items() );

	QStringList duplicateList;

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

	itemList.toFirst();

	for ( ; itemList.current(); ++itemList )
	{
        KURL amURL = itemList.current()->url();
        amURL.addPath("Makefile.am");
        if (KIO::NetAccess::exists(amURL))
		{
			importView->insertItem ( ( *itemList ) );
		}
	}

	importView->somethingDropped ( true );

	m_importList.clear();

	importView->update ();
}

void AddExistingDirectoriesDialog::slotOk()
{
    if ( importView->items()->count() == 0 ) QDialog::reject();

    KFileItemListIterator items ( *importView->items() );

    QStringList dirs;

    for ( ; items.current(); ++items )
    {
        //if the directory is outside the project directory
//        kdDebug() << "dir to add " << items.current()->url().path() << "   subproject " << m_spitem->path << endl;
        if (items.current()->url().path() != m_spitem->path)
        {
            //copy
            //FIXME: check this after 3.0 release and add a possibility to link, not just copy
            KProcess proc;

            proc << "cp";
            proc << "-r";
            proc << items.current()->url().path();
            proc << m_spitem->path;
            proc.start(KProcess::Block);
        }
        dirs << items.current()->name();
    }

    for (QStringList::const_iterator it = dirs.begin(); it != dirs.end(); ++it)
    {
        QString name = *it;

        // Adjust SUBDIRS variable in containing Makefile.am
        if (m_spitem->variables["SUBDIRS"].find("$(TOPSUBDIRS)") != -1)
        {
            QFile subdirsfile( m_spitem->path + "/subdirs" );
            if ( subdirsfile.open( IO_WriteOnly | IO_Append ) )
            {
                QTextStream subdirsstream( &subdirsfile );
                subdirsstream << name << endl;
                subdirsfile.close();
            }
        }
        else if (m_spitem->variables["SUBDIRS"].find("$(AUTODIRS)") != -1)
        {
        }
        else
        {
            m_spitem->variables["SUBDIRS"] += (" " + name);
            QMap<QString,QString> replaceMap;
            replaceMap.insert("SUBDIRS", m_spitem->variables["SUBDIRS"]);
            AutoProjectTool::modifyMakefileam(m_spitem->path + "/Makefile.am", replaceMap);
        }

        // Create new item in tree view
        SubprojectItem *newitem = new SubprojectItem(m_spitem, name);
        newitem->subdir = name;
        newitem->path = m_spitem->path + "/" + name;
        newitem->variables["INCLUDES"] = m_spitem->variables["INCLUDES"];
        newitem->setOpen(true);

        // Move to the bottom of the list
        QListViewItem *lastItem = m_spitem->firstChild();
        while (lastItem->nextSibling())
            lastItem = lastItem->nextSibling();
        if (lastItem != newitem)
            newitem->moveItem(lastItem);

        // Create a Makefile in the new subdirectory

        QDir dir( m_spitem->path );
        QFile f( dir.filePath("Makefile.am") );
        if (f.exists()) {
            m_widget->getSubprojectView()->parse( newitem );
        } else {
            if (!f.open(IO_WriteOnly)) {
//                KMessageBox::sorry(this, i18n("Could not create Makefile.am in subdirectory %1.").arg(name));
                continue;
            }
            QTextStream stream(&f);
            stream << "INCLUDES = " << newitem->variables["INCLUDES"] << endl << "METASOURCES = AUTO" << endl;
            f.close();
        }



        // if !isKDE: add the new sub-proj to configure.in
        if ( !m_part->isKDE() ) {
            QString projroot = m_part->projectDirectory() + "/";
            QString subdirectory = dir.path();
            QString relpath = subdirectory.replace(0, projroot.length(),"");

            QString configurein = projroot + "configure.in";

            QStringList list = AutoProjectTool::configureinLoadMakefiles(configurein);
            list.push_back( relpath + "/Makefile" );
            AutoProjectTool::configureinSaveMakefiles(configurein, list);
        }

        m_part->needMakefileCvs();

    }
    QDialog::accept();
}

void AddExistingDirectoriesDialog::slotAddSelected()
{
    KFileItemListIterator it ( *sourceSelector->dirOperator()->selectedItems() );

    for ( ; it.current(); ++it )
    {
        QString relPath = URLUtil::extractPathNameRelative(m_part->projectDirectory(), ( *it )->url());
        if (relPath[relPath.length()-1] == '/')
            relPath = relPath.left(relPath.length()-1);
        if ( (relPath.isEmpty()) || (! m_widget->allSubprojects().contains( relPath )) )
        {
            m_importList.append ( ( *it ) );
        }
    }

    importItems();
}

void AddExistingDirectoriesDialog::slotAddAll()
{
	KFileItemListIterator it ( *sourceSelector->dirOperator()->view()->items() );

	for ( ; it.current(); ++it )
	{
        QString relPath = URLUtil::extractPathNameRelative(m_part->projectDirectory(), ( *it )->url());
        if (relPath[relPath.length()-1] == '/')
            relPath = relPath.left(relPath.length()-1);
        if ( (relPath.isEmpty()) || ( ! m_widget->allSubprojects().contains( relPath )) )
 		{
			m_importList.append ( ( *it ) );
		}
	}

	importItems();
}

void AddExistingDirectoriesDialog::slotRemoveAll()
{
	KURL::List deletedFiles;
	KFileItemListIterator it ( *importView->items() );

	for ( ; it.current(); ++it )
	{
		kdDebug ( 9000 ) << "AddExistingDirectoriesDialog::slotRemoveAll()" << endl;
		//deletedFiles.append ( ( *it )->url() );
		if ( (*it ) ) importView->removeItem ( *it );
	}

	importView->somethingDropped ( false );

	importView->viewport()->update();
}

void AddExistingDirectoriesDialog::slotRemoveSelected()
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

void AddExistingDirectoriesDialog::slotDropped ( QDropEvent* ev )
{
    kdDebug ( 9000 ) << "AddExistingDirectoriesDialog::dropped()" << endl;

	KURL::List urls;

	KURLDrag::decode( ev, urls );

	KFileItem* item = 0L;
	KMimeType::Ptr type = 0L;


    for ( KURL::List::Iterator it = urls.begin(); it != urls.end(); ++it )
    {
        //check if this subproject is already in project
        QString relPath = URLUtil::extractPathNameRelative(m_part->projectDirectory(), *it);
        if (relPath[relPath.length()-1] == '/')
            relPath = relPath.left(relPath.length()-1);
        if ( (relPath.isEmpty()) || ( ! m_widget->allSubprojects().contains( relPath )) )
 		{
			type = KMimeType::findByURL ( ( *it ) );

			if ( type->name() != KMimeType::defaultMimeType() )
			{
				item = new KFileItem ( ( *it ) , type->name(), 0 );
			}
			else
			{
				item = new KFileItem ( ( *it ), "inode/directory", 0 );
			}

			m_importList.append ( item );
		}
    }

	importItems();
}

#include "addexistingdirectoriesdlg.moc"
