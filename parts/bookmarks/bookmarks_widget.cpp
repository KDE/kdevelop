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

#include <q3dict.h>
#include <q3header.h>
#include <qtooltip.h>
#include <qpair.h>
#include <q3stylesheet.h>
//Added by qt3to4:
#include <QPixmap>

#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kmenu.h>
#include <klocale.h>

#include <kdevdocumentcontroller.h>
#include <kdevcore.h>
#include <QTextDocument>


#include "bookmarks_part.h"
#include "bookmarks_widget.h"
#include "bookmarks_config.h"

namespace
{

// shamelessly lifted from kdelibs/kate/part/kateviewhelpers.cpp
static const char* const bookmark_xpm[]={
"12 12 4 1",
"b c #808080",
"a c #000080",
"# c #0000ff",
". c None",
"........###.",
".......#...a",
"......#.##.a",
".....#.#..aa",
"....#.#...a.",
"...#.#.a.a..",
"..#.#.a.a...",
".#.#.a.a....",
"#.#.a.a.....",
"#.#a.a......",
"#...a.......",
".aaa........"};

}

class BookmarkItem : public Q3ListViewItem
{
public:
    BookmarkItem( Q3ListView * parent, KUrl const & url )
            : Q3ListViewItem( parent, url.fileName() ),
            _url( url ), _line( -1 ), _isBookmark( false )
    {}

    BookmarkItem( Q3ListViewItem * parent, KUrl const & url, QPair<int,QString> mark )
            : Q3ListViewItem( parent, QString::number( mark.first +1 ).rightJustified( 5 ) ),
            _url( url ), _line( mark.first ), _isBookmark( true )
    {
        BookmarksWidget * lv = static_cast<BookmarksWidget*>( listView() );
        BookmarksConfig::CodeLineType codeline = lv->config()->codeline();

        if ( codeline == BookmarksConfig::Never )
        {
            return;
        }

        if ( codeline == BookmarksConfig::Token )
        {
            if ( mark.second.startsWith( lv->config()->token() ) )
            {
                setText( 0, text( 0 ) + "  " + mark.second );
            }
            return;
        }

        setText( 0, text( 0 ) + "  " + mark.second );
    }

    KUrl url()
    {
        return _url;
    }
    int line()
    {
        return _line;
    }

    QString tipText()
    {
        if ( _isBookmark )
        {
            BookmarksWidget * w = static_cast<BookmarksWidget*> ( listView() );
            QStringList list = w->getContext( _url, _line );

            QString code = "<qt><table><tr><td><pre>";
            for ( uint i = 0; i < list.count(); i++)
            {
                QString temp = Qt::escape( list[i] );

                if ( i == (list.count() / 2) )  // count() is always odd
                {
                    temp = "<b>" + temp + "</b>";
                }
                code += temp + "\n";
            }
            code += "</pre></td></tr></table></qt>";

            return code;
        }
        else
        {
            return _url.prettyURL();
        }
    }

    bool isBookmark()
    {
        return _isBookmark;
    }

private:
    KUrl _url;
    int _line;
    bool _isBookmark;
    QString _code;

};

BookmarksWidget::BookmarksWidget(BookmarksPart *part)
        : KListView(0, "bookmarks widget"), QToolTip( viewport() ),
        _part( part )

{
    addColumn( QString() );
    header()->hide();
    setRootIsDecorated( true );
    setResizeMode( Q3ListView::LastColumn );
    setAllColumnsShowFocus( true );

    connect( this, SIGNAL( executed( Q3ListViewItem * ) ), this, SLOT( itemClicked( Q3ListViewItem * ) ) );
    connect( this, SIGNAL( returnPressed( Q3ListViewItem * ) ), this, SLOT( itemClicked( Q3ListViewItem * ) ) );
    connect( this, SIGNAL( contextMenuRequested ( Q3ListViewItem *, const QPoint & , int ) ),
        this, SLOT( popupMenu(Q3ListViewItem *, const QPoint & , int ) ) );
}


BookmarksWidget::~BookmarksWidget()
{}

void BookmarksWidget::maybeTip(const QPoint &p)
{
//  kDebug(0) << "ToolTip::maybeTip()" << endl;

    if ( ! _part->config()->toolTip() ) return;

    BookmarkItem * item = dynamic_cast<BookmarkItem*>( itemAt( p ) );
    QRect r = itemRect( item );

    if ( item && r.isValid() )
    {
        tip( r, item->tipText() );
    }
}

