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

#ifndef __FILELIST_ITEM_H__
#define __FILELIST_ITEM_H__


#include <qlistview.h>

#include <kurl.h>
		 
#include <kdevpartcontroller.h>
		 
class FileListItem : public QListViewItem
{
public:
	FileListItem( QListView * parent, KURL const & url, DocumentState = Clean );
	
	KURL url();
	DocumentState state();
	void setState( DocumentState );
	virtual void setHeight( int );	// override of QListViewItem::setHeight()
	
private:
	KURL _url;
	DocumentState _state;
	
};



#endif

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
