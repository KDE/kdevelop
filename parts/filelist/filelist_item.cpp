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


FileListItem::FileListItem( QListView * parent, KURL const & url, DocumentState state )
	: QListViewItem( parent, QString(" ") + url.fileName() ), 
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
	QListViewItem::setHeight( KIcon::SizeSmall > listView()->fontMetrics().height() ? KIcon::SizeSmall : listView()->fontMetrics().height() );
}


// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
