#ifndef __BOOKMARKS_WIDGET_H__
#define __BOOKMARKS_WIDGET_H__


#include <qwidget.h>
#include <qstring.h>
#include <qlistview.h>
//#include <qptrdict.h>

#include "bookmarks_part.h"

class KDevProject;
class BookmarksPart;
class BookmarkItem;

class BookmarksWidget : public QListView
{
    Q_OBJECT

public:
    BookmarksWidget(BookmarksPart *part);
    ~BookmarksWidget();

    void update( QDict<EditorData> const & map );
	void updateURL( EditorData * );
	bool removeURL( KURL const & url );

private slots:
	void itemClicked( QListViewItem * );

private:
	void createURL( EditorData * );

	BookmarksPart * _part;

};


#endif
