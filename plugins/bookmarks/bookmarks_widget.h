/***************************************************************************
 *   Copyright (C) 2003 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __BOOKMARKS_WIDGET_H__
#define __BOOKMARKS_WIDGET_H__


#include <QWidget>
#include <QString>
//#include <qlistview.h>
#include <QToolTip>

#include <k3listview.h>

#include "bookmarks_part.h"

class BookmarksPart;
class BookmarkItem;
class BookmarksConfig;

class BookmarksWidget : public K3ListView, public QToolTip
{
	Q_OBJECT

public:
	BookmarksWidget( BookmarksPart * );
	~BookmarksWidget();

	void update( Q3Dict<EditorData> const & );
	void updateURL( EditorData * );
	bool removeURL( KUrl const & );
	
	BookmarksConfig * config();
	QStringList getContext( KUrl const &, unsigned int );

signals:
	void removeAllBookmarksForURL( const KUrl & );
	void removeBookmarkForURL( const KUrl &, int );

protected:
	void maybeTip( QPoint const & );

private slots:
	void itemClicked( Q3ListViewItem * );
	void popupMenu( Q3ListViewItem * , const QPoint & , int );
	void collapseAll();
	void expandAll();
	void doEmitRemoveBookMark();

private:
	void createURL( EditorData * );

	BookmarksPart * _part;
	BookmarkItem * _selectedItem;
	
//	static CodeLineType _s_codeline;
	
	friend class BookmarkItem;
};


#endif

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
