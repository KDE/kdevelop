
#include <qdict.h>
#include <qheader.h>

#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kdevpartcontroller.h>


#include <kdevcore.h>


#include "bookmarks_part.h"
#include "bookmarks_widget.h"

namespace
{
	// silly little function to make sure the line numbers get sorted correctly
	QString pad( int number )
	{
		static const int paddingbase = 5; // surely, noone has files with 100.000+ lines?!

		int x = number;
		int count = 0;
		while ( x )
		{
			x = x / 10;
			count++;
		}

		QString ret;
		ret.fill( ' ', paddingbase - count);
		return ret + QString::number( number );
	}
}

class BookmarkItem : public QListViewItem
{
public:
	BookmarkItem( QListView * parent, KURL const & url )
		: QListViewItem( parent, url.fileName() ), _url( url ), _line( -1 )
	{}

	BookmarkItem( QListViewItem * parent, KURL const & url, int line )
		: QListViewItem( parent, pad( line +1 ) ), _url( url ), _line( line )
	{}

	KURL url() { return _url; }
	int line() { return _line; }

private:
	KURL _url;
	int _line;

};

BookmarksWidget::BookmarksWidget(BookmarksPart *part)
	: QListView(0, "bookmarks widget"), _part( part )
{
	addColumn( "" );
	header()->hide();
	setRootIsDecorated( true );
	setResizeMode( QListView::LastColumn );

	connect( this, SIGNAL( clicked( QListViewItem * ) ), this, SLOT( itemClicked( QListViewItem * ) ) );
}


BookmarksWidget::~BookmarksWidget()
{}

void BookmarksWidget::update( QDict<EditorData> const & map )
{
	kdDebug(0) << "BookmarksWidget::update()" << endl;

	clear();

	QDictIterator<EditorData> it( map );
	while ( it.current() )
	{
		if ( ! it.current()->marks.isEmpty() )
		{
/*
			QListViewItem * file = new BookmarkItem( this, it.current()->url );
			file->setOpen( true );
			file->setPixmap( 0, SmallIcon( "document" ) );

			QValueListIterator<int> marks_it = it.current()->marks.begin();
			while ( marks_it != it.current()->marks.end() )
			{
				QListViewItem * item = new BookmarkItem( file, it.current()->url, *marks_it );
				item->setPixmap( 0, SmallIcon( "bookmark" ) );
				++marks_it;
			}
*/
			createURL( it.current() );
		}
		++it;
	}

}

void BookmarksWidget::updateURL( EditorData * data )
{
	kdDebug(0) << "BookmarksWidget::updateURL()" << endl;

	// remove the node that contains 'data'
	removeURL( data->url );

	// create it again with new data
	createURL( data );
}

void BookmarksWidget::createURL( EditorData * data )
{
	kdDebug(0) << "BookmarksWidget::createURL()" << endl;

	if ( data )
	{
		QListViewItem * file = new BookmarkItem( this, data->url );
		file->setOpen( true );
		file->setPixmap( 0, SmallIcon( "document" ) );

		QValueListIterator<int> it = data->marks.begin();
		while ( it != data->marks.end() )
		{
			QListViewItem * item = new BookmarkItem( file, data->url, *it );
			item->setPixmap( 0, SmallIcon( "bookmark" ) );
			++it;
		}
	}
}

bool BookmarksWidget::removeURL( KURL const & url )
{
	kdDebug(0) << "BookmarksWidget::removeURL()" << endl;

	QListViewItem * item = firstChild();
	while ( item )
	{
		BookmarkItem * bm = static_cast<BookmarkItem*>(item);
		if ( bm->url() == url )
		{
			kdDebug(0) << "node found. deleting!" << endl;
			
			delete item;
			return true;
		}
		item = item->nextSibling();
	}
	return false;
}

void BookmarksWidget::itemClicked( QListViewItem * clickedItem )
{
	kdDebug(0) << "BookmarksWidget::itemClicked()" << endl;

	if ( ! clickedItem ) return;

	BookmarkItem * item = static_cast<BookmarkItem*>( clickedItem );
	_part->partController()->editDocument( item->url(), item->line() );

}

#include "bookmarks_widget.moc"
