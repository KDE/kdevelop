/***************************************************************************
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <q3popupmenu.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <kdebug.h>
#include <kaction.h>
#include <klocale.h>
#include <kdeversion.h>

#include <kdevproject.h>

#include "fileviewpart.h"
#include "filetreewidget.h"
#include "fileitemfactory.h"

#include "filetreeviewwidgetimpl.h"

using namespace filetreeview;

///////////////////////////////////////////////////////////////////////////////
// class FileTreeViewWidgetImpl
///////////////////////////////////////////////////////////////////////////////

FileTreeViewWidgetImpl::FileTreeViewWidgetImpl( FileTreeWidget *parent, const char *name )
    : QObject( parent, name ), m_branchItemFactory( 0 ),
    m_part( parent->part() ), m_isReloadingTree( false )
{
    kdDebug(9017) << "FileTreeViewWidgetImpl::FileTreeViewWidgetImpl()" << endl;

    // Actions
    m_actionToggleShowNonProjectFiles = new KToggleAction( i18n("Show Non Project Files"), KShortcut(),
        this, SLOT(slotToggleShowNonProjectFiles()), this, "actiontoggleshowshownonprojectfiles" );
#if KDE_IS_VERSION(3,2,90)
    m_actionToggleShowNonProjectFiles->setCheckedState(i18n("Hide Non Project Files"));
#endif
    m_actionToggleShowNonProjectFiles->setWhatsThis(i18n("<b>Show non project files</b><p>Shows files that do not belong to a project in a file tree."));

    // Reload good ol' settings
    QDomDocument &dom = *m_part->projectDom();
    m_actionToggleShowNonProjectFiles->setChecked( !DomUtil::readBoolEntry(dom, "/kdevfileview/tree/hidenonprojectfiles") );
}

///////////////////////////////////////////////////////////////////////////////

FileTreeViewWidgetImpl::~FileTreeViewWidgetImpl()
{
    kdDebug(9017) << "FileTreeViewWidgetImpl::~FileTreeViewWidgetImpl()" << endl;

    delete m_branchItemFactory;

    QDomDocument &dom = *m_part->projectDom();
    DomUtil::writeBoolEntry( dom, "/kdevfileview/tree/hidenonprojectfiles", !showNonProjectFiles() );
}

///////////////////////////////////////////////////////////////////////////////

FileTreeWidget *FileTreeViewWidgetImpl::fileTree() const
{
    return static_cast<FileTreeWidget *>( parent() );
}

///////////////////////////////////////////////////////////////////////////////

QDomDocument &FileTreeViewWidgetImpl::projectDom() const
{
    return *part()->projectDom();
}

///////////////////////////////////////////////////////////////////////////////

QString FileTreeViewWidgetImpl::projectDirectory() const
{
    return part()->project()->projectDirectory();
}

///////////////////////////////////////////////////////////////////////////////

bool FileTreeViewWidgetImpl::showNonProjectFiles() const
{
    return m_actionToggleShowNonProjectFiles->isChecked();
}

///////////////////////////////////////////////////////////////////////////////

void FileTreeViewWidgetImpl::fillPopupMenu( Q3PopupMenu *popupMenu, Q3ListViewItem *item ) const
{
    // Show the "reload tree" menu-item only if it is requested for the root object
    // and we don't have a sync-with-repository operation pending (which otherwise will
    // kill the call-back's from working)
    if (item == fileTree()->firstChild() && canReloadTree())
    {
        int id = popupMenu->insertItem( i18n( "Reload Tree"), this, SLOT( slotReloadTree() ) );
        popupMenu->setWhatsThis( id, i18n("<b>Reload tree</b><p>Reloads the project files tree.") );
    }

    m_actionToggleShowNonProjectFiles->plug( popupMenu );
}

///////////////////////////////////////////////////////////////////////////////

KURL::List FileTreeViewWidgetImpl::selectedPathUrls()
{
    kdDebug(9017) << "FileTreeViewWidgetImpl::selectedPathUrls()" << endl;

	KURL::List urlList;

	Q3ValueList<Q3ListViewItem*> list = allSelectedItems( fileTree()->firstChild() );
	Q3ValueList<Q3ListViewItem*>::Iterator it = list.begin();
	while( it != list.end() )
	{
		KURL url;
		url.setPath( static_cast<FileTreeViewItem*>( *it )->path() );
		urlList << url;
		++it;
	}

	return urlList;
}

///////////////////////////////////////////////////////////////////////////////

Q3ValueList<Q3ListViewItem*> FileTreeViewWidgetImpl::allSelectedItems( Q3ListViewItem * item ) const
{
	Q3ValueList<Q3ListViewItem*> list;

	if ( item )
	{
		if ( item->isSelected() )
		{
			list << item;
		}

		Q3ListViewItem * it = item->firstChild();
		while( it  )
		{
			list += allSelectedItems( it );
			it = it->nextSibling();
		}
	}

	return list;
}

///////////////////////////////////////////////////////////////////////////////

void FileTreeViewWidgetImpl::slotReloadTree()
{
    fileTree()->openDirectory( projectDirectory() );
}

///////////////////////////////////////////////////////////////////////////////

void FileTreeViewWidgetImpl::slotToggleShowNonProjectFiles()
{
    fileTree()->hideOrShow();
}

#include "filetreeviewwidgetimpl.moc"
