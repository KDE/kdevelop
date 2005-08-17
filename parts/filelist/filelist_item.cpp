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

#include <kiconloader.h>
#include <qfontmetrics.h>
		 
#include "filelist_item.h"

FileListItem * FileListItem::s_activeItem = 0;

FileListItem::FileListItem( Q3ListView * parent, KURL const & url, DocumentState state )
	: Q3ListViewItem( parent, url.fileName() ),
	_url( url )
{
	setState( state );
}
	
KURL FileListItem::url()
{
	return _url;
}

DocumentState FileListItem::state( )
{
	return _state;
}

void FileListItem::setState( DocumentState state )
{
	_state = state;
	
	switch( state )
	{
		case Clean:
			setPixmap( 0, 0L );
			break;
		case Modified:
			setPixmap( 0, SmallIcon("filesave") );
			break;
		case Dirty:
			setPixmap( 0, SmallIcon("revert") );
			break;
		case DirtyAndModified:
			setPixmap( 0, SmallIcon("stop") );
			break;					
	}
}

void FileListItem::setHeight( int )
{
	Q3ListViewItem::setHeight( KIcon::SizeSmall > listView()->fontMetrics().height() ? KIcon::SizeSmall : listView()->fontMetrics().height() );
}

void FileListItem::paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int align )
{
	QColorGroup mcg = cg;
	
	if ( isActive() )
	{
		mcg.setColor( QColorGroup::Base, Qt::yellow );
	}
	
	Q3ListViewItem::paintCell( p, mcg, column, width, align );
}

bool FileListItem::isActive( )
{
	return ( s_activeItem == this );
}

//static
void FileListItem::setActive( FileListItem * item )
{
	s_activeItem = item;
}


// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
