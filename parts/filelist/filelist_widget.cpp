/***************************************************************************
 *   Copyright (C) 2004 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qheader.h>

#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>

#include <kdevcore.h>
#include <kdevpartcontroller.h>

#include "filelist_part.h"
#include "filelist_widget.h"
#include "filelist_item.h"


FileListWidget::FileListWidget(FileListPart *part)
 : KListView(0, "filelist widget"), QToolTip( viewport() ), _part( part )
{
	addColumn( QString::null );
	header()->hide();
	setRootIsDecorated( false );
	setResizeMode( QListView::LastColumn );

	connect( _part->partController(), SIGNAL( partAdded(KParts::Part*) ), this, SLOT( partChanged() ) );
	connect( _part->partController(), SIGNAL( partRemoved(KParts::Part*) ), this, SLOT( partChanged() ) );
	connect( _part->partController(), SIGNAL( activePartChanged(KParts::Part*) ), this, SLOT( activePartChanged(KParts::Part* )) );

	connect( this, SIGNAL( executed( QListViewItem * ) ), this, SLOT( itemClicked( QListViewItem * ) ) );
	connect( this, SIGNAL( returnPressed( QListViewItem * ) ), this, SLOT( itemClicked( QListViewItem * ) ) );
	
	partChanged();
}


FileListWidget::~FileListWidget()
{}

void FileListWidget::maybeTip( QPoint const & p )
{
	FileListItem * item = static_cast<FileListItem*>( itemAt( p ) );
	QRect r = itemRect( item );

	if ( item && r.isValid() )
	{
		tip( r, item->url().url() );
	}
}

void FileListWidget::partChanged()
{
	KListView::clear();
	
	KURL::List list( _part->openFiles() );	
	QValueListIterator<KURL> it = list.begin();
	while ( it != list.end() )
	{
		new FileListItem( this, *it );
		++it;
	}

	activePartChanged( _part->partController()->activePart() );
}

void FileListWidget::itemClicked( QListViewItem * item )
{
	if ( !item ) return;
	
	FileListItem * listItem = static_cast<FileListItem*>( item );
	_part->partController()->editDocument( listItem->url() );
}

void FileListWidget::activePartChanged( KParts::Part * part )
{
	KParts::ReadOnlyPart * ro_part = dynamic_cast<KParts::ReadOnlyPart*>( part );
	if ( ro_part )
	{
		FileListItem * item = static_cast<FileListItem*>( firstChild() );
		while ( item )
		{
			if ( item->url() == ro_part->url() )
			{
				setSelected( item, true );
				return;
			}
			item = static_cast<FileListItem*>( item->nextSibling() );
		}
	}
}

			
#include "filelist_widget.moc"


// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