void BookmarksWidget::update( Q3Dict<EditorData> const & map )
{
//  kDebug(0) << "BookmarksWidget::update()" << endl;

    Q3ListView::clear();

    Q3DictIterator<EditorData> it( map );
    while ( it.current() )
    {
        if ( ! it.current()->marks.isEmpty() )
        {
            createURL( it.current() );
        }
        ++it;
    }
}

void BookmarksWidget::updateURL( EditorData * data )
{
//  kDebug(0) << "BookmarksWidget::updateURL()" << endl;

    // remove the node that contains 'data'
    removeURL( data->url );

    // create it again with new data
    createURL( data );
}

void BookmarksWidget::createURL( EditorData * data )
{
//  kDebug(0) << "BookmarksWidget::createURL()" << endl;

    if ( data )
    {
        Q3ListViewItem * file = new BookmarkItem( this, data->url );
        file->setOpen( true );
        file->setPixmap( 0, SmallIcon( "document" ) );

        Q3ValueListIterator< QPair<int,QString> > it = data->marks.begin();
        while ( it != data->marks.end() )
        {
            Q3ListViewItem * item = new BookmarkItem( file, data->url, *it );
            item->setPixmap( 0, QPixmap((const char**)bookmark_xpm) );
            ++it;
        }
    }
}

bool BookmarksWidget::removeURL( KUrl const & url )
{
//  kDebug(0) << "BookmarksWidget::removeURL()" << endl;

    Q3ListViewItem * item = firstChild();
    while ( item )
    {
        BookmarkItem * bm = static_cast<BookmarkItem*>(item);
        if ( bm->url() == url )
        {
            delete item;
            return true;
        }
        item = item->nextSibling();
    }
    return false;
}

void BookmarksWidget::doEmitRemoveBookMark()
{
//  kDebug(0) << "BookmarksWidget::doEmitRemoveBookMark()" << endl;

    if ( _selectedItem->isBookmark() )
    {
        emit removeBookmarkForURL( _selectedItem->url(), _selectedItem->line() );
    }
    else
    {
        emit removeAllBookmarksForURL( _selectedItem->url() );
    }
}

void BookmarksWidget::popupMenu( Q3ListViewItem * item, const QPoint & p, int )
{
//  kDebug(0) << "BookmarksWidget::contextMenuRequested()" << endl;

    if ( item )
    {
        _selectedItem = static_cast<BookmarkItem *>(item);

        KMenu popup;

        if ( _selectedItem->isBookmark() )
        {
            popup.insertTitle( _selectedItem->url().fileName() + i18n(", line ")
                + QString::number( _selectedItem->line() +1 ) );

            popup.insertItem( i18n("Remove This Bookmark"), this, SLOT( doEmitRemoveBookMark() ) );
        }
        else
        {
            popup.insertTitle( _selectedItem->url().fileName() + i18n( ", All" ) );
            popup.insertItem( i18n("Remove These Bookmarks"), this, SLOT( doEmitRemoveBookMark() ) );
        }

        popup.insertSeparator();

        popup.insertItem( i18n( "Collapse All" ), this, SLOT(collapseAll()) );
        popup.insertItem( i18n( "Expand All" ), this, SLOT(expandAll()) );

        popup.exec(p);
    }

}

void BookmarksWidget::itemClicked( Q3ListViewItem * clickedItem )
{
//  kDebug(0) << "BookmarksWidget::itemClicked()" << endl;

    if ( ! clickedItem )
        return;

    BookmarkItem * item = static_cast<BookmarkItem*>( clickedItem );
    _part->documentController()->editDocument( item->url(), item->line() );

}

BookmarksConfig * BookmarksWidget::config( )
{
    return _part->config();
}

QStringList BookmarksWidget::getContext( KUrl const & url, unsigned int line )
{
    return _part->getContext( url, line, config()->context() );
}

void BookmarksWidget::collapseAll( )
{
    Q3ListViewItem * it = firstChild();
    while( it )
    {
        it->setOpen( false );
        it = it->nextSibling();
    }
}

void BookmarksWidget::expandAll( )
{
    Q3ListViewItem * it = firstChild();
    while( it )
    {
        it->setOpen( true );
        it = it->nextSibling();
    }
}


#include "bookmarks_widget.moc"

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
