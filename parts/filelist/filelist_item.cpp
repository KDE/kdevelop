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

#include "filelist_item.h"


FileListItem::FileListItem( QListView * parent, KURL const & url )
	: QListViewItem( parent, QString(" ") + url.fileName() ), 
	_url( url ), _state( Clean )
{}
	
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
}


// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
